#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

//za sve transformacije koristimo biblioteku glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "include/classes_impl/Shader.h"
#include "include/classes_impl/Camera.h"
#include "include/stb_image.h"
#include "include/classes_impl/model.h"

#include <iostream>
#include <vector>
#include <cmath>


void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window,double xpos, double ypos);
void processInput(GLFWwindow *window); // za kontinualno rukovanje tastaturom
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);
void DrawImGui();

unsigned int ucitaj_prostoriju();
unsigned int ucitaj_skybox();

//podesevanja
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1000;

Camera camera(glm::vec3(-21.0f,-2.0f,9.0f));
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
float deltaTime = 1.0f; // tastatura on/off

//imgui setup
bool m_EnableImgui = false;

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
    glfwSetKeyCallback(window, keyCallBack);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    //glad -- ucitavamo adrese opengl funkcija
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }
    //kazemo opengl da koristi z_buffer
    glEnable(GL_DEPTH_TEST);

    //Inicijalizujemo imGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    //ucitavamo sejdere
    Shader prostorijaShader("../resources/shaders/prostorija.vs","../resources/shaders/prostorija.fs");
    Shader ranacShader("../resources/shaders/model_ranac.vs","../resources/shaders/model_ranac.fs");
    Shader skyboxShader("../resources/shaders/skybox.vs","../resources/shaders/skybox.fs");


    //ucitavamo modele
    //kazemo stbi_image da flipuje teksturu po y-osi
    stbi_set_flip_vertically_on_load(true);
    Model ranacModel("../resources/objects/backpack/backpack.obj");
    stbi_set_flip_vertically_on_load(false);

    Model gunModel("../resources/objects/gun/Handgun_obj.obj");
    Shader gunShader("../resources/shaders/gun.vs","../resources/shaders/gun.fs");

    //ovde ucitavamo prostoriju
    unsigned int VAO_prostorija = ucitaj_prostoriju();
    unsigned texFloor = loadTexture("../resources/textures/floor.jpg");
    prostorijaShader.use();
    prostorijaShader.setUniform1int("texture0",0);

    //ovde ucitavamo skybox i teksture
    unsigned int VAO_skybox = ucitaj_skybox();
    std::vector<std::string> space_skybox = {
            "../resources/textures/skybox/bay_ft.jpg", //front
            "../resources/textures/skybox/bay_bk.jpg", //back
            "../resources/textures/skybox/bay_up.jpg", //up
            "../resources/textures/skybox/bay_dn.jpg", //down
            "../resources/textures/skybox/bay_rt.jpg", //right
            "../resources/textures/skybox/bay_lf.jpg", //left
    };
    unsigned int cubemapTextures = loadCubemap(space_skybox);
    skyboxShader.use();
    skyboxShader.setUniform1int("skybox",0);



//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    //petlja renderovanja
    while(!glfwWindowShouldClose(window)){

        glfwPollEvents();
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//ISCRTAVAMO PROSTORIJU
        prostorijaShader.use();
        //view/projection transformacije
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
        glm::mat4 view = camera.getViewMatrix();
        prostorijaShader.setUniformMat4("projection",projection);
        prostorijaShader.setUniformMat4("view",view);
        glm::mat4 model = glm::mat4(1.0f);
        float scale_value = 10.0f;
        model = glm::scale(model,glm::vec3(scale_value + 15.0f,scale_value,scale_value));
        prostorijaShader.setUniformMat4("model",model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texFloor);
        glBindVertexArray(VAO_prostorija);
        glDrawArrays(GL_TRIANGLES,0,36);


        ranacShader.use();
        //view/projection transformacije
        projection = glm::perspective(glm::radians(camera.Fov),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
        view = camera.getViewMatrix();
        ranacShader.setUniformMat4("projection",projection);
        ranacShader.setUniformMat4("view",view);
        model = glm::mat4(1.0f);
        ranacShader.setUniformMat4("model",model);
        //ovde kazemo da zelimo da se nacrta nas model pomocu nekog shadera
        ranacModel.Draw(ranacShader);


//ISCRAVAMO MODELE COVECULJAKA
    //TODO

//ICRTAVAMO PISTOLJ
//        gunShader.use();
//        projection = glm::perspective(glm::radians(camera.Fov),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
//        view  = camera.getViewMatrix();
//        gunShader.setUniformMat4("projection",projection);
//        gunShader.setUniformMat4("view",view);
//        model = glm::mat4(1.0f);
////        float angle = 45.0f;
////        model = glm::rotate(model,glm::radians(angle),glm::vec3(0.0f,1.0f,0.0f));
////        model = glm::translate(model,camera.Position + camera.Front);
//        gunShader.setUniformMat4("model",model);
//        gunModel.Draw(gunShader);


//SKYBOX PODESAVANJA
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.getViewMatrix())); // remove translation from the view matrix
        skyboxShader.setUniformMat4("view", view);
        skyboxShader.setUniformMat4("projection", projection);
        // skybox cube
        glBindVertexArray(VAO_skybox);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextures);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

