#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using FileHash = std::vector<std::string>;
using DuplicatesMap = std::map<FileHash, std::vector<fs::path>>;

class FileHasher {
public:
    FileHasher(size_t block_size, const std::string& hash_algorithm)
        : block_size_(block_size), hash_algorithm_(hash_algorithm) {}

    FileHash compute_file_hash(const fs::path& filepath) {
        FileHash hash_sequence;
        std::ifstream file(filepath.string(), std::ios::binary);

        if (!file) {
            throw std::runtime_error("Cannot open file: " + filepath.string());
        }

        std::vector<char> buffer(block_size_, 0);
        while (file) {
            file.read(buffer.data(), block_size_);
            std::streamsize bytes_read = file.gcount();

            // Pad with zeros if needed
            if (bytes_read < static_cast<std::streamsize>(block_size_)) {
                std::fill(buffer.begin() + bytes_read, buffer.end(), 0);
            }

            std::string block_hash;
            if (hash_algorithm_ == "crc32") {
                block_hash = compute_crc32(buffer);
            } else if (hash_algorithm_ == "md5") {
                block_hash = compute_md5(buffer);
            } else {
                throw std::runtime_error("Unsupported hash algorithm");
            }

            hash_sequence.push_back(block_hash);
        }

        return hash_sequence;
    }

private:
    std::string compute_crc32(const std::vector<char>& data) {
        boost::crc_32_type result;
        result.process_bytes(data.data(), data.size());
        return std::to_string(result.checksum());
    }

    std::string compute_md5(const std::vector<char>& data) {
        boost::uuids::detail::md5 hash;
        boost::uuids::detail::md5::digest_type digest;

        hash.process_bytes(data.data(), data.size());
        hash.get_digest(digest);

        const auto charDigest = reinterpret_cast<const char*>(&digest);
        std::string result;
        boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::md5::digest_type),
                             std::back_inserter(result));
        return result;
    }

    size_t block_size_;
    std::string hash_algorithm_;
};

class FileScanner {
public:
    FileScanner(const std::vector<fs::path>& include_dirs,
                const std::vector<fs::path>& exclude_dirs,
                size_t scan_level,
                size_t min_file_size,
                const std::vector<std::string>& file_masks)
        : include_dirs_(include_dirs),
          exclude_dirs_(exclude_dirs),
          scan_level_(scan_level),
          min_file_size_(min_file_size),
          file_masks_(file_masks) {}

    std::vector<fs::path> scan_files() {
        std::vector<fs::path> files;

        for (const auto& dir : include_dirs_) {
            if (fs::exists(dir)) {
                scan_directory(dir, files);
            }
        }

        // Filter by size and masks
        files.erase(std::remove_if(files.begin(), files.end(),
            [this](const fs::path& file) {
                return !matches_criteria(file);
            }), files.end());

        return files;
    }

private:
    void scan_directory(const fs::path& dir, std::vector<fs::path>& files, size_t current_level = 0) {
        if (std::find(exclude_dirs_.begin(), exclude_dirs_.end(), dir) != exclude_dirs_.end()) {
            return;
        }

        try {
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (fs::is_regular_file(entry.status())) {
                    files.push_back(entry.path());
                } else if (fs::is_directory(entry.status()) &&
                          (scan_level_ == 0 || current_level < scan_level_)) {
                    scan_directory(entry.path(), files, current_level + 1);
                }
            }
        } catch (const fs::filesystem_error&) {
            // Skip directories we can't access
        }
    }

    bool matches_criteria(const fs::path& file) const {
        // Check file size
        if (fs::file_size(file) < min_file_size_) {
            return false;
        }

        // Check file masks if any
        if (!file_masks_.empty()) {
            std::string extension = file.extension().string();
            boost::algorithm::to_lower(extension);

            bool matches = false;
            for (const auto& mask : file_masks_) {
                std::string lower_mask = mask;
                boost::algorithm::to_lower(lower_mask);

                if (extension == lower_mask ||
                    (lower_mask.size() > 0 && lower_mask[0] == '*' &&
                     extension.size() >= lower_mask.size() - 1 &&
                     extension.substr(extension.size() - (lower_mask.size() - 1)) ==
                     lower_mask.substr(1))) {
                    matches = true;
                    break;
                }
            }

            if (!matches) {
                return false;
            }
        }

        return true;
    }

    std::vector<fs::path> include_dirs_;
    std::vector<fs::path> exclude_dirs_;
    size_t scan_level_;
    size_t min_file_size_;
    std::vector<std::string> file_masks_;
};

void find_duplicates(const std::vector<fs::path>& files,
                     size_t block_size,
                     const std::string& hash_algorithm) {
    FileHasher hasher(block_size, hash_algorithm);
    DuplicatesMap duplicates;

    for (const auto& file : files) {
        try {
            FileHash file_hash = hasher.compute_file_hash(file);
            duplicates[file_hash].push_back(file);
        } catch (const std::exception& e) {
            std::cerr << "Error processing file " << file << ": " << e.what() << std::endl;
        }
    }

    // Output results
    for (const auto& [hash, files] : duplicates) {
        if (files.size() > 1) {
            for (const auto& file : files) {
                std::cout << file.string() << std::endl;
            }
            std::cout << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line options
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "Show this help message")
            ("include,i", po::value<std::vector<std::string>>()->multitoken(),
             "Directories to scan (can specify multiple)")
            ("exclude,e", po::value<std::vector<std::string>>()->multitoken(),
             "Directories to exclude (can specify multiple)")
            ("level,l", po::value<size_t>()->default_value(0),
             "Scan level (0 - only specified directory, no subdirectories)")
            ("min-size,m", po::value<size_t>()->default_value(1),
             "Minimum file size in bytes")
            ("mask", po::value<std::vector<std::string>>()->multitoken(),
             "File masks to include (e.g. *.txt *.cpp)")
            ("block-size,b", po::value<size_t>()->default_value(1024),
             "Block size for reading files")
            ("hash", po::value<std::string>()->default_value("crc32"),
             "Hash algorithm (crc32 or md5)")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        // Get include directories (default to current if none specified)
        std::vector<fs::path> include_dirs;
        if (vm.count("include")) {
            for (const auto& dir : vm["include"].as<std::vector<std::string>>()) {
                include_dirs.push_back(fs::absolute(dir));
            }
        } else {
            include_dirs.push_back(fs::current_path());
        }

        // Get exclude directories
        std::vector<fs::path> exclude_dirs;
        if (vm.count("exclude")) {
            for (const auto& dir : vm["exclude"].as<std::vector<std::string>>()) {
                exclude_dirs.push_back(fs::absolute(dir));
            }
        }

        // Get other parameters
        size_t scan_level = vm["level"].as<size_t>();
        size_t min_file_size = vm["min-size"].as<size_t>();
        size_t block_size = vm["block-size"].as<size_t>();
        std::string hash_algorithm = vm["hash"].as<std::string>();

        // Validate hash algorithm
        if (hash_algorithm != "crc32" && hash_algorithm != "md5") {
            throw std::runtime_error("Invalid hash algorithm. Use 'crc32' or 'md5'");
        }

        // Get file masks
        std::vector<std::string> file_masks;
        if (vm.count("mask")) {
            file_masks = vm["mask"].as<std::vector<std::string>>();
        }

        FileScanner scanner(include_dirs, exclude_dirs, scan_level, min_file_size, file_masks);
        std::vector<fs::path> files = scanner.scan_files();
        find_duplicates(files, block_size, hash_algorithm);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
