#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "include/classes_impl/Shader.h"
#include "include/stb_image.h"

#include <iostream>
#include <cmath>


void frameBufferSizeCallback(GLFWwindow *window, int width, int height);

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
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    //glad -- ucitavamo adrese opengl funkcija
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

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
    unsigned char* data = stbi_load("../resources/textures/container.jpg", &width, &height, &nrChannel, 0);

    if(data){
        //kacimo na trenutno aktivnu teksturu(objekat) ovaj data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to laod texture!" << std::endl;
    }
    stbi_image_free(data);

    //ovde pravimo drugu teksturu
    unsigned tex1id;
    glGenTextures(1,&tex1id);
    glBindTexture(GL_TEXTURE_2D,tex1id);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    //ucitavamo teksturu
    data = stbi_load("../resources/textures/awesomeface.png", &width, &height, &nrChannel, 0);

    if(data){
        //kacimo na trenutno aktivnu teksturu(objekat) ovaj data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Failed to laod texture!" << std::endl;
    }
    stbi_image_free(data);

    ourshader.use();
    ourshader.setUniform1int("tex0",0);
    ourshader.setUniform1int("tex1",1);


//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0f,0.2f,0.3f,1.0f);
    //petlja renderovanja
    while(!glfwWindowShouldClose(window)){

        glfwPollEvents();
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

//        //postavljamo vrednost;
//        //treba nam neka glatka, periodicna, oscilujuca funk
//        // sinx: R -> [-1,1] + 1 --> [0,2] /2 --> [0,1]
//        // f: (sinx + 1)/2 ili pak abs(sinx)
//        float timeValue = glfwGetTime();
//        float bluevalue = (sin(timeValue*4.0f) + 1)/2.0f;
//            //nalazimo tu uniformnu promenljivu
//        ourshader.setUniform4Float("unColor",0.5f,0.2f,bluevalue,1.0f);

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


