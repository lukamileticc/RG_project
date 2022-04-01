#include "glad/glad.h"
#include <GLFW/glfw3.h>

//za sve transformacije koristimo biblioteku glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "include/classes_impl/Shader.h"
#include "include/classes_impl/Camera.h"
#include "include/stb_image.h"

#include <iostream>
#include <cmath>


void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window,double xpos, double ypos);
void processInput(GLFWwindow *window); // za kontinualno rukovanje
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
//    glEnable(GL_DEPTH_TEST);


    Shader ourshader("../resources/shaders/vertex_Shader.vs",
                     "../resources/shaders/fragment_Shader.fs");


    float vertices[] = {
            //   x   y      z     offset
            -0.5f , -0.5f, 0.0f, 0.0f, 0.0f, //bottom left
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,//bottom right
            -0.5f,0.5f,0.0f, 0.0f, 1.0f,//top left
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f //top right
    };
    unsigned  indices[] {
            //frist
            0,1,2,
            // second
            1,2,3
    };

    unsigned VBO,VAO,EBO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //ovde kazemo grafickoj sta ti podaci zapravo predstavljaju
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float),(void*)0);
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5*sizeof(float),(void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    //Deaktiviramo ovaj objekat
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);


    stbi_set_flip_vertically_on_load(true);
    //ucitavamo prvu teksturu
    unsigned tex0id = loadTexture("../resources/textures/container.jpg");
    unsigned tex1id = loadTexture("../resources/textures/awesomeface.png");



    ourshader.use();
    ourshader.setUniform1int("tex0",0);
    ourshader.setUniform1int("tex1",1);


//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0f,0.2f,0.3f,1.0f);
    //petlja renderovanja
    while(!glfwWindowShouldClose(window)){

        glfwPollEvents();
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT);

        //ovde aktiviramo teksturu u svakom frejmu
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0id);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,tex1id);

        ourshader.use();

        float TimeValue = glfwGetTime();
        float p = abs(sin(TimeValue));
        ourshader.setUniform1Float("p",p);

        //view/projection transformacije

        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
        glm::mat4 view = camera.getViewMatrix();
        ourshader.setUniformMat4("projection",projection);
        ourshader.setUniformMat4("view",view);

        glm::mat4 model = glm::mat4(1.0f);
        ourshader.setUniformMat4("model",model);


        //ovde kazemo opengl da treba da nacrta sve sto smo poslali na gpu
        //on ce kreirati default-ni shader
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6 ,GL_UNSIGNED_INT, 0);


        glfwSwapBuffers(window);
    }

    //deinizijalizacija
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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

unsigned int loadTexture(const char *path) {

    //ovde pravimo prvu teksturu
    unsigned tex0id;
    glGenTextures(1,&tex0id);
    glBindTexture(GL_TEXTURE_2D, tex0id);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    //ucitavamo teksturu
    int width, height, nrChannel;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannel, 0);

    if(data){
        GLenum format;
        if(nrChannel == 1)
            format = GL_RED;
        else if(nrChannel == 3)
            format = GL_RGB;
        else if(nrChannel == 4)
            format = GL_RGBA;
        //kacimo na trenutno aktivnu teksturu(objekat) ovaj data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cerr << "Failed to laod texture:" << path << std::endl;
    }

    stbi_image_free(data);

    return tex0id;
}


