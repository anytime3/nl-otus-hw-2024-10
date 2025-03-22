#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <memory>
#include <vector>
#include "Primitive.h"

class Document {
public:
  Document();
  void addPrimitive(std::shared_ptr<Primitive> primitive);
  void removePrimitive(std::shared_ptr<Primitive> primitive);
  void exportToFile(const std::string & filename) const;
  void importFromFile(const std::string & filename);

private:
  std::vector<std::shared_ptr<Primitive>> primitives;
};

#endif // DOCUMENT_H
