#ifndef RG_PROJECT_MESH_H
#define RG_PROJECT_MESH_H

#include <glm/glm.hpp>

struct Texture{
    unsigned id;
    std::string type;
    std::string path;
};

struct Vertex{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    //atributi za neke dalje oblasti
    glm::vec3 Tangent;
    glm::vec3 BiTangent;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    std::vector<Texture> textures;

    void Draw(Shader &shader){
        //ovde povezujemo teksture
        shader.use();

        unsigned diffuseNr = 1;
        unsigned specularNr = 1;
        unsigned normalNr = 1;
        unsigned heightNr = 1;

        std::string buffer; //texture_specular
        buffer.reserve(64);
        for(unsigned  i = 0; i < textures.size(); ++i){

            buffer.append(textures[i].type);
            if(textures[i].type == "texture_diffuse"){
                buffer.append(std::to_string(diffuseNr++));
            }
            else if(textures[i].type == "texture_specular"){
                buffer.append(std::to_string(specularNr++)); //texture_specular1
            }
            else if(textures[i].type == "texture_normal"){
                buffer.append(std::to_string(normalNr++));
            }
            else if(textures[i].type == "texture_height"){
                buffer.append(std::to_string(heightNr++));
            }

            glActiveTexture(GL_TEXTURE0 + i);
            shader.setUniform1int(buffer.c_str(),i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
            buffer.clear();
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    //kod koji podatke mesha salje na graficku karticu
    void setupMesh(){
        unsigned VBO,EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1,&VBO);
        glGenBuffers(1,&EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0],GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*) offsetof(Vertex,Position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*) offsetof(Vertex,Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*) offsetof(Vertex,TexCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*) offsetof(Vertex,Tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(void*) offsetof(Vertex,BiTangent));

        glBindVertexArray(0);
    }

private:
    unsigned VAO;
};

#endif //RG_PROJECT_MESH_H
