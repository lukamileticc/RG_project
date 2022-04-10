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
unsigned int ucitaj_kocke();
unsigned int ucitaj_light_cube();


//podesevanja
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1000;

Camera camera(glm::vec3(-21.0f,-2.0f,9.0f));
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
float deltaTime = 1.0f; // tastatura on/off

//imgui setup
bool m_EnableImgui = false;

bool mouse_click_enabled = false;


//Svetlo
struct PointLight{
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

};

glm::vec3 lightPos(-1.2f, 0.0f, -6.0f);

bool spot_light_indicator = false;
bool advance_lighting_indicator = false;

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

    //face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_CULL_FACE);

    //Inicijalizujemo imGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    //ucitavamo sejdere
//    Shader prostorijaShader("../resources/shaders/prostorija.vs","../resources/shaders/prostorija.fs");
    Shader puskaShader("../resources/shaders/puska.vs","../resources/shaders/puska.fs");
    Shader skyboxShader("../resources/shaders/skybox.vs","../resources/shaders/skybox.fs");
//    Shader kockaShader("../resources/shaders/kocka.vs","../resources/shaders/kocka.fs");
    Shader lightingShader("../resources/shaders/swat.vs","../resources/shaders/swat.fs");
    Shader bulletShader("../resources/shaders/swat.vs","../resources/shaders/swat.fs");
    Shader lightCubeShader("../resources/shaders/light_cube.vs", "../resources/shaders/light_cube.fs");


    //ucitavamo modele
    Model puskaModel("../resources/objects/ak47/AK47.obj");
    Model swatModel("../resources/objects/green_swat/green swat.obj");
    swatModel.SetShaderTextureNamePrefix("material.");
    glm::vec3 swatPositions[] = {
            glm::vec3(17.0f, -4.5f, -15.0f),
            glm::vec3( -17.0f,  -4.5f, -37.0f),
            glm::vec3(-1.5f, -4.5f, -42.5f),
            glm::vec3(-7.8f, -4.5f, -45.3f),
            glm::vec3( 13.4f, -4.5f, -11.5f),
            glm::vec3(-17.0f, -4.5f, -15.0f),
            glm::vec3( 12.0f,  -4.5f, -37.0f),
            glm::vec3(14.5f, -4.5f, -42.5f),
            glm::vec3(19.8f, -4.5f, -45.3f),
            glm::vec3( -13.4f, -4.5f, -11.5f),
    };

    stbi_set_flip_vertically_on_load(true);
    Model bulletModel("../resources/objects/bullet/bullet.obj");
    stbi_set_flip_vertically_on_load(false);

    //ucitavamo point_light_cube koji prestavlja point izvor svetlosti
    unsigned int VAO_light_cube = ucitaj_light_cube();
    glm::vec3 pointLightPositions[] = {
            glm::vec3( 0.7f,  0.2f,  -15.0f),
            glm::vec3( 2.3f, -3.3f, -25.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3( 0.0f,  0.0f, -6.0f)
    };

    PointLight pointLight;
    pointLight.ambient = glm::vec3(0.8, 0.4, 0.2);
    pointLight.diffuse = glm::vec3(0.6, 0.5, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;
    pointLight.position = lightPos;


    //ovde ucitavamo prostoriju
    unsigned int VAO_prostorija = ucitaj_prostoriju();
    unsigned texFloor = loadTexture("../resources/textures/floor.jpg");
    lightingShader.use();
    lightingShader.setUniform1int("material.texture_diffuse1",0);

    //ovde ucitavamo kocku
    unsigned int VAO_kocke = ucitaj_kocke();
    unsigned texKocke = loadTexture("../resources/textures/brick.jpg");
    unsigned texKockeFrame = loadTexture("../resources/textures/metal_frame.jpg");
    lightingShader.use();
    lightingShader.setUniform1int("material.texture_diffuse1",0);
//    lightingShader.setUniform1int("material.texture_specular1",1);
    //zelimo da imamo vise kocki na razlicitm pozicijama
    glm::vec3 cubePositions[] = {
            glm::vec3(17.0f, -4.5f, -30.0f),
            glm::vec3( -17.0f,  -4.5f, -30.0f),
            glm::vec3(-1.5f, -4.5f, -35.5f),
            glm::vec3(-7.8f, -4.5f, -22.3f),
            glm::vec3( 13.4f, -4.5f, -15.5f),
            glm::vec3(-8.7f,  -4.5f, -7.5f),
            glm::vec3( 11.3f, -4.5f, -6.5f),
            glm::vec3( 19.5f,  -4.5f, -11.5f),
            glm::vec3( -8.5f,  -4.5f, -1.5f),
            glm::vec3(-16.3f,  -4.5f, -1.5f)
    };



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

        lightingShader.use();
        //podesavanje direkcionog
        lightingShader.setUniformVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setUniformVec3("dirLight.ambient", 0.005f, 0.005f, 0.005f);
        lightingShader.setUniformVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setUniformVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        lightingShader.setUniformVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setUniformVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setUniformVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setUniformVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setUniform1Float("pointLights[0].constant", 1.0f);
        lightingShader.setUniform1Float("pointLights[0].linear", 0.09);
        lightingShader.setUniform1Float("pointLights[0].quadratic", 0.032);
        // point light 2
        lightingShader.setUniformVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setUniformVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setUniformVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setUniformVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setUniform1Float("pointLights[1].constant", 1.0f);
        lightingShader.setUniform1Float("pointLights[1].linear", 0.09);
        lightingShader.setUniform1Float("pointLights[1].quadratic", 0.032);
        // point light 3
        lightingShader.setUniformVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setUniformVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setUniformVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setUniformVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setUniform1Float("pointLights[2].constant", 1.0f);
        lightingShader.setUniform1Float("pointLights[2].linear", 0.09);
        lightingShader.setUniform1Float("pointLights[2].quadratic", 0.032);
        // point light 4
        lightingShader.setUniformVec3("pointLights[3].position", pointLightPositions[3]);
        lightingShader.setUniformVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setUniformVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setUniformVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setUniform1Float("pointLights[3].constant", 1.0f);
        lightingShader.setUniform1Float("pointLights[3].linear", 0.09);
        lightingShader.setUniform1Float("pointLights[3].quadratic", 0.032);
        // spotLight
        lightingShader.setUniformVec3("spotLight.position", camera.Position);
        lightingShader.setUniformVec3("spotLight.direction", camera.Front);
        lightingShader.setUniformVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setUniformVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setUniformVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setUniform1Float("spotLight.constant", 1.0f);
        lightingShader.setUniform1Float("spotLight.linear", 0.09);
        lightingShader.setUniform1Float("spotLight.quadratic", 0.032);
        lightingShader.setUniform1Float("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setUniform1Float("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


        if(spot_light_indicator)
            lightingShader.setUniform1Float("spot_indikator",1);
        else
            lightingShader.setUniform1Float("spot_indikator",0);

        if(advance_lighting_indicator)
            lightingShader.setUniform1Float("advance_lighting",true);
        else
            lightingShader.setUniform1Float("advance_lighting",false);



//ISCRTAVAMO PROSTORIJU
        lightingShader.use();
        //view/projection transformacije
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
        glm::mat4 view = camera.getViewMatrix();
        lightingShader.setUniformMat4("projection",projection);
        lightingShader.setUniformMat4("view",view);
        glm::mat4 model = glm::mat4(1.0f);
        float scale_value = 10.0f;
        model = glm::scale(model,glm::vec3(scale_value + 15.0f,scale_value,scale_value));
        lightingShader.setUniformMat4("model",model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texFloor);
        glBindVertexArray(VAO_prostorija);
        glDrawArrays(GL_TRIANGLES,0,30);


//ISCRTAVAMO KOCKE
        lightingShader.use();
        projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        lightingShader.setUniformMat4("projection", projection);
        //ovde cemo postaviti pomerajucu kameru
        //drugi argument gde kamera gleda
        view = camera.getViewMatrix();
        lightingShader.setUniformMat4("view",view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texKocke);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D,texKockeFrame);

        //ovde kazemo opengl da treba da nacrta sve to sto smo gore poslali na gpu
        //ovde opet aktiviramo interpretaciju podataka
        glBindVertexArray(VAO_kocke);
        for(int i = 0; i < 10; i++){
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
//            float angle = glfwGetTime() * (i + 1);
//            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.5f,0.7f,0.0f));
            model = glm::scale(model,glm::vec3(3.0f,3.0f,3.0f));
            lightingShader.setUniformMat4("model",model);
            glDrawArrays(GL_TRIANGLES,0,36);
//            glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
//            std::cout << "Nacrtao sam " << i << std::endl;
        }


//ISCRTAVAMO PUSKU
        puskaShader.use();
        //view/projection transformacije
        projection = glm::perspective(glm::radians(camera.Fov),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f,100.0f);
        view = camera.getViewMatrix();
        puskaShader.setUniformMat4("projection",projection);
        puskaShader.setUniformMat4("view",view);
        model = glm::mat4(1.0f);
        model = glm::translate(model,camera.Position + glm::vec3(0.5f,-0.5f,-0.5f));
        model = glm::rotate(model, glm::radians(100.0f),glm::vec3(0.0f,1.0f,0.0f));
        model = glm::scale(model,glm::vec3(0.03f));
        puskaShader.setUniformMat4("model",model);
        //ovde kazemo da zelimo da se nacrta nas model pomocu nekog shadera
        puskaModel.Draw(puskaShader);


//ISCRAVAMO MODELE COVECULJAKA
        lightingShader.use();
        lightingShader.setUniformMat4("projection",projection);
        lightingShader.setUniformMat4("view",view);

        float current_time = glfwGetTime();

        for(int i = 0; i < 10; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, swatPositions[i] + glm::vec3(sin(current_time)*5,0.0f,0.0f));
            model = glm::rotate(model, glm::radians(current_time*100), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.03f));
            lightingShader.setUniformMat4("model", model);
            swatModel.Draw(lightingShader);
        }

