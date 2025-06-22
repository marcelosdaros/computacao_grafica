#ifndef OBJECT3D_HPP
#define OBJECT3D_HPP

#include <fstream>
#include <sstream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
using namespace std;

class Object3D {
public:
    GLuint VAO;
    GLuint texID;
    int numVertices;

    glm::vec3 objectPosition;
    glm::vec3 objectScale;

    // Struct para armazenamento dos vértices do .obj
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    // Construtor
    Object3D(const std::string& objPath, glm::vec3 initPosition, glm::vec3 initScale)
        : objectPosition(initPosition), objectScale(initScale) {
        std::tie(VAO, texID) = loadOBJ(objPath, numVertices);
    }

    // Destrutor para desalocar os buffers
    ~Object3D() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteTextures(1, &texID);
    }

    // Criação do model com translação, rotação e escala
    glm::mat4 getModelMatrix(float angle, bool isSelected, bool rotateUp, bool rotateDown, bool rotateLeft, bool rotateRight, bool rotate1, bool rotate2) {
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, objectPosition);

        if (isSelected) {
            if (rotateUp) {
                model = glm::rotate(model, angle, glm::vec3(-1.0f, 0.0f, 0.0f));
            } else if (rotateDown) {
                model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
            } else if (rotateLeft) {
                model = glm::rotate(model, angle, glm::vec3(0.0f, -1.0f, 0.0f));
            } else if (rotateRight) {
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            } else if (rotate1) {
                model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            } else if (rotate2) {
                model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, -1.0f));
            }
        }

        model = glm::scale(model, objectScale);
        return model;
    }

    // Desenho com textura
    void draw(GLuint modelLoc, glm::vec3 position, float angle, float scale, bool isSelected, bool rotateUp, bool rotateDown, bool rotateLeft, bool rotateRight, bool rotate1, bool rotate2) {
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, texID);

        objectScale = glm::vec3(scale);
        objectPosition = glm::vec3(position);

        glm::mat4 model = getModelMatrix(angle, isSelected, rotateUp, rotateDown, rotateLeft, rotateRight, rotate1, rotate2);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
        if (isSelected) glDrawArrays(GL_POINTS, 0, numVertices);
    }

    // Função para ler e carregar o .obj, retorna o VAO e sua textura
    std::pair<GLuint, GLuint> loadOBJ(const string& path, int &nVertices) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indexes;
        std::vector<glm::vec2> tempTexCoords;

        string line;
        vector<glm::vec3> tempPositions, tempNormals;
        string mtlFile;

        ifstream arqEntrada(path.c_str());
        if (!arqEntrada.is_open()) {
            cerr << "Erro ao tentar ler o arquivo " << path << endl;
            return { -1, -1 };
        }

        // Leitura de cada linha do arquivo .obj
        while (getline(arqEntrada, line)) {
            istringstream iss(line);
            string prefix;
            iss >> prefix;

            if (prefix == "mtllib") {
                iss >> mtlFile;
            } else if (prefix == "v") {
                glm::vec3 pos;
                iss >> pos.x >> pos.y >> pos.z;
                tempPositions.push_back(pos);
            } else if (prefix == "vt") {
                glm::vec2 tex;
                iss >> tex.x >> tex.y;
                tempTexCoords.push_back(tex);
            } else if (prefix == "vn") {
                glm::vec3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                tempNormals.push_back(normal);
            } else if (prefix == "f") {
                unsigned int pIdx[3], tIdx[3], nIdx[3];
                char slash;
                for (int i = 0; i < 3; ++i) {
                    iss >> pIdx[i] >> slash >> tIdx[i] >> slash >> nIdx[i];
                    Vertex vertex;
                    vertex.position = tempPositions[pIdx[i] - 1];
                    vertex.normal = tempNormals[nIdx[i] - 1];
                    vertex.texCoords = tempTexCoords[tIdx[i] - 1];
                    vertices.push_back(vertex);
                    indexes.push_back(vertices.size() - 1);
                }
            }
        }
        arqEntrada.close();

        int texWidth, texHeight;
        GLuint texID;

        // Carregamento da textura presente no arquivo mtl
        string textureFile = loadTextureMTL("../assets/Modelos3D/" + mtlFile);
        texID = loadTexture("../assets/textures/" + textureFile, texWidth, texHeight);

        // Vetor para armazenamento dos vértices
        std::vector<GLfloat> vBuffer;
        for (const auto& v : vertices) {
            vBuffer.push_back(v.position.x);
            vBuffer.push_back(v.position.y);
            vBuffer.push_back(v.position.z);
            vBuffer.push_back(v.normal.x);
            vBuffer.push_back(v.normal.y);
            vBuffer.push_back(v.normal.z);
            vBuffer.push_back(v.texCoords.x);
            vBuffer.push_back(v.texCoords.y);
        }
        
        std::cout << "Gerando o buffer de geometria..." << std::endl;
        GLuint VBO, VAO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);
        
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        
        // Posição (location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        
        // Normal (location 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        
        // Textura (location 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        nVertices = vBuffer.size() / 8; // x, y, z, nx, ny, nz, u, v
        return { VAO, texID };
    }

    GLuint loadTexture(string filePath, int &width, int &height) {
        GLuint texID; // id da textura a ser carregada

        // Gera o identificador da textura na memória
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        // Ajuste dos parâmetros de wrapping e filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Carregamento da imagem usando a função stbi_load da biblioteca stb_image
        int nrChannels;
        unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

        if (data) {
            if (nrChannels == 3) // jpg, bmp
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }
            else // assume que é 4 canais png
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture " << filePath << std::endl;
        }

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return texID;
    }

    // Função para carregar o material/textura
    string loadTextureMTL(const string& mtlPath) {
        ifstream file(mtlPath);
        string line, textureFile;

        // Encontra o parametro map_Kd, e retorna o arquivo
        while (getline(file, line)) {
            istringstream iss(line);
            string prefix;
            iss >> prefix;
            if (prefix == "map_Kd") {
                iss >> textureFile;
                break;
            }
        }
        return textureFile;
    }

    // Função para carregar os parametros de iluminação
    std::tuple<float, float, float, float> loadLightingParamsFromMTL(const string& mtlPath) {
        std::ifstream file(mtlPath);
        std::string line;

        // Valores padrão
        float ka = 0.0f, kd = 0.0f, ks = 0.0f, brightness = 1.0f;

        // Encontra e retorna os parametros ka, kd, ks, brightness
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "ka") {
                iss >> ka;
            } else if (prefix == "kd") {
                iss >> kd;
            } else if (prefix == "ks") {
                iss >> ks;
            } else if (prefix == "brightness") {
                iss >> brightness;
            }
        }
        return std::make_tuple(ka, kd, ks, brightness);
    }
};
#endif