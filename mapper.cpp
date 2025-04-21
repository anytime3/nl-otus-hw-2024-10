#include <iostream>
#include <string>
#include <sstream>
using namespace std;

int main(int argc, char ** argv)
{
    string line;
    while (std::getline(std::cin, line))
    {
        std::string token;
        stringstream ss{line};

        string location;
        double price = 0;
        int tokenNum = 0;
        const int TOKEN_LOCATION = 5;
        const int TOKEN_PRICE = 9;
        const int TOTAL_TOKENS_IN_VALID_DATA = 16;

        while(getline(ss, token, ','))
        {
            if ( tokenNum == TOKEN_LOCATION )
            {
                if ( ! token.empty() )
                    location = std::move(token);
            }
            else if( tokenNum == TOKEN_PRICE )
            {
                try { price = stod(token); }
                catch ( ... ) {}
            }
            tokenNum++;
        }

        if ( location.empty() || price <= 0 || tokenNum != TOTAL_TOKENS_IN_VALID_DATA )
            continue; // do not use empty location or price, treat them as invalid input
        cout << location << "," << price <<endl;
    }

    return 0;
}
