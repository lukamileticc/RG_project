#ifndef RG_PROJECT_MODEL_H
#define RG_PROJECT_MODEL_H

#include <iostream>
#include <vector>
#include <unordered_set>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

unsigned int texture_from_file(const char *path, const std::string &directory);

class Model{
public:
    std::vector<Mesh> m_mashes;
    explicit Model(const std::string &path){
        loadModel(path);
    }
    void Draw(Shader& shader){
        for(auto &m : m_mashes)
            m.Draw(shader);
    }

private:
    std::string  m_directory;
    std::unordered_set<std::string> m_loaded_textures;

    void loadModel(const std::string &path){
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                |aiProcess_FlipUVs|aiProcess_CalcTangentSpace);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR:: ASSIMP:: " << importer.GetErrorString() << std::endl;
            std::terminate();
        }
        m_directory = path.substr(0,path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode *node, const aiScene *scene){
        //za odredjeni node vadimo sve meseve koje on ima
        for(unsigned i = 0; i < node->mNumMeshes;i++){
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            m_mashes.push_back(processMesh(mesh,scene));
        }

        //rekurzivno obilazimo sve nodove(cvorove)
        for (unsigned i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene);
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene){

        Mesh result;

        for(unsigned  i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex = {};

            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;

            if(mesh->HasNormals()){
                vertex.Normal.x = mesh->mNormals[i].x;
                vertex.Normal.y = mesh->mNormals[i].y;
                vertex.Normal.z = mesh->mNormals[i].z;
            }

            if(mesh->mTextureCoords[0]){

                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;

                vertex.Tangent.x = mesh->mTangents[i].x;
                vertex.Tangent.y = mesh->mTangents[i].y;
                vertex.Tangent.z = mesh->mTangents[i].z;

                vertex.BiTangent.x = mesh->mBitangents[i].x;
                vertex.BiTangent.y = mesh->mBitangents[i].y;
                vertex.BiTangent.z = mesh->mBitangents[i].z;
            }

            result.vertices.push_back(vertex);
        }

    //ovde ucitavamo indekse za crtanje trouglova koje takodje saljemo grafickoj
        for(unsigned  i = 0; i < mesh->mNumFaces; i++){
            aiFace face = mesh->mFaces[i];
            for(unsigned j = 0; j < face.mNumIndices;j++){
                result.indices.push_back(face.mIndices[j]);
            }
        }

        // ovde izvlacimo teksture
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        //difuzne,spekularne,normale,visine
        std::vector<Texture> diffuseMaps = loadMaterialTexture(mat, aiTextureType_DIFFUSE, "texture_diffuse");
        std::vector<Texture> specularMaps = loadMaterialTexture(mat, aiTextureType_SPECULAR, "texture_specular");
        std::vector<Texture> normalMaps = loadMaterialTexture(mat, aiTextureType_HEIGHT, "texture_normal");
        std::vector<Texture> heightMaps = loadMaterialTexture(mat, aiTextureType_AMBIENT, "texture_height");

        result.textures.insert(result.textures.end(),diffuseMaps.begin(),diffuseMaps.end());
        result.textures.insert(result.textures.end(),specularMaps.begin(),specularMaps.end());
        result.textures.insert(result.textures.end(),normalMaps.begin(),normalMaps.end());
        result.textures.insert(result.textures.end(),heightMaps.begin(),heightMaps.end());

        result.setupMesh();

        return result;
    }


    std::vector<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type, const std::string &type_name){

        std::vector<Texture> result;
        for(unsigned i = 0; i < mat->GetTextureCount(type); i++){
            aiString str;
            mat->GetTexture(type, i, &str);

            //provera da i je tekstura vec ucitana
            if(m_loaded_textures.count(str.C_Str()) == 0) {
                Texture texture;
                texture.id = texture_from_file(str.C_Str(), m_directory);
                texture.type = type_name;
                texture.path = str.C_Str();
                result.push_back(texture);
                m_loaded_textures.insert(texture.path);
                std::cout << texture.path <<':' << texture.type << std::endl;
            }
        }
        return result;
    }

};


unsigned int texture_from_file(const char *path, const std::string &directory) {

    std::string filename = std::string(path);
    filename = directory + '/' + filename;

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
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannel, 0);

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


#endif //RG_PROJECT_MODEL_H
