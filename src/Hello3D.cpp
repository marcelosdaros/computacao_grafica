/* Hello Cube - Marcelo Daros */

#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cmath>
using namespace std;

// GLAD
#include <glad/glad.h>
// GLFW
#include <GLFW/glfw3.h>
//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();
std::pair<GLuint, GLuint> loadOBJ(const std::string& path, int &nVertices);
GLuint loadTexture(string filePath, int &width, int &height);
string loadMTL(const string& mtlPath);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL):
const GLchar* vertexShaderSource = R"(
#version 450
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texBuff;

uniform mat4 model;
uniform mat4 projection;

out vec2 texCoords;
out vec3 vNormal;
out vec3 fragPos;

void main()
{
    gl_Position = projection * model * vec4(position, 1.0);
	fragPos = vec3(model * vec4(position, 1.0));
    texCoords = texBuff;
	// Uso de transpose e inverse para que o vetor normal também seja transformado (escalas e rotações)
	vNormal = mat3(transpose(inverse(model))) * normal;
}
)";

// Código fonte do Fragment Shader (em GLSL):
const GLchar* fragmentShaderSource = R"(
#version 450
in vec2 texCoords;
in vec3 vNormal;
in vec3 fragPos;
out vec4 FragColor;

// Estrutura para luz pontual
struct PointLight {
    vec3 position;
    float intensity;
    bool enabled;

    float constant;
    float linear;
    float quadratic;
};

uniform PointLight mainLight;
uniform PointLight fillLight;
uniform PointLight backLight;

uniform sampler2D tex_buffer;
uniform vec3 viewPos;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float brightness;

// Função para o cálculo de cada ponto de luz com atenuação
vec3 CalcLight(PointLight light, vec3 normal, vec3 viewDir, vec3 texColor)
{
	// Se a luz estiver desativada, não faz os cálculos
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
	// Coeficiente difuso
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = kd * diff * texColor;

	// Coeficiente especular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), brightness);
    vec3 specular = ks * spec * vec3(1.0);

    return light.intensity * attenuation * (diffuse + specular);
}

void main()
{
    vec3 texColor = texture(tex_buffer, texCoords).rgb;
	vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
	
	// Coeficiente de luz ambiente
	vec3 ambient = ka * texColor;

	vec3 result = ambient;
    result += CalcLight(mainLight, normal, viewDir, texColor);
    result += CalcLight(fillLight, normal, viewDir, texColor);
    result += CalcLight(backLight, normal, viewDir, texColor);

    FragColor = vec4(result, 1.0);
}
)";

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

vector<Vertex> vertices;
vector<unsigned int> indices;
vector<glm::vec2> tempTexCoords;

struct Light {
    glm::vec3 pos;
    float intensity;
    bool enabled;
};

Light mainLight   = {{ 0.0f, 0.5f, -2.0f }, 1.0f, true};  // Luz principal
Light fillLight   = {{-1.5f, 0.3f, -2.0f }, 0.4f, true};  // Luz de preenchimento
Light backLight   = {{ 0.0f, 0.5f, -4.0f }, 0.6f, true};  // Luz de fundo

float x = 0.0f;  // o cubo inicia com x = 0
float y = 0.4f;  // o cubo inicia com y = 0.4
float z = -3.0f; // o cubo inicia com z = -3

