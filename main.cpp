#include "glad/glad.h"
#include <GLFW/glfw3.h>

//za sve transformacije koristimo biblioteku glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "include/classes_impl/Shader.h"
#include "include/classes_impl/Camera.h"
#include "include/stb_image.h"
#include "include/classes_impl/Model.h"

#include <iostream>
#include <cmath>


void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window,double xpos, double ypos);
void processInput(GLFWwindow *window); // za kontinualno rukovanje tastaturom
unsigned int loadTexture(const char *path);

//podesevanja
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f,0.0f,3.0f));
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
float deltaTime = 1.0f; // tastatura on/off

int main(){

    //inicijalizujemo glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //kreiranje prozora
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"RG_project", nullptr, nullptr);
    if (window == NULL){
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    //CALLBACK functions and cursor mode
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetCursorPosCallback(window,mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    //glad -- ucitavamo adrese opengl funkcija
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    //kazemo opengl da koristi z_buffer
    glEnable(GL_DEPTH_TEST);

    //kazemo stbi_image da flipuje teksturu po y-osi
    stbi_set_flip_vertically_on_load(true);


    Shader ourshader("../resources/shaders/model_ranac.vs",
                     "../resources/shaders/model_ranac.fs");

    //ucitavamo model
    Model ourModel("../resources/objects/backpack/backpack.obj");




//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.0f,0.2f,0.3f,1.0f);
    //petlja renderovanja
    while(!glfwWindowShouldClose(window)){

        glfwPollEvents();
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        ourshader.use();
        //view/projection transformacije
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
        glm::mat4 view = camera.getViewMatrix();
        ourshader.setUniformMat4("projection",projection);
        ourshader.setUniformMat4("view",view);

        glm::mat4 model = glm::mat4(1.0f);
        ourshader.setUniformMat4("model",model);

        //ovde kazemo da zelimo da se nacrta nas model pomocu nekog shadera
        ourModel.Draw(ourshader);

        glfwSwapBuffers(window);
    }

    //deinit
    ourshader.deleteProgram();
    glfwTerminate();

    return 0;
}

void frameBufferSizeCallback(GLFWwindow *window, int width, int height) {
    //funkcija koja ze reaguje na promene velicine prozora koji renderujemo
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.mouseCallBack(xoffset, yoffset);
}

void scrollCallback(GLFWwindow *window, double xpos, double ypos) {

    camera.ScrollCallBack(ypos);
}
void processInput(GLFWwindow *window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        std::cout << "End of program!" << std::endl;
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}



