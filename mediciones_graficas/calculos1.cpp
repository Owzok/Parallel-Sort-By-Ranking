#include <iostream>
#include <iomanip> // Para std::fixed y std::setprecision
#include <cmath>

#define N 14400 // Define el valor de n como una constante

double calcular_expresion(double p) {
    if (p <= 0) {
        throw std::invalid_argument("p debe ser mayor que 0.");
    }

    double term1 = 2 * p * (N / p);
    double term2 = 3 * std::sqrt(p) * (N / std::sqrt(p));
    double term3 = (N / std::sqrt(p)) * (std::log(N / std::sqrt(p)) / std::log(2)); // Log base 2
    double term4 = (N * N) / p;

    return term1 + term2 + term3 + term4;
}

int main() {
    double p;

    std::cout << "Ingrese el valor de p: ";
    std::cin >> p;

    try {
        double resultado = calcular_expresion(p);
        // Mostrar el resultado con 6 decimales
        std::cout << std::fixed << std::setprecision(10);
        std::cout << "El resultado de la expresión es: " << resultado << std::endl;
    } catch (const std::invalid_argument &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