//ISCRTAVANJE METAKA
//        if(mouse_click_enabled){
//            bulletShader.use();
//            bulletShader.setUniformMat4("projection",projection);
//            bulletShader.setUniformMat4("view",view);
//
//            float current_time = glfwGetTime();
//
//            model = glm::mat4(1.0f);
//            model = glm::translate(model,camera.Position + glm::vec3(0.0f,0.0f,-current_time*6));
//            model = glm::rotate(model,glm::radians(195.0f),glm::vec3(0.0f,1.0f,0.0f));
//            model = glm::scale(model, glm::vec3(3.0f));
//            bulletShader.setUniformMat4("model", model);
//            bulletModel.Draw(bulletShader);
//        }


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



//ISCRTAVAM IZVOR POINT_LIGHT_SVETLOSTI
        lightCubeShader.use();
        lightCubeShader.setUniformMat4("projection", projection);
        view = camera.getViewMatrix();
        lightCubeShader.setUniformMat4("view", view);
        glBindVertexArray(VAO_light_cube);
        for(int i = 0; i < 4; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.5f)); // a smaller cube
            lightCubeShader.setUniformMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

// ISCRTAVAMO IMGUI
        if(m_EnableImgui)
            DrawImGui();

        glfwSwapBuffers(window);
    }



    //deinit