bool rotateUp=false, rotateDown=false, rotateLeft=false, rotateRight=false, rotate1=false, rotate2=false;
float scale = 0.5f;
float ka = 0.1f, kd = 0.8f, ks = 0.6f, brightness = 60.0f;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Marcelo!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();
	glUseProgram(shaderID);

	// Gerando projeção, para fazer a profundidade na tela
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),
		(float)WIDTH / (float)HEIGHT,
		0.1f,
		100.0f
	);
	GLuint projLoc = glGetUniformLocation(shaderID, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Posição de visualização
	glm::vec3 viewPos(0.0f, 0.0f, 3.0f);
	glUniform3fv(glGetUniformLocation(shaderID, "viewPos"), 1, glm::value_ptr(viewPos));

	// Gerando o buffer de VAO e textura de cada cubo
	int numVertices1;
	GLuint VAO, texID1;
	std::tie(VAO, texID1) = loadOBJ("../assets/Modelos3D/Cube1.obj", numVertices1);

	// Variável que armazenará o buffer de textura no fragment shader
	glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);

	// Variaveis para o cálculo de atenuação
	float constant = 1.0f;
	float linear = 0.045f;
	float quadratic = 0.0075f;

	// Envia as informações de cada ponto de luz para o fragment shader
	auto setLight = [&](const char* name, const Light& light) {
		glUniform3fv(glGetUniformLocation(shaderID, (std::string(name) + ".position").c_str()), 1, glm::value_ptr(light.pos));
		glUniform1f(glGetUniformLocation(shaderID, (std::string(name) + ".intensity").c_str()), light.intensity);
		glUniform1i(glGetUniformLocation(shaderID, (std::string(name) + ".enabled").c_str()), light.enabled);
		glUniform1f(glGetUniformLocation(shaderID, (std::string(name) + ".constant").c_str()), constant);
		glUniform1f(glGetUniformLocation(shaderID, (std::string(name) + ".linear").c_str()), linear);
		glUniform1f(glGetUniformLocation(shaderID, (std::string(name) + ".quadratic").c_str()), quadratic);
	};

	// Enviar as variáveis que armazenarão os buffers de iluminação (incluindo ka, kd, ks) no fragment shader
	glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
	glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
	glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
	glUniform1f(glGetUniformLocation(shaderID, "brightness"), brightness);

	// Ativando o primeiro buffer de textura da OpenGL
	glActiveTexture(GL_TEXTURE0);

	// Cubo 1
	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Verifica se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		// Luzes atualizadas a cada frame
		setLight("mainLight", mainLight);
		setLight("fillLight", fillLight);
		setLight("backLight", backLight);

		glBindVertexArray(VAO);

		// Instanciação do cubo e aplicação de rotação em cada um
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(x, y, z)); // Move cubo para cima

		if (rotateUp)
		{
			model = glm::rotate(model, angle, glm::vec3(-1.0f, 0.0f, 0.0f)); // Rotação no eixo X
		}
		else if (rotateDown)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotação no eixo X
		}
		else if (rotateLeft)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, -1.0f, 0.0f)); // Rotação no eixo Y
		}
		else if (rotateRight)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotação no eixo Y
		}
		else if (rotate1)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotação no eixo Z
		}
		else if (rotate2)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, -1.0f)); // Rotação no eixo Z
		}

		// Ativa textura do cubo antes de desenhar
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID1);

		// Aplica a escala
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		// Chamada de desenho (drawcall) e polígono preenchido com GL_TRIANGLES
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, numVertices1);
		glDrawArrays(GL_POINTS, 0, numVertices1);

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_4) mainLight.enabled = !mainLight.enabled;
		if (key == GLFW_KEY_5) fillLight.enabled = !fillLight.enabled;
		if (key == GLFW_KEY_6) backLight.enabled = !backLight.enabled;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		rotateUp = true;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		rotateUp = false;
		rotateDown = true;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		rotateUp = false;
		rotateDown = false;
		rotateLeft = true;
		rotateRight = false;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		rotateUp = false;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = true;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		rotateUp = false;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = true;
		rotate2 = false;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		rotateUp = false;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = false;
		rotate2 = true;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) { // Aumenta a escala
        scale += 0.1f; 
    }
	if (key == GLFW_KEY_X && action == GLFW_PRESS) { // Diminui a escala e impede valores negativos
        scale = glm::max(0.1f, scale - 0.1f); 
    }
	if (key == GLFW_KEY_W && action == GLFW_PRESS) { // Move no eixo Y (para cima)
		y += 0.2f;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) { // Move no eixo Y (para baixo)
		y -= 0.2f;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) { // Move no eixo X (para a esquerda)
		x -= 0.2f;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) { // Move no eixo X (para a direita)
		x += 0.2f;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) { // Move no eixo Z (para frente)
		z += 0.2f;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) { // Move no eixo Z (para trás)
		z -= 0.2f;
	}
}

// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Função para ler e carregar o .obj, retorna o VAO e sua textura
std::pair<GLuint, GLuint> loadOBJ(const string& path, int &nVertices) {
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
                indices.push_back(vertices.size() - 1);
            }
        }
    }
    arqEntrada.close();

    int texWidth, texHeight;
	GLuint texID;

	// Carregamento da textura presente no arquivo mtl
    string textureFile = loadMTL("../assets/Modelos3D/" + mtlFile);
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
string loadMTL(const string& mtlPath) {
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