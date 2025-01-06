// ViewOBJModel.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <Windows.h>
#include <locale>
#include <codecvt>

#include <stdlib.h> // necesare pentru citirea shader-elor
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
#include "Skybox.h"
#include "Model.h"
#include "FlyingCube.h"
#include "Camera.h"
#include "ETrainMovementType.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

bool ThirdPersonFlag = true;
bool FirstPersonFlag = false;
bool FreeCameraFlag = false;

bool drawStation = false;
bool firstStation = false;

GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;
float trainAcceleration = 0;

void PlayTrainSound(const std::string& soundFilePath)
{
	std::wstring soundFilePathW = std::wstring(soundFilePath.begin(), soundFilePath.end());
	PlaySound(soundFilePathW.c_str(), NULL, SND_SYNC);
}

void PlayTrainMovementSound(const std::string& soundFilePath)
{
	std::wstring soundFilePathW = std::wstring(soundFilePath.begin(), soundFilePath.end());
	PlaySound(soundFilePathW.c_str(), NULL, SND_SYNC);
}

void PlayBackgroundMusic(const std::string& soundFilePath)
{
	std::wstring soundFilePathW = std::wstring(soundFilePath.begin(), soundFilePath.end());
	PlaySound(soundFilePathW.c_str(), NULL, SND_ASYNC | SND_LOOP);
}

void StopTrainSound()
{
	PlaySound(NULL, 0, 0);
}

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
std::chrono::high_resolution_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();

void HandleInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (FreeCameraFlag == true)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(ECameraMovementType::FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(ECameraMovementType::BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(ECameraMovementType::LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(ECameraMovementType::RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			camera.ProcessKeyboard(ECameraMovementType::UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			pCamera->ProcessKeyboard(ECameraMovementType::DOWN, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		FirstPersonFlag = true;
		FreeCameraFlag = false;
		ThirdPersonFlag = false;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		FirstPersonFlag = false;
		FreeCameraFlag = true;
		ThirdPersonFlag = false;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		FirstPersonFlag = false;
		FreeCameraFlag = false;
		ThirdPersonFlag = true;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		if (trainAcceleration < 0.2)
		{
			trainAcceleration += 0.01;
			if (trainAcceleration > 0.5) // Clamp to 0.5
				trainAcceleration = 0.5;
			std::cout << trainAcceleration << '\n';
		}
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		if (trainAcceleration > 0)
		{
			trainAcceleration -= 0.01;
			if (trainAcceleration < 0) // Clamp to 0
				trainAcceleration = 0;
			std::cout << trainAcceleration << '\n';
		}
	}

	// Check for 'H' key press to play the horn sound
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		std::string hornFilePath = "C:\\Users\\Tudor\\Desktop\\G3D\\Proiect\\Proiect-G3D\\Proiect_G3D\\Sound\\horn.wav"; // Use the correct path to your horn sound file
		std::string backgroundFilePath = "C:\\Users\\Tudor\\Desktop\\G3D\\Proiect\\Proiect-G3D\\Proiect_G3D\\Sound\\rain.wav"; // Use the correct path to your background sound file

		// Stop the background sound
		PlaySound(NULL, 0, 0);

		// Play the horn sound asynchronously
		PlayTrainSound(hornFilePath);

		// Resume the background sound
		std::wstring backgroundFilePathW = std::wstring(backgroundFilePath.begin(), backgroundFilePath.end());
		PlaySound(backgroundFilePathW.c_str(), NULL, SND_ASYNC | SND_LOOP);
	}


}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{}

std::vector<glm::vec3> treePositions;
void GenerateTreePositions(float trainPathWidth, float trainPathHeight, float trainZMin, float trainZMax, int treeCount, const glm::vec3& modelMin, const glm::vec3& modelMax, const std::vector<glm::vec3>& pathPoints) {

    for (int i = 0; i < treeCount; ++i) {
        glm::vec3 position;
        bool validPosition = false;

        while (!validPosition) {
            // Randomly generate tree position
			position.x = rand() % int(modelMax.x - modelMin.x) + modelMin.x;
			position.y = 0.0f;  // Assuming the trees are placed at ground level
			position.z = rand() % int(modelMax.z - modelMin.z) + modelMin.z;

            // Check if the position is outside the train's path zone
            if (!(position.x > -trainPathWidth / 2.0f && position.x < trainPathWidth / 2.0f &&
                  position.z > trainZMin && position.z < trainZMax)) {
                // Further check if the position is too close to the path
                bool isTooClose = false;
                for (const auto& pathPoint : pathPoints) {
                    // You can adjust the threshold distance (e.g., 5.0f) based on your needs
                    float distance = glm::distance(position, pathPoint);
                    if (distance < 5.0f) {  // Check if the tree is too close to the path
                        isTooClose = true;
                        break;
                    }
                }
                if (!isTooClose) {
                    validPosition = true;
                }
            }
        }

        treePositions.push_back(position);
    }
}

std::string grassLawnObjFileName;
std::string trainObjFileName, trainStationObjFileName;
std::string tree1ObjFileName, tree2ObjFileName;
std::string railwayObjFileName;

Model grassLawnObjModel;
Model trainObjModel, trainStationObjModel;
Model tree1ObjModel, tree2ObjModel;
Model railwayObjModel;
std::vector<Model> trainStationObjModels;

std::vector<std::string> trainStationNames = {"Brasov", "Predeal", "Sinaia", "Campina", "Ploiesti", "Bucuresti"};

void loadModels(std::string currentPath)
{
	grassLawnObjFileName = (currentPath + "\\Models\\GrassLawn\\GrassLawn.obj");
	grassLawnObjModel = Model(grassLawnObjFileName, false);

	trainObjFileName = (currentPath + "\\Models\\Train\\thomas_the_tank_engine.obj");
	trainObjModel = Model(trainObjFileName, false);

	railwayObjFileName = (currentPath + "\\Models\\Railway\\railway.obj");
	railwayObjModel = Model(railwayObjFileName, false);

	tree1ObjFileName = (currentPath + "\\Models\\Tree1\\Tree1.obj");
	tree1ObjModel = Model(tree1ObjFileName, false);

	tree2ObjFileName = (currentPath + "\\Models\\Tree2\\Tree2.obj");
	tree2ObjModel = Model(tree2ObjFileName, false);

	for (int i = 0; i < 6; ++i)
	{
		trainStationObjFileName = (currentPath + "\\Models\\Trainstation_" + trainStationNames[i] + "\\train_station.obj");
		trainStationObjModels.push_back(Model(trainStationObjFileName, false));
	}
}

const int trainStationTileOffset = 4;
static int trainStationIndex = 0;

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simulare Tren", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);


	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 0.0, 3.0));
	glm::vec3 cameraPos = pCamera->GetPosition();
	//pCamera->SetOrientation(90.f, -20.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	std::string musicFilePath(currentPath + "\\Sound\\rain.wav"); // Use an absolute path to your .wav file
	std::wstring musicFilePathW = std::wstring(musicFilePath.begin(), musicFilePath.end());
	PlaySound(musicFilePathW.c_str(), NULL, SND_ASYNC | SND_LOOP);

	Shader lightingShader((currentPath + "\\Shaders\\PhongLight.vs").c_str(), (currentPath + "\\Shaders\\PhongLight.fs").c_str());
	Shader lightingWithTextureShader((currentPath + "\\Shaders\\PhongLightWithTexture.vs").c_str(), (currentPath + "\\Shaders\\PhongLightWithTexture.fs").c_str());
	Shader lampShader((currentPath + "\\Shaders\\Lamp.vs").c_str(), (currentPath + "\\Shaders\\Lamp.fs").c_str());
	Shader skyboxShader((currentPath + "\\Shaders\\Skybox.vs").c_str(), (currentPath + "\\Shaders\\Skybox.fs").c_str());

	std::vector<std::string> skyPaths = {
		currentPath + "\\Models\\Skybox_images\\px.jpg",
		currentPath + "\\Models\\Skybox_images\\nx.jpg",
		currentPath + "\\Models\\Skybox_images\\py.jpg",
		currentPath + "\\Models\\Skybox_images\\ny.jpg",
		currentPath + "\\Models\\Skybox_images\\pz.jpg",
		currentPath + "\\Models\\Skybox_images\\nz.jpg"
	};

	Skybox skybox(skyPaths);


	loadModels(currentPath);

	//draw trees
	float trainPathWidth = 10.0f;
	float trainPathHeight = 10.0f;
	float trainZMin = -100.0f;
	float trainZMax = 100.0f;
	int treeCount = 150;
	glm::vec3 modelMin(-30.0f, 0.0f, -30.0f);  // Minimum coordinates (example)
	glm::vec3 modelMax(30.0f, 0.0f, 30.0f);    // Maximum coordinates (example)

	std::vector<glm::vec3> pathPoints;
	for (float z = trainZMin; z <= trainZMax; z += 1.0f) {
		pathPoints.push_back(glm::vec3(0.0f, 0.0f, z)); 
	}

	GenerateTreePositions(trainPathWidth, trainPathHeight, trainZMin, trainZMax, treeCount, modelMin, modelMax, pathPoints);

	std::vector<std::pair<glm::vec3, int>> treeData; // Position + Type
	for (const auto& pos : treePositions) {
		int randNum = rand() % 2; // Randomly choose 0 or 1
		treeData.emplace_back(pos, randNum); // Store position and type
	}

	glm::vec3 trainPos{-2.5f, 0.0f, -4.0f};

	// RENDER LOOP

	bool isMoving = false;

	while (!glfwWindowShouldClose(window)) {

		// per-frame time logic
		std::chrono::high_resolution_clock::time_point currentFrame = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrame - lastFrame).count();
		lastFrame = currentFrame;

		HandleInput(window, *pCamera, deltaTime);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//lightPos.x = 2.5 * cos(glfwGetTime());
		//lightPos.z = 2.5 * sin(glfwGetTime());

		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL); // Permite desenarea Skybox in spate

		skyboxShader.use();
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(pCamera->GetViewMatrix()))); // Elimina translatia
		skyboxShader.setMat4("projection", pCamera->GetProjectionMatrix());
		skyboxShader.setMat4("model", glm::mat4(1.0f));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTextureID());
		skyboxShader.setInt("skybox", 0);

		skybox.draw(skyboxShader);

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);


		lightingShader.use();
		lightingShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		//lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());

		lightingShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.setMat4("view", pCamera->GetViewMatrix());

		//flyingCubeModel.Draw(lightingShader);

		lightingWithTextureShader.use();
		lightingWithTextureShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingWithTextureShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		//lightingWithTextureShader.SetVec3("lightPos", lightPos);
		lightingWithTextureShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingWithTextureShader.setInt("texture_diffuse1", 0);
		lightingWithTextureShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingWithTextureShader.setMat4("view", pCamera->GetViewMatrix());

		glm::mat4 trainModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.f));
		trainModelMatrix = glm::translate(trainModelMatrix, trainPos);
		lightingWithTextureShader.setMat4("model", trainModelMatrix);
		trainObjModel.Draw(lightingWithTextureShader);

		trainPos.z += trainAcceleration;

		if (trainAcceleration > 0 && !isMoving)
		{
			isMoving = true;
			std::string movingMusicFilePath = currentPath + "\\Sound\\train_sound.wav"; // Use the correct path to your moving music file
			PlayBackgroundMusic(movingMusicFilePath);
		}
		else if (trainAcceleration == 0 && isMoving)
		{
			isMoving = false;
			std::string idleMusicFilePath = currentPath + "\\Sound\\rain.wav"; // Use the correct path to your idle music file
			PlayBackgroundMusic(idleMusicFilePath);
		}

		if (ThirdPersonFlag == true)
		{
			glm::vec3& cameraPos = pCamera->GetPosition();
			glm::vec3 cameraOffset(2.25f, 10.f, -20.f);
			//pCamera->SetOrientation(180.0f, -20.0f);
			cameraPos = cameraOffset + trainPos;
		}
		if (FirstPersonFlag == true)
		{
			glm::vec3& cameraPos = pCamera->GetPosition();
			glm::vec3 cameraOffset(2.25f, 4.f, -4.5f);
			cameraPos = cameraOffset + trainPos;
		}
		
		const float lawnLength = 60.0f;
		const float lawnHalfLength = lawnLength / 2.0f; // midpoint of lawn
		const float railwayLength = 2.82f;
		
		static int currentLawnSegment = 0;

		// Check if the train has passed half of the current lawn
		if (trainPos.z > (currentLawnSegment + 1) * lawnLength - lawnHalfLength) {
			currentLawnSegment++;
		}
		else if (trainPos.z < currentLawnSegment * lawnLength - lawnHalfLength) {
			currentLawnSegment--;
		}

		if (currentLawnSegment == 0)
		{
			firstStation == true;
		}

		if (currentLawnSegment % 4 == 2 || currentLawnSegment == 0)
		{
			drawStation = true;
		}

		if ((drawStation == true && currentLawnSegment % 4 == 2))
		{
			glm::mat4 trainStationModelMatrix = glm::mat4(1.0f);
			trainStationModelMatrix = glm::translate(trainStationModelMatrix, glm::vec3(-11.0f, 0.40f, (currentLawnSegment + 1) * lawnLength));
			trainStationModelMatrix = glm::scale(trainStationModelMatrix + glm::mat4(1.f), glm::vec3(1.0f));
			trainStationModelMatrix = glm::rotate(trainStationModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			lightingWithTextureShader.setMat4("model", trainStationModelMatrix);
			
			trainStationObjModels[trainStationIndex % trainStationObjModels.size()].Draw(lightingWithTextureShader);
			
			trainStationIndex++;
			drawStation = false;
		}

		for (int i = -1; i <= 1; ++i) {

			glm::mat4 grassLawnModelMatrix_middle = glm::mat4(1.f);
			grassLawnModelMatrix_middle = glm::translate(grassLawnModelMatrix_middle, glm::vec3(0.f, 0.f, (currentLawnSegment + i) * lawnLength));
			grassLawnModelMatrix_middle = glm::scale(grassLawnModelMatrix_middle, glm::vec3(3000.f, 1.f, 3000.f));
			lightingWithTextureShader.setMat4("model", grassLawnModelMatrix_middle);
			grassLawnObjModel.Draw(lightingWithTextureShader);

			for (int j = -2; j < 3; ++j)
			{
				glm::mat4 railwayModelMatrix = glm::mat4(1.f);
				railwayModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.f, 1.f, 4.f));
				railwayModelMatrix = glm::translate(railwayModelMatrix, glm::vec3(-0.3, 0.2, j * railwayLength + (currentLawnSegment + i) * lawnLength / 4));
				railwayModelMatrix = glm::rotate(railwayModelMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
				lightingWithTextureShader.setMat4("model", railwayModelMatrix);
				railwayObjModel.Draw(lightingWithTextureShader);
			}

			for (const auto& tree : treeData) {
				glm::mat4 treeMatrix = glm::translate(glm::mat4(1.0f), tree.first + glm::vec3(0.f, 0.f, (currentLawnSegment + i) * lawnLength));
				//lightingWithTextureShader.setMat4("model", treeMatrix);

				//if (tree.second == 0)
				//{
					lightingWithTextureShader.setMat4("model", treeMatrix);
					tree1ObjModel.Draw(lightingWithTextureShader);
				//}
				/*else
				{
					treeMatrix = glm::scale(treeMatrix + glm::mat4(1.0f), glm::vec3(1.0f));
					lightingWithTextureShader.setMat4("model", treeMatrix);
					tree2ObjModel.Draw(lightingWithTextureShader);
				}*/
			}
		}

		// also draw the lamp object
		lampShader.use();
		lampShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lampShader.setMat4("view", pCamera->GetViewMatrix());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	PlaySound(NULL, 0, 0); // Stop the music
	Cleanup();
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