//    prostorijaShader.deleteProgram();
    skyboxShader.deleteProgram();
    puskaShader.deleteProgram();
//    kockaShader.deleteProgram();
    lightingShader.deleteProgram();
    bulletShader.deleteProgram();
    lightCubeShader.deleteProgram();

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

    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.KeySpeed = 0.8;
    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.KeySpeed = 0.2;

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
            1.0, 0.5, 1.0,     0.0f,0.0f,-1.0f, 0.0, 0.0,
            -1.0, 0.5 , 1.0,     0.0f,0.0f,-1.0f, 0.0, 0.0,
            1.0, -0.5, 1.0 ,     0.0f,0.0f,-1.0f, 0.0, 0.0,
            //second triangle
            1.0, -0.5, 1.0,      0.0f,0.0f,-1.0f, 0.0, 0.0,
            -1.0, 0.5, 1.0,      0.0f,0.0f,-1.0f, 0.0, 0.0,
            -1.0, -0.5, 1.0,      0.0f,0.0f,-1.0f, 0.0, 0.0,

            //desni pravougaonik
            1.0, 0.5, 1.0,      -1.0f,0.0f, 0.0f, 0.0, 0.0,
            1.0, -0.5, 1.0,      -1.0f,0.0f, 0.0f,0.0, 0.0,
            1.0, 0.5, -6.0,     -1.0f,0.0f, 0.0f,0.0, 0.0,
            1.0, 0.5, -6.0,    -1.0f,0.0f, 0.0f,0.0, 0.0,
            1.0, -0.5, 1.0,      -1.0f,0.0f, 0.0f,0.0, 0.0,
            1.0, -0.5, -6.0,     -1.0f,0.0f, 0.0f,0.0, 0.0,

            //levi pravouganik
            -1.0, 0.5 , 1.0,      1.0f,0.0f, 0.0f, 0.0, 0.0,
            -1.0, -0.5, 1.0,    1.0f,0.0f, 0.0f, 0.0, 0.0,
            -1.0, 0.5, -6.0,     1.0f,0.0f, 0.0f, 0.0, 0.0,
            -1.0, -0.5, 1.0,     1.0f,0.0f, 0.0f, 0.0, 0.0,
            -1.0, 0.5, -6.0,    1.0f,0.0f, 0.0f, 0.0, 0.0,
            -1.0, -0.5, -6.0,     1.0f,0.0f, 0.0f, 0.0, 0.0,

            //zadnja stranica
            1.0, 0.5, -6.0,        0.0f,0.0f, 1.0f, 0.0, 0.0,
            -1.0, 0.5 , -6.0,      0.0f,0.0f, 1.0f,0.0, 0.0,
            1.0, -0.5, -6.0 ,      0.0f,0.0f, 1.0f,0.0, 0.0,
            1.0, -0.5, -6.0,       0.0f,0.0f, 1.0f,0.0, 0.0,
            -1.0, 0.5, -6.0,       0.0f,0.0f, 1.0f,0.0, 0.0,
            -1.0, -0.5, -6.0,      0.0f,0.0f, 1.0f,0.0, 0.0,

            //donja strana
            1.0, -0.5, 1.0,  0.0f,1.0f, 0.0f, 1.0 ,0.0,
            -1.0, -0.5, 1.0,     0.0f,1.0f, 0.0f, 0.0, 0.0,
            -1.0, -0.5, -6.0,    0.0f,1.0f, 0.0f, 0.0, 1.0,
            1.0, -0.5, 1.0,      0.0f,1.0f, 0.0f,1.0 ,0.0,
            1.0, -0.5, -6.0,     0.0f,1.0f, 0.0f,1.0, 1.0,
            -1.0, -0.5, -6.0,   0, 0.0f,1.0f, 0.0f,0.0, 1,

