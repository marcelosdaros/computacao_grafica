#ifndef BEZIER_HPP
#define BEZIER_HPP

#include <glm/glm.hpp>
#include <vector>
#include <cmath>

class Bezier {
private:
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> curvePoints;
    glm::mat4 M;  // Bernstein matrix

    int currentIndex = 0;      // Índice atual na curva
    float lerpT = 0.0f;        // Tempo acumulado
    bool movingForward = true; // Controla o sentido da curva (ida ou volta)
    float speed;               // Tempo entre dois pontos

    void initializeBernsteinMatrix() {
        M = glm::mat4(
            -1,  3, -3, 1,
             3, -6,  3, 0,
            -3,  3,  0, 0,
             1,  0,  0, 0
        );
    }

    // Gera todos os pontos intermediários
    void generateBezierCurvePoints(int numPoints) {
        curvePoints.clear();
        initializeBernsteinMatrix(); // Inicializa a matriz

        float piece = 1.0f / numPoints;
        float t;

        // Percorre cada segmento de 4 pontos de controle
        for (int i = 0; i < controlPoints.size() - 3; i += 3) {
            for (int j = 0; j < numPoints; j++) {
                t = j * piece;
                glm::vec4 T(pow(t, 3), t*t, t, 1.0f); // Vetor com as potências de t (t^3, t^2, t^1, 1)

                glm::vec3 P0 = controlPoints[i];
                glm::vec3 P1 = controlPoints[i + 1];
                glm::vec3 P2 = controlPoints[i + 2];
                glm::vec3 P3 = controlPoints[i + 3];

                // Cria a matriz G, calcula o ponto na curva e armazena
                glm::mat4x3 G(P0, P1, P2, P3);
                glm::vec3 point = G * M * T;
                curvePoints.push_back(point);
            }
        }
    }

public:
    Bezier(const std::vector<glm::vec3>& points, int numPoints = 100, float movementSpeed = 0.1f)
        : controlPoints(points), speed(movementSpeed)
    {
        generateBezierCurvePoints(numPoints);
    }

    // Atualiza a posição ao longo da curva
    glm::vec3 update(float deltaTime) {
        lerpT += deltaTime / speed;

        // Verifica se o índice está dentro dos limites válidos
        if (currentIndex >= 0 && currentIndex < curvePoints.size() - 1) {
            glm::vec3 start, end;

            // Verifica a direção (ida ou volta)
            if (movingForward) {
                start = curvePoints[currentIndex];
                end = curvePoints[currentIndex + 1];
            } else {
                start = curvePoints[currentIndex + 1];
                end = curvePoints[currentIndex];
            }

            // Interpolação linear entre dois pontos
            glm::vec3 interpolatedPos = glm::mix(start, end, lerpT);

            // Se a interpolação chegou ao fim, avança para o próximo segmento
            if (lerpT >= 1.0f) {
                lerpT = 0.0f;

                // Se chegou ao final, inverte a direção (e vice-versa)
                if (movingForward) {
                    currentIndex++;
                    if (currentIndex >= curvePoints.size() - 1) {
                        currentIndex = curvePoints.size() - 2;
                        movingForward = false;
                    }
                } else {
                    currentIndex--;
                    if (currentIndex < 0) {
                        currentIndex = 0;
                        movingForward = true;
                    }
                }
            }

            return interpolatedPos;
        }

        // Em caso de erro, retorna o primeiro ponto
        return curvePoints.front();
    }
};

#endif