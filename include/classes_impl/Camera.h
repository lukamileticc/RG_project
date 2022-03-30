#ifndef CAS5_COLORS_BASIC_LIGHTING_MATERIALS_CAMERA_H
#define CAS5_COLORS_BASIC_LIGHTING_MATERIALS_CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};


const float YAW = -90.0f;
const float PITCH = 0.0f;
const float FOV = 45.0f;
const float SENSITIVITY = 0.1;
const float KEYSPEED = 0.090;


class Camera {
public:
    //kamera atributi
    glm::vec3 Position;
    glm::vec3 Up;
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    //ojlerovi uglovi
    float Yaw;
    float Pitch;
    //opcije kamere
    float Fov;
    float Sensitivity;
    float KeySpeed;


    //kostrukotr sa vektorima
    Camera(glm::vec3 postition = glm::vec3(0.0f,0.0f,0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH)
           : Front(glm::vec3(0.0f,0.0f,-1.0f)),
           Fov(FOV),Sensitivity(SENSITIVITY),KeySpeed(KEYSPEED)
    {
        Position = postition;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

        updateCameraVectors();
    }
    //konstruktor sa skalarima
    Camera(float posX, float posY, float posZ,
           float upX, float upY, float upZ,
           float yaw, float pitch)
           : Front(glm::vec3(0.0f,0.0f,-1.0f)),
             Fov(FOV),Sensitivity(SENSITIVITY),KeySpeed(KEYSPEED)
    {

        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    void mouseCallBack(double xoffset, double yoffset) {
//        float xoffset = xpos - lastX;
//        float yoffset = lastY - ypos;
//        lastX = xpos;
//        lastY = ypos;

        xoffset *= Sensitivity;
        yoffset *= Sensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        //ogranicavamo u gao teta(pitch) od -Pi/2 do Pi/2;
        Pitch = std::min(Pitch,89.0f);
        Pitch = std::max(Pitch,-89.0f);

        updateCameraVectors();
    }
    void ScrollCallBack(double ypos) {

        Fov -= (float)ypos;
        Fov = std::max(1.0f, Fov);
        Fov = std::min(45.0f, Fov);
    }
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = KeySpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    glm::mat4 getViewMatrix(){
        return  glm::lookAt(Position, Position + Front,Up);
    }

private:
    void updateCameraVectors(){
        //racunamo novi fron vektor
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        //ponovno izracunavanje desnog i gornjeg vektora
        Right = glm::normalize(glm::cross(Front,WorldUp));
        Up = glm::normalize(glm::cross(Right,Front));
    }

};


#endif //CAS5_COLORS_BASIC_LIGHTING_MATERIALS_CAMERA_H
