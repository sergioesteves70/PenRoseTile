#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

const double PI = std::acos(-1.0);

struct RGB { int r, g, b; };

// Paleta térmica de alta visibilidade para mapear a densidade do quasicristal
RGB getColor(double val) {
    val = std::clamp(val, 0.0, 1.0);

    // Mapeamento de intensidade: Preto -> Azul -> Turquesa -> Amarelo -> Branco
    int r = static_cast<int>(255.0 * std::pow(val, 1.6));
    int g = static_cast<int>(255.0 * std::pow(val, 0.7));
    int b = static_cast<int>(255.0 * (1.0 - std::abs(val - 0.5) * 1.8));

    return {
        std::clamp(r, 0, 255),
        std::clamp(g, 0, 255),
        std::clamp(b, 0, 255)
    };
}

int main() {
    // -------------------------------------------------------------
    // PARÂMETROS DO QUASICRISTAL
    // -------------------------------------------------------------
    const int N = 14;             // Simetria de N-dobras (Altere para 5, 7, 8, 10, 12, 14, etc.)
    const int RESOLUCAO = 500;   // Largura e altura em pixels
    const double ESCALA = 40.0;  // Zoom do campo espacial do quasicristal

    // Vetores de onda k_j distribuídos com simetria rotacional N-dobra
    std::vector<double> kx(N), ky(N);
    for (int j = 0; j < N; ++j) {
        double angle = 2.0 * PI * j / N;
        kx[j] = std::cos(angle);
        ky[j] = std::sin(angle);
    }

    // Calculando o campo de densidade I(x, y) = |sum_j cos(k_j . r)|^2
    std::vector<double> grid(RESOLUCAO * RESOLUCAO);
    double minVal = 1e9, maxVal = -1e9;

    for (int y = 0; y < RESOLUCAO; ++y) {
        double ny = (static_cast<double>(y) / RESOLUCAO - 0.5) * ESCALA;
        for (int x = 0; x < RESOLUCAO; ++x) {
            double nx = (static_cast<double>(x) / RESOLUCAO - 0.5) * ESCALA;

            // Superposição das N ondas planas
            double psi = 0.0;
            for (int j = 0; j < N; ++j) {
                psi += std::cos(nx * kx[j] + ny * ky[j]);
            }

            // Densidade de intensidade atômica
            double intensity = psi * psi;
            grid[y * RESOLUCAO + x] = intensity;

            minVal = std::min(minVal, intensity);
            maxVal = std::max(maxVal, intensity);
        }
    }

    // Exportação em formato SVG
    std::ofstream svgFile("quasicrystal.svg");
    svgFile << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << RESOLUCAO 
            << "\" height=\"" << RESOLUCAO << "\" viewBox=\"0 0 " << RESOLUCAO << " " << RESOLUCAO << "\">\n";
    svgFile << "  <rect width=\"100%\" height=\"100%\" fill=\"#050508\"/>\n";

    // Desenha a malha de densidade
    for (int y = 0; y < RESOLUCAO; ++y) {
        for (int x = 0; x < RESOLUCAO; ++x) {
            double val = (grid[y * RESOLUCAO + x] - minVal) / (maxVal - minVal);
            
            // Otimização: ignora pixels muito escuros para reduzir tamanho do SVG
            if (val < 0.04) continue;

            RGB c = getColor(val);
            svgFile << "  <rect x=\"" << x << "\" y=\"" << y 
                    << "\" width=\"1.05\" height=\"1.05\" fill=\"rgb(" 
                    << c.r << "," << c.g << "," << c.b << ")\"/>\n";
        }
    }

    svgFile << "</svg>\n";

    std::cout << "Quasicristal gerado com simetria de " << N << " dobras em 'quasicrystal.svg'!" << std::endl;
    return 0;
}