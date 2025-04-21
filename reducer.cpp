#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
using namespace std;

int main(int argc, char ** argv)
{
    size_t count = 0;
    double sum;
    double sumOfSquares = 0;
    string line;
    while (std::getline(std::cin, line))
    {
        stringstream ss(line);
        string tmp;
        getline(ss, tmp, ','); // location
        getline(ss, tmp, ','); // price
        double price;
        try { price = stod(tmp); }
        catch(...){ continue;}

        sum += price;
        sumOfSquares += price*price;
        count++;
    }

    // NOTE если используем несколько редюсеров - то возможно надо выводить sum,sumOfSquares,count
    // а потом их вывод схлопывать на финальном шаге
    double mean = sum/count;
    double stdDev = sqrt(sumOfSquares/count - mean*mean);
    cout << "mean=" << mean << " stdDev=" << stdDev << endl;
    return 0;
}
