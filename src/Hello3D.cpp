/* Hello Triangle - Marcelo Daros */
// Configuração do cmake:
// Ctrl + Shift + P > CMake: Scan for kit
// Ctrl + Shift + P > CMake: Select a kit
// Ctrl + Shift + P > CMake: Configure
// No terminal: cmake --build . > ./Hello3D.exe

#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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
GLuint loadOBJ(const std::string& path, int &nVertices);

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

void main()
{
    gl_Position = projection * model * vec4(position, 1.0);
    texCoords = texBuff;
}
)";

// Código fonte do Fragment Shader (em GLSL):
const GLchar* fragmentShaderSource = R"(
#version 450
in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D tex_buffer;

void main()
{
    FragColor = texture(tex_buffer, texCoords);
}
)";

float cubo1x = 0.3f, cubo2x = -0.3f; // positiveX = inicia o eixo X com +0.4; negativeX = inicia o eixo X com -0.4
float cubo1y = 0.0f, cubo2y = 0.0f;  // os 2 cubos iniciam com y = 0
float cubo1z = -2.0f, cubo2z = -2.0f; // os 2 cubos iniciam com z = -3
float scaleCubo1 = 0.5f, scaleCubo2 = 0.5f;
bool rotateUp=false, rotateDown=false, rotateLeft=false, rotateRight=false, rotate1=false, rotate2=false;
bool cubo1selecionado=false, cubo2selecionado=true;

// struct para os cubos
struct Cubo {
	GLuint VAO;
	int numVertices;
	glm::vec3 position;
};

// struct para armazenamento de cada vértice do .obj
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};
vector<Vertex> vertices;
vector<unsigned int> indices;
vector<glm::vec2> tempTexCoords;

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

	std::vector<Cubo> cubos;

	// Cubo 1
	Cubo cubo1;
	cubo1.VAO = loadOBJ("../assets/Modelos3D/Cube.obj", cubo1.numVertices);
	cubo1.position = glm::vec3(cubo1x, cubo1y, cubo1z);
	cubos.push_back(cubo1);

	// Cubo 2
	Cubo cubo2;
	cubo2.VAO = loadOBJ("../assets/Modelos3D/Cube.obj", cubo2.numVertices);
	cubo2.position = glm::vec3(cubo2x, cubo2y, cubo2z);
	cubos.push_back(cubo2);

	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	for (auto& cubo : cubos) {
		glm::mat4 model = glm::mat4(1.0f); // matriz identidade
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // aplica rotação
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // envia ao shader
	}

	glEnable(GL_DEPTH_TEST);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Verifica se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);
		float angle = (GLfloat)glfwGetTime();

		for (auto& cubo : cubos) {
			for (size_t i = 0; i < cubos.size(); ++i) {
			    if (i == 0)
					cubos[i].position = glm::vec3(cubo1x, cubo1y, cubo1z); // posição do cubo1
				else if (i == 1)
					cubos[i].position = glm::vec3(cubo2x, cubo2y, cubo2z); // posição do cubo2
			}	

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubo.position);

			if (rotateUp)
				model = glm::rotate(model, angle, glm::vec3(-1.0f, 0.0f, 0.0f)); // Rotação no eixo X
			else if (rotateDown)
				model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotação no eixo X
			else if (rotateLeft)
				model = glm::rotate(model, angle, glm::vec3(0.0f, -1.0f, 0.0f)); // Rotação no eixo Y
			else if (rotateRight)
				model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotação no eixo Y
			else if (rotate1)
				model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotação no eixo Z
			else if (rotate2)
				model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, -1.0f)); // Rotação no eixo Z

			for (size_t i = 0; i < cubos.size(); ++i) {
			    if (i == 0)
					model = glm::scale(model, glm::vec3(scaleCubo1, scaleCubo1, scaleCubo1)); // escala do cubo1
				else if (i == 1)
					model = glm::scale(model, glm::vec3(scaleCubo2, scaleCubo2, scaleCubo2)); // escala do cubo2
			}	

			glBindVertexArray(cubo.VAO);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, cubo.numVertices);
			glDrawArrays(GL_POINTS, 0, cubo.numVertices);
		}

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	for (auto& cubo : cubos) {
    	glDeleteVertexArrays(1, &cubo.VAO);
	}
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

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
        if (cubo1selecionado) scaleCubo1 += 0.1f;
		if (cubo2selecionado) scaleCubo2 += 0.1f;
    }
	if (key == GLFW_KEY_X && action == GLFW_PRESS) { // Diminui a escala e impede valores negativos
        if (cubo1selecionado) scaleCubo1 = glm::max(0.1f, scaleCubo1 - 0.1f);
		if (cubo2selecionado) scaleCubo2 = glm::max(0.1f, scaleCubo2 - 0.1f);
    }
	if (key == GLFW_KEY_W && action == GLFW_PRESS) { // Move no eixo Y (para cima)
		if (cubo1selecionado) cubo1y += 0.2f;
		if (cubo2selecionado) cubo2y += 0.2f;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) { // Move no eixo Y (para baixo)
		if (cubo1selecionado) cubo1y -= 0.2f;
		if (cubo2selecionado) cubo2y -= 0.2f;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) { // Move no eixo X (para a esquerda)
		if (cubo1selecionado) cubo1x -= 0.2f;
		if (cubo2selecionado) cubo2x -= 0.2f;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) { // Move no eixo X (para a direita)
		if (cubo1selecionado) cubo1x += 0.2f;
		if (cubo2selecionado) cubo2x += 0.2f;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) { // Move no eixo Z (para frente)
		if (cubo1selecionado) cubo1z += 0.2f;
		if (cubo2selecionado) cubo2z += 0.2f;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) { // Move no eixo Z (para trás)
		if (cubo1selecionado) cubo1z -= 0.2f;
		if (cubo2selecionado) cubo2z -= 0.2f;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) { // Seleciona cubo 1
		cubo1selecionado = false;
		cubo2selecionado = true;
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS) { // Seleciona cubo 2
		cubo1selecionado = true;
		cubo2selecionado = false;
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

// Função para ler e carregar o .obj; retorna o VAO
GLuint loadOBJ(const string& path, int &nVertices) {
    string line;
    vector<glm::vec3> tempPositions, tempNormals;
    string mtlFile;

	ifstream arqEntrada(path.c_str());
    if (!arqEntrada.is_open()) {
        cerr << "Erro ao tentar ler o arquivo " << path << endl;
        return -1;
    }

	// Leitura de cada linha do arquivo .obj
    while (getline(arqEntrada, line)) {
        istringstream iss(line);
        string prefix;
        iss >> prefix;

        if (prefix == "mtllib") {
            iss >> mtlFile;
        } else if (prefix == "v") { // vertices
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            tempPositions.push_back(pos);
        } else if (prefix == "vt") { // coords de textura
            glm::vec2 tex;
            iss >> tex.x >> tex.y;
            tempTexCoords.push_back(tex);
        } else if (prefix == "vn") { // normais
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        } else if (prefix == "f") { // recupera e armazena valores de cada indice
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
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	nVertices = vBuffer.size() / 8; // x, y, z, nx, ny, nz, u, v

	return VAO;
}