#include <iostream>
#include <string>
#include <cmath>

int main()
{
    const float dacBase = 2047.0;
    const float dacMax = 2047.0;
    const int steps  = 120;
    
    std::cout << "int noteVoltsLookUp[] = {\n";
    
    for (int i = 0; i <= steps; i++) {
        int modStep = i & 0xf;
        float dacValue = dacBase - (dacMax * float(i) / float(steps));
        if (modStep == 0) {
         std::cout << "  ";   
        }
        std::cout << roundf(dacValue);
        if (i < steps) {
            if (modStep == 15) {
             std::cout << ",\n";   
            } else {
             std::cout << ", ";   
            }
        } else {
            std::cout << "\n";            
        }
    }

    std::cout << "};";
}