//            //gornja strana
//            -1.0, 0.5 , 1.0,     0.0f,0.0f, 0.0f, 0.0, 0.0,
//            1.0, 0.5, 1.0,        0.0f,0.0f, 0.0f, 0.0, 0.0,
//            -1.0, 0.5, -6.0,      0.0f,0.0f, 0.0f,0.0, 0.0,
//            -1.0, 0.5, -6.0,     0.0f,0.0f, 0.0f,0.0, 0.0,
//            1.0, 0.5, 1.0,        0.0f,0.0f, 0.0f,0.0, 0.0,
//            1.0, 0.5, -6.0,      0.0f,0.0f, 0.0f,0.0, 0.0,

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(3*sizeof (float )));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(6*sizeof (float )));
    glEnableVertexAttribArray(2);

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


    if(key == GLFW_KEY_SPACE & action == GLFW_PRESS)
        camera.Position += glm::vec3(0.0f, 2.0f, 0.0f);
    if (key == GLFW_KEY_SPACE & action == GLFW_RELEASE)
        camera.Position -= glm::vec3(0.0f, 2.0f, 0.0f);
    if(key == GLFW_KEY_LEFT_CONTROL & action == GLFW_PRESS)
        camera.Position -= glm::vec3(0.0f, 2.0f, 0.0f);
    if (key == GLFW_KEY_LEFT_CONTROL & action == GLFW_RELEASE)
        camera.Position += glm::vec3(0.0f, 2.0f, 0.0f);
    if (key == GLFW_KEY_ENTER & action == GLFW_PRESS)
        mouse_click_enabled = true;
    if (key == GLFW_KEY_ENTER & action == GLFW_RELEASE)
        mouse_click_enabled = false;


    //spotlight control
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
        spot_light_indicator = true;
    if (key == GLFW_KEY_F && action == GLFW_RELEASE)
        spot_light_indicator = false;

    //advance_lightin_control
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
        advance_lighting_indicator = true;
    if (key == GLFW_KEY_B && action == GLFW_RELEASE)
        advance_lighting_indicator = false;



}

unsigned int ucitaj_kocke() {

    float vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
//    unsigned  indices[] {
//            //frist
//            0,1,2,
//            // second
//            1,2,3,
//            //druga strana kocke
//            4,5,6,
//            5,6,7,
//            //treca strana
//            8,9,10,
//            9,10,11,
//            //cevrta strana
//            12,13,14,
//            13,14,15,
//            //peta strana
//            16,17,18,
//            17,18,19,
//            //sesta strana
//            20,21,22,
//            21,22,23
//    };

    unsigned VBO,VAO,EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
//    glGenBuffers(1,&EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    //ovde kazemo grafickoj sta ti podaci zapravo predstavljaju
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Deaktiviramo ovaj objekat
//    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    return VAO;
}

unsigned int ucitaj_light_cube(){

    float vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    unsigned int VBO,lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(lightCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    return lightCubeVAO;
}