// ISCRTAVAMO IMGUI
        if(m_EnableImgui)
            DrawImGui();

        glfwSwapBuffers(window);
    }



    //deinit
    prostorijaShader.deleteProgram();
    gunShader.deleteProgram();
    skyboxShader.deleteProgram();
    ranacShader.deleteProgram();

    //deinit imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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

    if(!m_EnableImgui)
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

unsigned int ucitaj_prostoriju() {


    //pravimo kvadar u 3D-u
    float vertices[] = {
            //prednji pravougaonik
            //first triangle
            1.0, 0.5, 1.0,   0.0, 0.0,//top right
            -1.0, 0.5 , 1.0,    0.0, 0.0,//top left
            1.0, -0.5, 1.0 ,    0.0, 0.0,// bottom right
            //second triangle
            1.0, -0.5, 1.0,     0.0, 0.0,//bottom right
            -1.0, 0.5, 1.0,     0.0, 0.0,//top left
            -1.0, -0.5, 1.0,     0.0, 0.0, // bottom left

            //desni pravougaonik
            1.0, 0.5, 1.0,     0.0, 0.0,
            1.0, -0.5, 1.0,     0.0, 0.0,
            1.0, 0.5, -6.0,    0.0, 0.0,
            1.0, 0.5, -6.0,    0.0, 0.0,
            1.0, -0.5, 1.0,     0.0, 0.0,
            1.0, -0.5, -6.0,    0.0, 0.0,

            //levi pravouganik
            -1.0, 0.5 , 1.0,   0.0, 0.0,
            -1.0, -0.5, 1.0,    0.0, 0.0,
            -1.0, 0.5, -6.0,    0.0, 0.0,
            -1.0, -0.5, 1.0,    0.0, 0.0,
            -1.0, 0.5, -6.0,   0.0, 0.0,
            -1.0, -0.5, -6.0,    0.0, 0.0,

            //zadnja stranica
            1.0, 0.5, -6.0,       0.0, 0.0,
            -1.0, 0.5 , -6.0,     0.0, 0.0,
            1.0, -0.5, -6.0 ,     0.0, 0.0,
            1.0, -0.5, -6.0,      0.0, 0.0,
            -1.0, 0.5, -6.0,      0.0, 0.0,
            -1.0, -0.5, -6.0,     0.0, 0.0,

            //donja strana
            1.0, -0.5, 1.0, 1.0 ,0.0,
            -1.0, -0.5, 1.0,    0.0, 0.0,
            -1.0, -0.5, -6.0,    0.0, 1.0,
            1.0, -0.5, 1.0,     1.0 ,0.0,
            1.0, -0.5, -6.0,    1.0, 1.0,
            -1.0, -0.5, -6.0,   0.0, 1,0,

            //gornja strana
            -1.0, 0.5 , 1.0,    0.0, 0.0,
            1.0, 0.5, 1.0,       0.0, 0.0,
            -1.0, 0.5, -6.0,     0.0, 0.0,
            -1.0, 0.5, -6.0,    0.0, 0.0,
            1.0, 0.5, 1.0,       0.0, 0.0,
            1.0, 0.5, -6.0,     0.0, 0.0,

    };



    unsigned int VAO,VBO;
    //kreirmao objekat koji ce da kaze sta znace podaci iz VBO
    glGenVertexArrays(1,&VAO);
    //kreiramo baffer preko kog saljemo podatke ka gpu
    glGenBuffers(1,&VBO);

    //aktiviramo ovaj objekat
    glBindVertexArray(VAO);


    //aktiviramo ovaj bafer
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    //smestamo podatke u ovaj bafer
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);


    //kazemo sta znace podaci iz vbo
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(3*sizeof (float )));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    return VAO;
}
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int ucitaj_skybox() {

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    return skyboxVAO;
}

void DrawImGui() {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("ImGui Settings");

        //tekst
        //color
        //slider
        //button

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void keyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_F1 & action == GLFW_PRESS) {
        m_EnableImgui = !m_EnableImgui;

        if(m_EnableImgui){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else{
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

    }
}


