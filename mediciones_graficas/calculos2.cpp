#include <iostream>
#include <iomanip> // Para std::fixed y std::setprecision
#include <cmath>

#define N 14400 // Define el valor de n como una constante

double calcular_expresion(double p) {
    if (p <= 0) {
        throw std::invalid_argument("p debe ser mayor que 0.");
    }

    // Primer término: p * (n / p)
    double term1 = p * (N / p);

    // Segundo término: 2 * log2(sqrt(p)) * (n / sqrt(p))
    double term2 = 2 * (std::log(std::sqrt(p)) / std::log(2)) * (N / std::sqrt(p));

    // Tercer término: sqrt(p) * log2(sqrt(p)) * (n / p)
    double term3 = std::sqrt(p) * (std::log(std::sqrt(p)) / std::log(2)) * (N / p);

    // Cuarto término: (n / sqrt(p)) * log2(n / sqrt(p))
    double term4 = (N / std::sqrt(p)) * (std::log(N / std::sqrt(p)) / std::log(2));

    // Quinto término: (n^2 / p)
    double term5 = (N * N) / p;

    // Sexto término: sqrt(p) * (n / sqrt(p))
    double term6 = std::sqrt(p) * (N / std::sqrt(p));

    // Sumar todos los términos
    return term1 + term2 + term3 + term4 + term5 + term6;
}

int main() {
    double p;

    std::cout << "Ingrese el valor de p: ";
    std::cin >> p;

    try {
        double resultado = calcular_expresion(p);
        // Mostrar el resultado con 6 decimales
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "El resultado de la expresión es: " << resultado << std::endl;
    } catch (const std::invalid_argument &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
