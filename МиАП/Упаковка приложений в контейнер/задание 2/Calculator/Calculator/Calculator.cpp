#include <iostream>
#include <vector>
#include <string>

class Calculator {
public:
    double add(double a, double b) { return a + b; }
    double subtract(double a, double b) { return a - b; }
    double multiply(double a, double b) { return a * b; }
    double divide(double a, double b) {
        if (b == 0) throw std::runtime_error("Division by zero!");
        return a / b;
    }
};

int main() {
    Calculator calc;

    std::cout << "Calculator Test in Docker Container" << std::endl;
    std::cout << "5 + 3 = " << calc.add(5, 3) << std::endl;
    std::cout << "5 - 3 = " << calc.subtract(5, 3) << std::endl;
    std::cout << "5 * 3 = " << calc.multiply(5, 3) << std::endl;
    std::cout << "6 / 3 = " << calc.divide(6, 3) << std::endl;

    return 0;
}