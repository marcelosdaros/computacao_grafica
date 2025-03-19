/* Hello Triangle - */
#include <iostream>
#include <string>
#include <assert.h>

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

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
//...pode ter mais linhas de código aqui!
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateW=false, rotateS=false, rotateA=false, rotateD=false, rotateI=false, rotateJ=false;
float scale = 0.5f;

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

	// Gerando um buffer simples, com a geometria de um triângulo
	GLuint VAO = setupGeometry();
	glUseProgram(shaderID);

	// Cubo 1
	glm::mat4 model1 = glm::mat4(1); //matriz identidade;
	// Cubo 2
	glm::mat4 model2 = glm::mat4(1); //matriz identidade;
	// Cubo 3
	glm::mat4 model3 = glm::mat4(1); //matriz identidade;
	// Cubo 4
	glm::mat4 model4 = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	model1 = glm::rotate(model1, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));

	model2 = glm::rotate(model2, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));

	model3 = glm::rotate(model3, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));

	model4 = glm::rotate(model4, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));

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

		// Instanciação dos cubos e aplicação de rotação em cada um
		model1 = glm::mat4(1);
		model2 = glm::mat4(1);
		model3 = glm::mat4(1);
		model4 = glm::mat4(1);
		if (rotateW)
		{
			model1 = glm::rotate(model1, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			model2 = glm::rotate(model2, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			model3 = glm::rotate(model3, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			model4 = glm::rotate(model4, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateS)
		{
			model1 = glm::rotate(model1, angle, glm::vec3(-1.0f, 0.0f, 0.0f));
			model2 = glm::rotate(model2, angle, glm::vec3(-1.0f, 0.0f, 0.0f));
			model3 = glm::rotate(model3, angle, glm::vec3(-1.0f, 0.0f, 0.0f));
			model4 = glm::rotate(model4, angle, glm::vec3(-1.0f, 0.0f, 0.0f));
		}
		else if (rotateA)
		{
			model1 = glm::rotate(model1, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			model3 = glm::rotate(model3, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			model4 = glm::rotate(model4, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (rotateD)
		{
			model1 = glm::rotate(model1, angle, glm::vec3(0.0f, -1.0f, 0.0f));
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, -1.0f, 0.0f));
			model3 = glm::rotate(model3, angle, glm::vec3(0.0f, -1.0f, 0.0f));
			model4 = glm::rotate(model4, angle, glm::vec3(0.0f, -1.0f, 0.0f));
		}
		else if (rotateI)
		{
			model1 = glm::rotate(model1, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			model3 = glm::rotate(model3, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			model4 = glm::rotate(model4, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (rotateJ)
		{
			model1 = glm::rotate(model1, angle, glm::vec3(0.0f, 0.0f, -1.0f));
			model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 0.0f, -1.0f));
			model3 = glm::rotate(model3, angle, glm::vec3(0.0f, 0.0f, -1.0f));
			model4 = glm::rotate(model4, angle, glm::vec3(0.0f, 0.0f, -1.0f));
		}

		glBindVertexArray(VAO);

		// Move cubo 1 para a esquerda e para baixo
		model1 = glm::translate(model1, glm::vec3(-0.4f, -0.4f, 0.0f));
		// Aplica a escala
		model1 = glm::scale(model1, glm::vec3(scale, scale, scale));
		// Chamada de desenho (drawcall) e polígono preenchido com GL_TRIANGLES
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS, 0, 36);

		// Move cubo 2 para a direita e para baixo
		model2 = glm::translate(model2, glm::vec3(0.4f, -0.4f, 0.0f));
		// Aplica a escala
		model2 = glm::scale(model2, glm::vec3(scale, scale, scale));
		// Chamada de desenho (drawcall) e polígono preenchido com GL_TRIANGLES
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS, 0, 36);

		// Move cubo 3 para a esquerda e para cima
		model3 = glm::translate(model3, glm::vec3(-0.4f, 0.4f, 0.0f));
		// Aplica a escala
		model3 = glm::scale(model3, glm::vec3(scale, scale, scale));
		// Chamada de desenho (drawcall) e polígono preenchido com GL_TRIANGLES
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS, 0, 36);

		// Move cubo 4 para a direita e para cima
		model4 = glm::translate(model4, glm::vec3(0.4f, 0.4f, 0.0f));
		// Aplica a escala
		model4 = glm::scale(model4, glm::vec3(scale, scale, scale));
		// Chamada de desenho (drawcall) e polígono preenchido com GL_TRIANGLES
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_POINTS, 0, 36);

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
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		rotateW = true;
		rotateS = false;
		rotateA = false;
		rotateD = false;
		rotateI = false;
		rotateJ = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		rotateW = false;
		rotateS = true;
		rotateA = false;
		rotateD = false;
		rotateI = false;
		rotateJ = false;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		rotateW = false;
		rotateS = false;
		rotateA = true;
		rotateD = false;
		rotateI = false;
		rotateJ = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		rotateW = false;
		rotateS = false;
		rotateA = false;
		rotateD = true;
		rotateI = false;
		rotateJ = false;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		rotateW = false;
		rotateS = false;
		rotateA = false;
		rotateD = false;
		rotateI = true;
		rotateJ = false;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
	{
		rotateW = false;
		rotateS = false;
		rotateA = false;
		rotateD = false;
		rotateI = false;
		rotateJ = true;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        scale += 0.1f; // Aumenta a escala
    }
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        scale = glm::max(0.1f, scale - 0.1f); // Diminui a escala e impede valores negativos
    }
}

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
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

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {

		//Base do cubo: 2 triângulos
		//x    y    z    r    g    b
		-0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
		-0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
		 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

		-0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
		 0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
		 0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

		 // 1º lado
		-0.5, -0.5, -0.5, 0.5, 1.0, 0.5,
		-0.5,  0.5, -0.5, 0.5, 1.0, 0.5,
		 0.5, -0.5, -0.5, 0.5, 1.0, 0.5,

		-0.5,  0.5, -0.5, 0.5, 1.0, 0.5,
		 0.5,  0.5, -0.5, 0.5, 1.0, 0.5,
		 0.5, -0.5, -0.5, 0.5, 1.0, 0.5,

		 // 2º lado
		-0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
		-0.5,  0.5, -0.5, 1.0, 0.0, 1.0,
		-0.5, -0.5, 0.5, 1.0, 0.0, 1.0,

		-0.5,  0.5, -0.5, 1.0, 0.0, 1.0,
		-0.5,  0.5,  0.5, 1.0, 0.0, 1.0,
		-0.5, -0.5, 0.5, 1.0, 0.0, 1.0,

		  // 3º lado
		-0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
		-0.5,  0.5, 0.5, 1.0, 1.0, 0.0,
		 0.5, -0.5, 0.5, 1.0, 1.0, 0.0,

		-0.5,  0.5, 0.5, 1.0, 1.0, 0.0,
		 0.5,  0.5, 0.5, 1.0, 1.0, 0.0,
		 0.5, -0.5, 0.5, 1.0, 1.0, 0.0,

		  // 4º lado
		 0.5, -0.5, 0.5, 0.0, 1.0, 1.0,
		 0.5,  0.5,  0.5, 0.0, 1.0, 1.0,
		 0.5, -0.5, -0.5, 0.0, 1.0, 1.0,

		 0.5,  0.5, 0.5, 0.0, 1.0, 1.0,
		 0.5,  0.5, -0.5, 0.0, 1.0, 1.0,
		 0.5, -0.5, -0.5, 0.0, 1.0, 1.0,

		// topo do cubo
		-0.5, 0.5, -0.5, 1.0, 1.0, 0.0,
		-0.5, 0.5,  0.5, 0.0, 1.0, 1.0,
		 0.5, 0.5, -0.5, 1.0, 0.0, 1.0,

		-0.5, 0.5, 0.5, 1.0, 1.0, 0.0,
		 0.5, 0.5,  0.5, 0.0, 1.0, 1.0,
		 0.5, 0.5, -0.5, 1.0, 0.0, 1.0,
	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}