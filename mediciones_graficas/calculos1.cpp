#include <iostream>
#include <cmath>
#include <iomanip>

// Definimos N como constante
#define N 14400 // Cambia este valor según tus necesidades

// Declaración de funciones para los términos de la fórmula
double calcularT1(double p, double x, double beta)
{
    return 2 * p * (x + (N / (p * beta)));
}

double calcularT2(double p, double alpha, double beta)
{
    return 3 * std::sqrt(p) * (alpha + (N / std::sqrt(p) * beta));
}

double calcularT3(double p)
{
    return 2 * (N / std::sqrt(p)) * std::log(N / std::sqrt(p));
}

int main()
{
    double p, x = 5.0, alpha = 2.0, beta = 3.0; // Puedes cambiar x, α y β según necesidad

    std::cout << "Ingrese el valor de p: ";
    std::cin >> p;

    // Calcular cada término
    double T1 = calcularT1(p, x, beta);
    double T2 = calcularT2(p, alpha, beta);
    double T3 = calcularT3(p);

    // Suma de términos
    double T_imp = T1 + T2 + T3;

    // Resultado
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "El valor aproximado de T_imp es: " << T_imp << std::endl;

    return 0;
}
