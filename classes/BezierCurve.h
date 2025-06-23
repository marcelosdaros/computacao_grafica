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
    float speed;               // Tempo entre dois pontos (quanto maior, mais devagar)

    void initializeBernsteinMatrix() {
        M = glm::mat4(
            -1,  3, -3, 1,
             3, -6,  3, 0,
            -3,  3,  0, 0,
             1,  0,  0, 0
        );
    }

    void generateBezierCurvePoints(int numPoints) {
        curvePoints.clear();
        initializeBernsteinMatrix();

        float piece = 1.0f / numPoints;
        float t;

        for (int i = 0; i < controlPoints.size() - 3; i += 3) {
            for (int j = 0; j < numPoints; j++) {
                t = j * piece;
                glm::vec4 T(pow(t, 3), t*t, t, 1.0f);

                glm::vec3 P0 = controlPoints[i];
                glm::vec3 P1 = controlPoints[i + 1];
                glm::vec3 P2 = controlPoints[i + 2];
                glm::vec3 P3 = controlPoints[i + 3];

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

    glm::vec3 update(float deltaTime) {
        lerpT += deltaTime / speed;

        if (currentIndex >= 0 && currentIndex < curvePoints.size() - 1) {
            glm::vec3 start, end;

            if (movingForward) {
                start = curvePoints[currentIndex];
                end = curvePoints[currentIndex + 1];
            } else {
                start = curvePoints[currentIndex + 1];
                end = curvePoints[currentIndex];
            }

            glm::vec3 interpolatedPos = glm::mix(start, end, lerpT);

            if (lerpT >= 1.0f) {
                lerpT = 0.0f;

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

        // Caso aconteça algum erro de índice, retornar o primeiro ponto
        return curvePoints.front();
    }
};

#endif