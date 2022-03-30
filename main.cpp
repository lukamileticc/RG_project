#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <iostream>


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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
    //TODO CALLBACK functions and cursor mode

    //glad -- ucitavamo adrese opengl funkcija
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }


    glClearColor(1.0f,0.2f,0.3f,1.0f);

    //petlja renderovanja
    while(!glfwWindowShouldClose(window)){

        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);


        glfwSwapBuffers(window);
    }

    //deinizijalizacija
    glfwTerminate();

    return 0;
}

