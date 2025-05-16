#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float fov = 45.0f;

    // Construtor
    Camera(glm::vec3 position, glm::vec3 up, float initYaw, float initPitch):
        cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(2.5f),
        mouseSensitivity(0.1f)
    {
        cameraPos = position;
        worldUp = up;
        yaw = initYaw;
        pitch = initPitch;
        updateCameraVectors();
    }

    // Retorna a matriz de visualização
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }

    // Movimento com teclado
    void updateCameraPos(GLFWwindow *window, float deltaTime) {
        float cameraSpeed = movementSpeed * deltaTime;
	    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	        cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	        cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += right * cameraSpeed;
    }

    // Movimento com mouse (yaw e pitch)
    void updateMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;
        yaw   += xoffset;
        pitch += yoffset;

        // Restringe o pitch para não "virar de cabeça para baixo"
        if (constrainPitch) {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }
        updateCameraVectors();
    }

    void updateMouseScroll(float yoffset) {
        if (fov >= 1.0f && fov <= 45.0f)
            fov -= yoffset;
        if (fov < 1.0f)
            fov = 1.0f;
        if (fov > 45.0f)
            fov = 45.0f;
    }

private:
    // Atualiza os vetores cameraFront, right e Up com base em Yaw e Pitch
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        // Recalcula right e Up
        right = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp = glm::normalize(glm::cross(right, cameraFront));
    }
};

#endif