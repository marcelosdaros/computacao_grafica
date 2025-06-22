/* Hello Triangle - Marcelo Daros */
// Configuração do cmake:
// Ctrl + Shift + P > CMake: Scan for kit
// Ctrl + Shift + P > CMake: Select a kit
// Ctrl + Shift + P > CMake: Configure
// No terminal: cmake --build . > ./Hello3D.exe

using namespace std;
// GLAD
#include <glad/glad.h>
// GLFW
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Object3D.h"
#include "BezierCurve.h"
#include "json.hpp"
#include <fstream>
#include <vector>
using json = nlohmann::json;

// Camera global e posição inicial da câmera
Camera camera(
    glm::vec3(-3.56f, 0.45f, 2.55f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -30.0f, 0.0f
);

// Protótipo das funções de callback de teclado e mouse
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// Protótipos das funções
int setupShader();

// Vertex Shader (em GLSL):
const GLchar* vertexShaderSource = R"(
#version 450
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texBuff;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 texCoords;
out vec3 vNormal;
out vec3 fragPos;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
	fragPos = vec3(model * vec4(position, 1.0));
    texCoords = texBuff;
	// Uso de transpose e inverse para que o vetor normal também seja transformado (escalas e rotações)
	vNormal = mat3(transpose(inverse(model))) * normal;
}
)";

// Fragment Shader (em GLSL):
const GLchar* fragmentShaderSource = R"(
#version 450
in vec2 texCoords;
in vec3 vNormal;
in vec3 fragPos;
out vec4 FragColor;

uniform sampler2D tex_buffer;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float brightness;

void main()
{
    vec3 texColor = texture(tex_buffer, texCoords).rgb;
	vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

	// Coeficiente de luz ambiente
	vec3 ambient = ka * texColor;

	// Coeficiente difuso
	float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = kd * diff * texColor;

	// Coeficiente especular (luz branca)
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), brightness);
    vec3 specular = ks * spec * vec3(1.0);

	vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Seleção e valores iniciais de rotações, escala, iluminação
bool rotateUp=false, rotateDown=false, rotateLeft=false, rotateRight=false, rotate1=false, rotate2=false;
float scale1 = 0.5, scale2 = 0.9f, scale3 = 0.7f, scale4 = 0.1f;
float ka = 0.0, kd = 0.0f, ks = 0.0f, brightness = 1.0f;
bool isPyramid1Selected=true, isPyramid2Selected=false, isPyramid3Selected=false;

// Variaveis para controle de movimentação da camera
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Variaveis para controle de movimentação do mouse
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

std::vector<glm::vec3> objetosPosicoes;
std::vector<float> objetosEscalas;
std::vector<std::string> objetosModelPaths;

