#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Flux {

    enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

    class Camera {
    public:
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;

        float Yaw   = -90.0f;
        float Pitch =   0.0f;
        float Zoom  =  2.5f;

        float MovementSpeed    = 10.0f;
        float MouseSensitivity =  0.25f;

        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f))
            : Front(glm::vec3(0.0f, 0.0f, -1.0f))
        {
            Position = position;
            WorldUp  = glm::vec3(0.0f, 1.0f, 0.0f);
            updateCameraVectors();
        }

        glm::mat4 GetViewMatrix() const {
            return glm::lookAt(Position, Position + Front, Up);
        }

        void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
            float velocity = MovementSpeed * deltaTime;
            switch (direction) {
                case FORWARD:  Position += Front              * velocity; break;
                case BACKWARD: Position -= Front              * velocity; break;
                case LEFT:     Position -= Right              * velocity; break;
                case RIGHT:    Position += Right              * velocity; break;
                case UP:       Position += WorldUp            * velocity; break;
                case DOWN:     Position -= WorldUp            * velocity; break;
            }
        }

        void ProcessMouseMovement(float xoffset, float yoffset,
                                  bool constrainPitch = true) {
            Yaw   += xoffset * MouseSensitivity;
            Pitch += yoffset * MouseSensitivity;

            if (constrainPitch) {
                if (Pitch >  89.0f) Pitch =  89.0f;
                if (Pitch < -89.0f) Pitch = -89.0f;
            }
            updateCameraVectors();
        }

        void ProcessMouseScroll(float yoffset) {
            Zoom -= yoffset * 2.0f;
            if (Zoom <  10.0f) Zoom =  10.0f;
            if (Zoom > 120.0f) Zoom = 120.0f;
        }

    private:
        void updateCameraVectors() {
            glm::vec3 front;
            front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            front.y = sin(glm::radians(Pitch));
            front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            Front   = glm::normalize(front);
            Right   = glm::normalize(glm::cross(Front, WorldUp));
            Up      = glm::normalize(glm::cross(Right, Front));
        }
    };

}