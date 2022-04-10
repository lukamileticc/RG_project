//
// Created by luka on 30.3.22..
//

#ifndef RG_PROJECT_SHADER_H
#define RG_PROJECT_SHADER_H

#include "glad/glad.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

std::string readFileContent(const char* path){
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return  ss.str();
}
class Shader{
public:
    Shader(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath){

        std::string  vertexShaderSourceString = readFileContent(vertexShaderSourcePath);
        int vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexShaderSource = vertexShaderSourceString.c_str();
        glShaderSource(vertexShaderId,1, &vertexShaderSource, NULL);
        glCompileShader(vertexShaderId);
        int success;
        char infoLog[512] = {0};
        glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS,&success);
        if(!success){
            glGetShaderInfoLog(vertexShaderId,512, nullptr, infoLog);
            std::cout << "ERRO::VERTEX::SHADER \n" << infoLog << '\n';
        }

        std::string fragmentShaderSourceString = readFileContent(fragmentShaderSourcePath);
        int fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentShaderSource = fragmentShaderSourceString.c_str();
        glShaderSource(fragmentShaderId,1,&fragmentShaderSource,NULL);
        glCompileShader(fragmentShaderId);
        glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS,&success);
        if(!success){
            glGetShaderInfoLog(fragmentShaderId,512, nullptr, infoLog);
            std::cout << "ERRO::FRAGMENT::SHADER \n" << infoLog << '\n';
        }

        int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram,vertexShaderId);
        glAttachShader(shaderProgram,fragmentShaderId);
        glLinkProgram(shaderProgram);
        glGetShaderiv(shaderProgram,GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(shaderProgram,512, nullptr,infoLog);
            std::cout << "ERROR::PROGRAM::\n" << infoLog << '\n';
        }

        m_shaderProgramId = shaderProgram;
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
    }
    void use(){
        assert(m_shaderProgramId > 0);
        glUseProgram(m_shaderProgramId);
    }
    void deleteProgram(){
        glDeleteShader(m_shaderProgramId);
    }
    void setUniform4Float(const char *varName, float v1, float v2, float v3, float v4){
        //nalazimo tu uniformnu promenljivu
        int varId = glGetUniformLocation(m_shaderProgramId,varName);
        glUniform4f(varId,v1, v2, v3, v4);
    }

    void setUniform1int(const char *varName, int v1) {
        int varId = glGetUniformLocation(m_shaderProgramId,varName);
        glUniform1i(varId, v1);
    }

    void setUniform1Float(const char *varName, float v1) {
        int varId = glGetUniformLocation(m_shaderProgramId,varName);
        glUniform1f(varId, v1);
    }

    void setUniformMat4(const char *varName, glm::mat4 &matrix) {
        int varLock = glGetUniformLocation(m_shaderProgramId,varName);
        glUniformMatrix4fv(varLock, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void setUniformVec3(const char *varName, glm::vec3 vec1) {
        int varLock = glGetUniformLocation(m_shaderProgramId,varName);
        glUniform3fv(varLock, 1, &vec1[0]);
    }
    void setUniformVec3(const char *varName, float v1, float  v2, float v3) {
        glm::vec3 vec1(v1,v2,v3);
        int varLock = glGetUniformLocation(m_shaderProgramId,varName);
        glUniform3fv(varLock, 1, &vec1[0]);
    }

private:
    unsigned  m_shaderProgramId = 0;
};


#endif //RG_PROJECT_SHADER_H
