#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <cmath>

// Constantes matemáticas
const double PI = std::acos(-1.0);
const double PHI = (1.0 + std::sqrt(5.0)) / 2.0; // Razão de Ouro (~1.6180339887)

using Point = std::complex<double>;

// Tipos de triângulos de Robinson
enum class TriangleType {
    Acute,  // Triângulo Agudo (Acoustico / Ouro) - Ângulos 36°, 72°, 72°
    Obtuse  // Triângulo Obtuso (Gnomon de Ouro)   - Ângulos 108°, 36°, 36°
};

struct Triangle {
    TriangleType type;
    Point A, B, C;
};

// Algoritmo de subdivisão (deflação) de Penrose
std::vector<Triangle> subdivide(const std::vector<Triangle>& triangles) {
    std::vector<Triangle> result;
    result.reserve(triangles.size() * 2);

    for (const auto& t : triangles) {
        if (t.type == TriangleType::Acute) {
            // Subdivide triângulo agudo em 1 agudo e 1 obtuso
            // P divide o lado AB na proporção 1 : PHI
            Point P = t.A + (t.B - t.A) / PHI;
            result.push_back({TriangleType::Acute, t.C, P, t.B});
            result.push_back({TriangleType::Obtuse, P, t.C, t.A});
        } else {
            // Subdivide triângulo obtuso em 1 obtuso e 1 agudo
            // Q divide o lado BC na proporção 1 : PHI
            Point Q = t.B + (t.C - t.B) / PHI;
            result.push_back({TriangleType::Obtuse, Q, t.C, t.A});
            result.push_back({TriangleType::Acute, t.A, Q, t.B});
        }
    }
    return result;
}

// Exporta o padrão gerado para um arquivo SVG
void exportToSVG(const std::string& filename, const std::vector<Triangle>& triangles, int width, int height) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao criar o arquivo " << filename << std::endl;
        return;
    }

    // Definir área de visualização centralizada
    double scale = width * 0.45;
    Point center(width / 2.0, height / 2.0);

    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width 
         << "\" height=\"" << height << "\" viewBox=\"0 0 " << width << " " << height << "\">\n";
    file << "  <rect width=\"100%\" height=\"100%\" fill=\"#111116\"/>\n";
    file << "  <g stroke=\"#ffffff\" stroke-width=\"0.5\" stroke-linecap=\"round\" stroke-linejoin=\"round\">\n";

    for (const auto& t : triangles) {
        // Converte coordenadas complexas para o sistema de tela (SVG)
        Point pA = center + t.A * scale;
        Point pB = center + t.B * scale;
        Point pC = center + t.C * scale;

        // Cores distintas para os dois tipos de triângulos/azulejos
        std::string color = (t.type == TriangleType::Acute) ? "#2b5c8f" : "#d97736";

        file << "    <polygon points=\""
             << pA.real() << "," << pA.imag() << " "
             << pB.real() << "," << pB.imag() << " "
             << pC.real() << "," << pC.imag() << "\" "
             << "fill=\"" << color << "\" fill-opacity=\"0.85\" />\n";
    }

    file << "  </g>\n";
    file << "</svg>\n";

    std::cout << "Mosaico gerado com sucesso: " << filename << std::endl;
}

int main() {
    // Configurações
    const int N_SUBDIVISOES = 6; // Ajuste entre 4 e 8 para alterar o nível de detalhamento
    const int LARGURA = 1000;
    const int ALTURA = 1000;

    std::vector<Triangle> triangles;

    // Inicialização: Roda simétrica de 10 triângulos agudos ao redor da origem
    for (int i = 0; i < 10; ++i) {
        Point B = std::polar(1.0, (2 * i - 1) * PI / 10.0);
        Point C = std::polar(1.0, (2 * i + 1) * PI / 10.0);
        
        if (i % 2 == 0) {
            triangles.push_back({TriangleType::Acute, Point(0, 0), B, C});
        } else {
            triangles.push_back({TriangleType::Acute, Point(0, 0), C, B});
        }
    }

    // Executa as subdivisões recursivas de Penrose
    for (int i = 0; i < N_SUBDIVISOES; ++i) {
        triangles = subdivide(triangles);
    }

    std::cout << "Total de triângulos gerados: " << triangles.size() << std::endl;

    // Exporta para SVG
    exportToSVG("mosaico_penrose.svg", triangles, LARGURA, ALTURA);

    return 0;
}