#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <array>
#include <algorithm>

// Constantes matemáticas
const double PI = std::acos(-1.0);

struct Vector2D {
    double x, y;

    Vector2D operator+(const Vector2D& o) const { return {x + o.x, y + o.y}; }
    Vector2D operator-(const Vector2D& o) const { return {x - o.x, y - o.y}; }
    Vector2D operator*(double s) const { return {x * s, y * s}; }
};

double dot(const Vector2D& a, const Vector2D& b) {
    return a.x * b.x + a.y * b.y;
}

enum class RhombusType {
    Thick, // Losango Largo (ângulos 72° e 108°)
    Thin   // Losango Fino  (ângulos 36° e 144°)
};

struct Rhombus {
    RhombusType type;
    std::array<Vector2D, 4> vertices;
};

// Resolve o sistema 2x2 para encontrar a interseção entre duas linhas da grade 5D
bool intersectGridLines(const Vector2D& er, double vr, const Vector2D& es, double vs, Vector2D& outPt) {
    double det = er.x * es.y - er.y * es.x;
    if (std::abs(det) < 1e-9) return false; // Linhas paralelas

    outPt.x = (vr * es.y - vs * er.y) / det;
    outPt.y = (er.x * vs - es.x * vr) / det;
    return true;
}

int main() {
    // 1. Definição dos 5 vetores unitários direcionais (vértices do pentágono)
    std::array<Vector2D, 5> e;
    for (int j = 0; j < 5; ++j) {
        double angle = 2.0 * PI * j / 5.0;
        e[j] = {std::cos(angle), std::sin(angle)};
    }

    // 2. Deslocamentos de grade (gammas) satisfazendo sum(gamma_j) = 0
    // Garante que não haja interseções triplas (propriedade de não-singularidade)
    std::array<double, 5> gamma = {0.12, 0.28, -0.18, 0.08, -0.30};

    const int GRID_RANGE = 18; // Alcance das linhas de grade (-N até N)
    std::vector<Rhombus> rhombuses;

    // 3. Iterar sobre todos os pares de famílias de linhas (r, s)
    for (int r = 0; r < 5; ++r) {
        for (int s = r + 1; s < 5; ++s) {
            // Tipo de losango determinado pela diferença entre as direções das linhas
            int diff = (s - r) % 5;
            RhombusType type = (diff == 1 || diff == 4) ? RhombusType::Thick : RhombusType::Thin;

            for (int kr = -GRID_RANGE; kr <= GRID_RANGE; ++kr) {
                for (int ks = -GRID_RANGE; ks <= GRID_RANGE; ++ks) {
                    // Equações das linhas da grade: x · e_r + gamma_r = k_r
                    double vr = kr - gamma[r];
                    double vs = ks - gamma[s];

                    Vector2D pt;
                    if (!intersectGridLines(e[r], vr, e[s], vs, pt)) continue;

                    // 4. Mapeamento para o vetor de inteiros 5D K = (k_0, k_1, k_2, k_3, k_4)
                    std::array<int, 5> K;
                    for (int j = 0; j < 5; ++j) {
                        if (j == r) K[j] = kr;
                        else if (j == s) K[j] = ks;
                        else {
                            // Função teto para determinar o domínio da célula 5D
                            K[j] = static_cast<int>(std::ceil(dot(pt, e[j]) + gamma[j]));
                        }
                    }

                    // 5. Projeção 5D -> 2D do vértice principal
                    Vector2D V = {0.0, 0.0};
                    for (int j = 0; j < 5; ++j) {
                        V = V + e[j] * K[j];
                    }

                    // Os 4 vértices do losango dual gerado no plano
                    Rhombus rho;
                    rho.type = type;
                    rho.vertices[0] = V;
                    rho.vertices[1] = V + e[r];
                    rho.vertices[2] = V + e[r] + e[s];
                    rho.vertices[3] = V + e[s];

                    rhombuses.push_back(rho);
                }
            }
        }
    }

    std::cout << "Total de losangos gerados (De Bruijn): " << rhombuses.size() << std::endl;

    // 6. Cálculo dos limites (Bounding Box) para centralizar e ajustar a escala do SVG
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& rho : rhombuses) {
        for (const auto& v : rho.vertices) {
            minX = std::min(minX, v.x);
            maxX = std::max(maxX, v.x);
            minY = std::min(minY, v.y);
            maxY = std::max(maxY, v.y);
        }
    }

    // 7. Renderização para o arquivo SVG com a tag <rect> corrigida
    const int SVG_SIZE = 1000;
    const double margin = 40.0;
    double scale = (SVG_SIZE - 2 * margin) / std::max(maxX - minX, maxY - minY);

    std::ofstream file("mosaico_penrose_debruijn.svg");
    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << SVG_SIZE 
         << "\" height=\"" << SVG_SIZE << "\" viewBox=\"0 0 " << SVG_SIZE << " " << SVG_SIZE << "\">\n";
    file << "  <rect width=\"100%\" height=\"100%\" fill=\"#0f111a\"/>\n";
    file << "  <g stroke=\"#ffffff\" stroke-width=\"0.6\" stroke-linecap=\"round\" stroke-linejoin=\"round\">\n";

    for (const auto& rho : rhombuses) {
        std::string color = (rho.type == RhombusType::Thick) ? "#2b5c8f" : "#d97736";

        file << "    <polygon points=\"";
        for (int i = 0; i < 4; ++i) {
            double px = margin + (rho.vertices[i].x - minX) * scale;
            double py = SVG_SIZE - (margin + (rho.vertices[i].y - minY) * scale); // Inverte eixo Y
            file << px << "," << py << (i < 3 ? " " : "");
        }
        file << "\" fill=\"" << color << "\" fill-opacity=\"0.85\" />\n";
    }

    file << "  </g>\n";
    file << "</svg>\n";

    std::cout << "Arquivo 'mosaico_penrose_debruijn.svg' gerado com sucesso!" << std::endl;
    return 0;
}