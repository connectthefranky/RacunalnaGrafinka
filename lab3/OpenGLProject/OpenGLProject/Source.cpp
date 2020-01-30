#include <glad/glad.h> 
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ctime>
#include <cmath>
#include <iostream>
#include <vector>

const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 normal; \n"
"\n"
"out vec4 vertexColor;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform float MBB1;\n"
"uniform float MBB2;\n"
"uniform vec3 obj2Pos;\n"
"uniform float deltaTime;\n"
"\n"
"void main() {\n"
"   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
"	vec3 vertexObj2 = obj2Pos - position;\n"
"	float transmittance = exp(-0.8f * distance(obj2Pos, position));\n"
"	float radiance = min(MBB2, MBB1 + MBB2 * deltaTime * transmittance * max(0, dot(normalize(normal), normalize(vertexObj2))));\n"
"	float red = (radiance - 1.55923f) / (19.4616f - 1.55923f);\n"
"	float blue = 1 - red;\n"
"	if (normal.x == 10.0f) {\n"
"		red = 1.0f;\n"
"		blue = 0.0f;\n"
"	}\n"
"   vertexColor = vec4(red, 0.0f, blue, 1.0f);\n"
"}\0";

const char* fragmentShaderSource =
"#version 330 core\n"
"in vec4 vertexColor;\n"
"\n"
"out vec4 FragColor;\n"
"void main() {\n"
"	FragColor = vertexColor;\n"
"}\0";

float vertices[] = {
	//position        //normal            
	0.0f, 0.0f, 0.0f, -1.0f, -1.0f, -1.0f,
	1.0f, 0.0f, 0.0f,  1.0f, -1.0f, -1.0f,
	1.0f, 0.0f, 1.0f,  1.0f, -1.0f,  1.0f,
	0.0f, 0.0f, 1.0f, -1.0f, -1.0f,  1.0f,
	0.5f, 0.0f, 0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, 0.5f, 0.0f,  0.0f,  0.0f, -1.0f,
	1.0f, 0.5f, 0.0f,  1.0f,  0.0f,  0.0f,
	0.5f, 0.5f, 0.0f,  0.0f,  0.0f,  1.0f,
	0.0f, 0.5f, 0.0f, -1.0f,  0.0f,  0.0f,
	0.0f, 1.0f, 0.0f, -1.0f,  1.0f, -1.0f,
	1.0f, 1.0f, 0.0f,  1.0f,  1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
	0.0f, 1.0f, 1.0f, -1.0f,  1.0f,  1.0f,
	0.5f, 1.0f, 0.5f,  0.0f,  1.0f,  0.0f,

	2.0f, 2.0f, 2.0f, 10.0f, 0.0f, 0.0f,
	2.5f, 2.0f, 2.0f, 10.0f, 0.0f, 0.0f,
	2.5f, 2.0f, 2.5f, 10.0f, 0.0f, 0.0f,
	2.0f, 2.0f, 2.5f, 10.0f, 0.0f, 0.0f,
	2.0f, 2.5f, 2.0f, 10.0f, 0.0f, 0.0f,
	2.5f, 2.5f, 2.0f, 10.0f, 0.0f, 0.0f,
	2.5f, 2.5f, 2.5f, 10.0f, 0.0f, 0.0f,
	2.0f, 2.5f, 2.5f, 10.0f, 0.0f, 0.0f
};

unsigned int indices[] = {
	0,1,5,
	1,10,5,
	10,9,5,
	9,0,5,
	1,2,6,
	2,11,6,
	11,10,6,
	10,1,6,
	2,3,7,
	3,12,7,
	12,11,7,
	11,2,7,
	3,0,8,
	0,9,8,
	9,12,8,
	12,3,8,
	9,10,13,
	10,11,13,
	11,12,13,
	12,9,13,
	0,1,4,
	1,2,4,
	2,3,4,
	3,0,4,

	14,15,19,
	14,19,18,
	15,16,20,
	15,20,19,
	16,17,21,
	16,21,20,
	17,14,18,
	17,18,21,
	18,19,20,
	18,20,21,
	17,16,15,
	17,15,14
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

float coldFunction(float x, float maxRadiance) {
	return 1 - (x / maxRadiance);
}

float hotFunction(float x, float maxRadiance) {
	return x / maxRadiance;
}

float extinction(float a) {
	return 0.8f;
}

float transmittance(float a, float x) {
	return exp(-extinction(a) * x);
}

float cToK(float C) {
	return C + 274.15f;
}

float blackBodyRadiance(float t) {
	float a = 3.8f;

	float c1 = 3.7418 * pow(10, 8);
	float c2 = 1.4388 * pow(10, 4);
	return c1 / ((pow(a, 5) * (exp(c2 / (a * t)) - 1)));
}

float startTime;
float pathRadiance;
float emissivity = 1.0f;
float obj1Radiance = emissivity * blackBodyRadiance(300);
float obj2Radiance = emissivity * blackBodyRadiance(375);
float objDist = sqrt(3 * 4);

unsigned int vertexShader;
unsigned int fragmentShader;
unsigned int shaderProgram;
unsigned int VAO;
unsigned int VBO;
unsigned int EBO;

void renderingCommands() {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);

	float time = glfwGetTime();

	float deltaTime = time - startTime;
	float MBB1 = 1.55923f;
	float MBB2 = 19.4616f;

	int obj2PosLoc = glGetUniformLocation(shaderProgram, "obj2Pos");
	glUniform3f(obj2PosLoc, 2.0f, 2.0f, 2.0f);

	int deltaTimeLoc = glGetUniformLocation(shaderProgram, "deltaTime");
	glUniform1f(deltaTimeLoc, deltaTime);

	int MBB1Loc = glGetUniformLocation(shaderProgram, "MBB1");
	glUniform1f(MBB1Loc, MBB1);

	int MBB2Loc = glGetUniformLocation(shaderProgram, "MBB2");
	glUniform1f(MBB2Loc, MBB2);

	const float radius = 10.0f;
	float camX = sin(glfwGetTime()) * radius;
	float camZ = cos(glfwGetTime()) * radius;

	glm::mat4 view = glm::mat4(1.0f);
	// note that we're translating the scene in the reverse direction of where we want to move
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	int viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Normal cube
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 0.0f));
	int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 108, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void loadAndAttachShaders() {
	// Load vertex Shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Checke compilation result for vertex shader
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// Load fragment Shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Checke compilation result for vertex shader
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// Attach shaders to program and link program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check compilation results for program
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	}
	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

int main() {

	startTime = (float)glfwGetTime();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// bind Vertex Array Object
	glBindVertexArray(VAO);
	// copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	loadAndAttachShaders();

	// Render loop
	while (!glfwWindowShouldClose(window)) {

		// Inputs
		processInput(window);

		// Render commands
		renderingCommands();

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}