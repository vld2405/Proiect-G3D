#include <Windows.h>
#include <locale>
#include <codecvt>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <GL/glew.h>
#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Shader.h"
#include "Model.h"
#include "FlyingCube.h"
#include "Camera.h"
#include "ETrainMovementType.h"
#include "CSkybox.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool ThirdPersonFlag = true;
bool FirstPersonFlag = false;
bool FreeCameraFlag = false;

GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;
float trainAcceleration = 0;

void Cleanup() {
	std::cout << "Cleaning up resources..." << std::endl;
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

double deltaTime = 0.0f;
std::chrono::high_resolution_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();

//void HandleInput(GLFWwindow* window, Camera& camera, float deltaTime);
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//std::vector<glm::vec3> treePositions;
//void GenerateTreePositions(float trainPathWidth, float trainPathHeight, float trainZMin, float trainZMax, int treeCount, const glm::vec3& modelMin, const glm::vec3& modelMax, const std::vector<glm::vec3>& pathPoints);

int main() {
	std::cout << "Initializing GLFW..." << std::endl;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simulare Tren", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	else {
		std::cout << "GLFW window created successfully" << std::endl;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();
	glEnable(GL_DEPTH_TEST);

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(cubeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 0.0, 3.0));
	glm::vec3 lightPos(0.0f, 2.0f, 1.0f);
	glm::vec3 cubePos(0.0f, 5.0f, 1.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	Shader lightingShader((currentPath + "\\Shaders\\PhongLight.vs").c_str(), (currentPath + "\\Shaders\\PhongLight.fs").c_str());
	Shader lightingWithTextureShader((currentPath + "\\Shaders\\PhongLightWithTexture.vs").c_str(), (currentPath + "\\Shaders\\PhongLightWithTexture.fs").c_str());
	Shader lampShader((currentPath + "\\Shaders\\Lamp.vs").c_str(), (currentPath + "\\Shaders\\Lamp.fs").c_str());

	std::string objFileName = (currentPath + "\\Models\\FlyingCube.obj");
	FlyingCube flyingCubeModel(objFileName, false);

	std::string grassLawnObjFileName = (currentPath + "\\Models\\GrassLawn\\GrassLawn.obj");
	Model grassLawnObjModel(grassLawnObjFileName, false);

	std::string trainObjFileName = (currentPath + "\\Models\\Train2\\thomas_the_tank_engine.obj");
	Model trainObjModel(trainObjFileName, false);

	std::string tree1ObjFileName = (currentPath + "\\Models\\Tree1\\Tree1.obj");
	Model tree1ObjModel(tree1ObjFileName, false);

	std::string tree2ObjFileName = (currentPath + "\\Models\\Tree2\\Tree2.obj");
	Model tree2ObjModel(tree2ObjFileName, false);

	float trainPathWidth = 10.0f;
	float trainPathHeight = 10.0f;
	float trainZMin = -100.0f;
	float trainZMax = 100.0f;
	int treeCount = 150;
	glm::vec3 modelMin(-30.0f, 0.0f, -30.0f);
	glm::vec3 modelMax(30.0f, 0.0f, 30.0f);

	std::vector<glm::vec3> pathPoints;
	for (float z = trainZMin; z <= trainZMax; z += 1.0f) {
		pathPoints.push_back(glm::vec3(0.0f, 0.0f, z));
	}

	//GenerateTreePositions(trainPathWidth, trainPathHeight, trainZMin, trainZMax, treeCount, modelMin, modelMax, pathPoints);

	//std::vector<std::pair<glm::vec3, int>> treeData;
	//for (const auto& pos : treePositions) {
	//	int randNum = rand() % 2;
	//	treeData.emplace_back(pos, randNum);
	//}

	glm::vec3 trainPos{ -2.5f, 0.0f, 0.0f };


	// Create a CSkybox object
	CSkybox skybox;

	// Set the directory and texture file names
	std::string directory = "C:/Users/Tudor/Desktop/cloudy/";
	std::string front = "bluecloud_bk.jpg";
	std::string back = "bluecloud_dn.jpg";
	std::string left = "bluecloud_ft.jpg";
	std::string right = "bluecloud_lf.jpg";
	std::string top = "bluecloud_lf.jpg";
	std::string bottom = "bluecloud_lf.jpg";

	// Load the skybox textures using the specified files
	skybox.loadSkybox(directory, front, back, left, right, top, bottom);


	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		std::chrono::high_resolution_clock::time_point currentFrame = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrame - lastFrame).count();
		lastFrame = currentFrame;

		//HandleInput(window, *pCamera, deltaTime);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightPos.x = 2.5 * cos(glfwGetTime());
		lightPos.z = 2.5 * sin(glfwGetTime());

		cubePos.x = 10 * sin(glfwGetTime());
		cubePos.z = 10 * cos(glfwGetTime());

		lightingShader.use();
		lightingShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());

		lightingShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.setMat4("view", pCamera->GetViewMatrix());

		// render the model
		glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
		model = glm::translate(model, cubePos);
		flyingCubeModel.SetRootTransf(model);
		//lightingShader.setMat4("model", model);
		flyingCubeModel.Draw(lightingShader);

		lightingWithTextureShader.use();
		lightingWithTextureShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingWithTextureShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingWithTextureShader.SetVec3("lightPos", lightPos);
		lightingWithTextureShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingWithTextureShader.setInt("texture_diffuse1", 0);
		lightingWithTextureShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingWithTextureShader.setMat4("view", pCamera->GetViewMatrix());

		glm::mat4 trainModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.f));
		trainModelMatrix = glm::translate(trainModelMatrix, trainPos);
		lightingWithTextureShader.setMat4("model", trainModelMatrix);
		trainObjModel.Draw(lightingWithTextureShader);

		trainPos.z += trainAcceleration;
		if (ThirdPersonFlag == true)
		{
			glm::vec3& cameraPos = pCamera->GetPosition();
			glm::vec3 cameraOffset(2.25f, 10.f, -20.f);
			cameraPos = cameraOffset + trainPos;
		}
		if (FreeCameraFlag == true)
		{
			// to be implemented
		}
		if (FirstPersonFlag == true)
		{
			glm::vec3& cameraPos = pCamera->GetPosition();
			glm::vec3 cameraOffset(2.25f, 4.f, -4.5f);
			cameraPos = cameraOffset + trainPos;
		}

		/*if (trainPos.z > 1 && trainPos.z < 3)
		{
			trainModelMatrix = glm::rotate(trainModelMatrix, glm::radians(45.f), glm::vec3(0, 1.f, 0));
			lightingWithTextureShader.setMat4("model", trainModelMatrix);
			trainObjModel.Draw(lightingWithTextureShader);
		}*/

		glm::mat4 grassLawnModelMatrix = glm::scale(glm::mat4(1.f), glm::vec3(3000, 1, 3000));
		lightingWithTextureShader.setMat4("model", grassLawnModelMatrix);
		grassLawnObjModel.Draw(lightingWithTextureShader);

		//for (const auto& tree : treeData) {
		//	glm::mat4 treeModelMatrix = glm::mat4(1.0f);
		//	treeModelMatrix = glm::translate(treeModelMatrix, tree.first); // Tree position

		//	if (tree.second == 0) {
		//		// Tree1: Scale and draw
		//		glm::mat4 finalModelMatrix = glm::scale(treeModelMatrix, glm::vec3(0.5f));
		//		lightingWithTextureShader.setMat4("model", finalModelMatrix);
		//		tree1ObjModel.Draw(lightingWithTextureShader);
		//	}
		//	else {
		//		// Tree2: Scale and draw
		//		glm::mat4 finalModelMatrix = glm::scale(treeModelMatrix, glm::vec3(1.0f));
		//		lightingWithTextureShader.setMat4("model", finalModelMatrix);
		//		tree2ObjModel.Draw(lightingWithTextureShader);
		//	}
		//}

		// also draw the lamp object
		lampShader.use();
		lampShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lampShader.setMat4("view", pCamera->GetViewMatrix());
		glm::mat4 lightModel = glm::translate(glm::mat4(1.0), lightPos);
		lightModel = glm::scale(lightModel, glm::vec3(0.05f)); // a smaller cube
		lampShader.setMat4("model", lightModel);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// OpenGL error logging
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}
	else {
		std::cout << "No OpenGL errors detected." << std::endl;
	}

	Cleanup();

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}