// Função MAIN
int main()
{
	glfwInit(); // Inicialização da GLFW

	// Criação da janela GLFW e cursor escondido
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Trabalho GB -- Marcelo!", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Identificação do arquivo de configuração dos objetos
	std::ifstream configFile("../src/config.json");
	json config;
	configFile >> config;

	// Leitura dos objetos 3D via config.json
	for (const auto& obj : config["objects"]) {
		std::string nome = obj["name"];
		std::string modelPath = obj["model"];
		glm::vec3 pos(obj["x"], obj["y"], obj["z"]);
		float escala = obj["scale"];

		objetosModelPaths.push_back(modelPath);
		objetosPosicoes.push_back(pos);
		objetosEscalas.push_back(escala);
	}

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// GLAD: carrega todos os ponteiros de funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();
	glUseProgram(shaderID);

	GLuint projLoc = glGetUniformLocation(shaderID, "projection");
	GLint viewLoc = glGetUniformLocation(shaderID, "view");
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	// Criação dos objetos 3D (piramides)
	Object3D piramide1(objetosModelPaths[0], objetosPosicoes[0], glm::vec3(objetosEscalas[0]));
	Object3D piramide2(objetosModelPaths[1], objetosPosicoes[1], glm::vec3(objetosEscalas[1]));
	Object3D piramide3(objetosModelPaths[2], objetosPosicoes[2], glm::vec3(objetosEscalas[2]));
	Object3D lua(objetosModelPaths[3], objetosPosicoes[3], glm::vec3(objetosEscalas[3]));

	// Enviar a variável que armazenará o buffer de textura no fragment shader
	glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);

	// Enviar as variáveis que armazenarão os buffers de iluminação (incluindo ka, kd, ks) no fragment shader
	glUniform3f(glGetUniformLocation(shaderID, "lightPos"), 1.0f, 2.0f, 1.0f);
	std::tie(ka, kd, ks, brightness) = piramide1.loadLightingParamsFromMTL("../assets/Modelos3D/Piramide.mtl");
	glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
	glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
	glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
	glUniform1f(glGetUniformLocation(shaderID, "brightness"), brightness);

	// Ativando o primeiro buffer de textura da OpenGL
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_DEPTH_TEST);

	// Criação da curva de Bezier para movimentação
	std::vector<glm::vec3> bezierControlPoints = {
		glm::vec3(1.2f, 0.0f, 1.0f),
		glm::vec3(2.0f, 2.0f, 0.0f),
		glm::vec3(-2.0f, 2.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	};
	// 100 pontos por segmento com velocidade de 0.1s por segmento
	Bezier bezierCurve(bezierControlPoints, 100, 0.1f);
    
	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Movimento suave de câmera
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Verifica se houveram eventos de input e chama as funções de callback
		glfwPollEvents();
		camera.updateCameraPos(window, deltaTime);
		
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view)); // Enviar view atualizada para o shader
		glUniform3fv(glGetUniformLocation(shaderID, "viewPos"), 1, glm::value_ptr(camera.cameraPos)); // Atualiza a viewPos usada no cálculo de iluminação

		// Gerando projeção, para fazer a profundidade na tela
		glm::mat4 projection = glm::perspective(
			glm::radians(camera.fov),
			(float)WIDTH / (float)HEIGHT,
			0.1f,
			100.0f
		);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);
		float angle = (GLfloat)glfwGetTime();

		// Atualiza posição do cubo de acordo com a curva de Bezier
		objetosPosicoes[3] = bezierCurve.update(deltaTime);
		
		// Desenho das piramides
		piramide1.draw(modelLoc, objetosPosicoes[0], angle, objetosEscalas[0], isPyramid1Selected, rotateUp, rotateDown, rotateLeft, rotateRight, rotate1, rotate2);
		piramide2.draw(modelLoc, objetosPosicoes[1], angle, objetosEscalas[1], isPyramid2Selected, rotateUp, rotateDown, rotateLeft, rotateRight, rotate1, rotate2);
		piramide3.draw(modelLoc, objetosPosicoes[2], angle, objetosEscalas[2], isPyramid3Selected, rotateUp, rotateDown, rotateLeft, rotateRight, rotate1, rotate2);
		lua.draw(modelLoc, objetosPosicoes[3], angle, objetosEscalas[3], false, false, false, false, false, false, false);

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_4 && action == GLFW_PRESS) { // Seleciona piramide 1
		isPyramid1Selected = true;
		isPyramid2Selected = false;
		isPyramid3Selected = false;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS) { // Seleciona piramide 2
		isPyramid1Selected = false;
		isPyramid2Selected = true;
		isPyramid3Selected = false;
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS) { // Seleciona piramide 3
		isPyramid1Selected = false;
		isPyramid2Selected = false;
		isPyramid3Selected = true;
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) { // Rotação no eixo X
		rotateUp = !rotateUp;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) { // Rotação no eixo X
		rotateUp = false;
		rotateDown = !rotateDown;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) { // Rotação no eixo Y
		rotateUp = false;
		rotateDown = false;
		rotateLeft = !rotateLeft;
		rotateRight = false;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { // Rotação no eixo Y
		rotateUp = false;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = !rotateRight;
		rotate1 = false;
		rotate2 = false;
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) { // Rotação no eixo Z
		rotateUp = false;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = !rotate1;
		rotate2 = false;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) { // Rotação no eixo Z
		rotateUp = false;
		rotateDown = false;
		rotateLeft = false;
		rotateRight = false;
		rotate1 = false;
		rotate2 = !rotate2;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) { // Aumenta a escala
        if (isPyramid1Selected) objetosEscalas[0] += 0.1f;
		if (isPyramid2Selected) objetosEscalas[1] += 0.1f;
		if (isPyramid3Selected) objetosEscalas[2] += 0.1f;
    }
	if (key == GLFW_KEY_X && action == GLFW_PRESS) { // Diminui a escala e impede valores negativos
        if (isPyramid1Selected) objetosEscalas[0] = glm::max(0.1f, objetosEscalas[0] - 0.1f);
		if (isPyramid2Selected) objetosEscalas[1] = glm::max(0.1f, objetosEscalas[1] - 0.1f);
		if (isPyramid3Selected) objetosEscalas[2] = glm::max(0.1f, objetosEscalas[2] - 0.1f);
    }
	// if (key == GLFW_KEY_J && action == GLFW_PRESS) { // Translação no eixo X (para o lado esquerdo)
	// 	if (isPyramid1Selected) coord_x1 -= 0.2f;
	// 	if (isPyramid2Selected) coord_x2 -= 0.2f;
	// 	if (isPyramid3Selected) coord_x3 -= 0.2f;
	// }
	// if (key == GLFW_KEY_L && action == GLFW_PRESS) { // Translação no eixo X (para o lado direito)
	// 	if (isPyramid1Selected) coord_x1 += 0.2f;
	// 	if (isPyramid2Selected) coord_x2 += 0.2f;
	// 	if (isPyramid3Selected) coord_x3 += 0.2f;
	// }
	// if (key == GLFW_KEY_I && action == GLFW_PRESS) { // Translação no eixo Z (para frente)
	// 	if (isPyramid1Selected) coord_z1 -= 0.2f;
	// 	if (isPyramid2Selected) coord_z2 -= 0.2f;
	// 	if (isPyramid3Selected) coord_z3 -= 0.2f;
	// }
	// if (key == GLFW_KEY_K && action == GLFW_PRESS) { // Translação no eixo Z (para trás)
	// 	if (isPyramid1Selected) coord_z1 += 0.2f;
	// 	if (isPyramid2Selected) coord_z2 += 0.2f;
	// 	if (isPyramid3Selected) coord_z3 += 0.2f;
	// }
}

// Função de callback do mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.updateMouseMovement(xoffset, yoffset);
}

// Função de callback do scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.updateMouseScroll(static_cast<float>(yoffset));
}

// Retorna o identificador do programa de shader
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