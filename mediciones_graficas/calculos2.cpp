#include <iostream>
#include <cmath>
#include <iomanip>

// Definimos N como constante
#define N 14400 // Cambia este valor según tus necesidades

// Declaración de funciones para los términos de la fórmula
double calcularTermino1(double p, double alpha, double beta)
{
    return p * (alpha + (N / (p * beta)));
}

double calcularTermino2(double p, double alpha, double beta)
{
    return 2 * std::log(std::sqrt(p)) * (alpha + (N / std::sqrt(p) * beta));
}

double calcularTermino3(double p, double alpha, double beta)
{
    return std::sqrt(p) * std::log(std::sqrt(p)) * (alpha + (N / std::sqrt(p) * beta));
}

double calcularTermino4(double p)
{
    return 2 * (N / std::sqrt(p)) * std::log(N / std::sqrt(p));
}

double calcularTermino5(double p, double alpha, double beta)
{
    return std::sqrt(p) * (alpha + (N / std::sqrt(p) * beta));
}

int main()
{
    double p, alpha = 2.0, beta = 3.0; // Cambia estos valores según necesidad

    std::cout << "Ingrese el valor de p: ";
    std::cin >> p;

    // Calcular cada término
    double T1 = calcularTermino1(p, alpha, beta);
    double T2 = calcularTermino2(p, alpha, beta);
    double T3 = calcularTermino3(p, alpha, beta);
    double T4 = calcularTermino4(p);
    double T5 = calcularTermino5(p, alpha, beta);

    // Suma de términos
    double T_ideal = T1 + T2 + T3 + T4 + T5;

    // Resultado
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "El valor aproximado de T_ideal es: " << T_ideal << std::endl;

    return 0;
}
