// ViewOBJModel.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <Windows.h>
#include <locale>
#include <codecvt>
#include <string>

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 
#include <chrono>
#include <thread>
#include <unordered_map>

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



// timing
double deltaTime = 0.0f;	// time between current frame and last frame
std::chrono::high_resolution_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();
auto lastHornTime = std::chrono::steady_clock::now();

bool ThirdPersonFlag = true;
bool FirstPersonFlag = false;
bool FreeCameraFlag = false;
bool hornKeyPressed = false;
bool isNight = false;
bool nightKeyPressed = false;

bool drawStation = false;
bool firstStation = false;

GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;
float trainAcceleration = 0;

const int trainStationTileOffset = 4;
const int hornCooldownMs = 1000; // 1-second cooldown

std::vector<glm::vec3> treePositions;
std::vector<glm::vec3> treePositionsTrainStation;
std::vector<float> treeScales;

std::vector<Model> allModels;

glm::vec3 trainPos{ -2.5f, 0.0f, -4.0f };
glm::vec3 lightPos = trainPos + glm::vec3(0.f, 50.f, 0.f);

std::string movingTrainSoundFilePath;
std::string idleMusicFilePath;
std::string hornFilePath;

std::string grassLawnObjFileName;
std::string trainObjFileName, trainStationObjFileName;
std::string tree1ObjFileName, tree2ObjFileName;
std::string railwayObjFileName;
std::string rockObjFileName;
std::string mountainObjFileName;
std::string ApartmentObjFileName;

Model rockObjModel;
Model mountainObjModel;
Model grassLawnObjModel;
Model trainObjModel, trainStationObjModel;
Model tree1ObjModel, tree2ObjModel;
Model railwayObjModel;
Model ApartmentObjModel;
std::vector<Model> trainStationObjModels;

std::vector<std::string> trainStationNames = { "Brasov", "Predeal", "Sinaia", "Campina", "Ploiesti", "Bucuresti" };
std::vector<std::pair<int, int>> activeStations;
std::vector<std::pair<glm::vec3, int>> treeData;
std::vector<std::pair<glm::vec3, int>> treeData2;

const float lawnLength = 60.0f;
const float lawnHalfLength = lawnLength * 0.75f;  // midpoint of lawn
const float railwayLength = 2.82f;

static int currentLawnSegment = 0;

void SetVolume(float volume)
{
	// Clamp volume between 0.0f and 1.0f
	volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;

	// Convert to DWORD volume format
	DWORD dwVolume = static_cast<DWORD>(volume * 0xFFFF);
	dwVolume = (dwVolume & 0xFFFF) | (dwVolume << 16); // Left and Right channel

	waveOutSetVolume(0, dwVolume); // Set master volume
}

void PlayTrainSound(const std::string& soundFilePath, const std::string& movingSoundFilePath, const std::string& idleSoundFilePath)
{
	// Convert the file paths to wide strings
	std::wstring hornFilePathW = std::wstring(hornFilePath.begin(), hornFilePath.end());
	std::wstring idleSoundFilePathW = std::wstring(idleMusicFilePath.begin(), idleMusicFilePath.end());
	std::wstring movingSoundFilePathW = std::wstring(movingTrainSoundFilePath.begin(), movingTrainSoundFilePath.end());

	// Play the horn sound synchronously (blocking the thread it's on)
	PlaySound(hornFilePathW.c_str(), NULL, SND_SYNC);

	// After the horn finishes, check if the train is moving
	if (trainAcceleration > 0) {
		// If the train is moving, play the moving sound
		PlaySound(movingSoundFilePathW.c_str(), NULL, SND_ASYNC | SND_LOOP);
	}
	else {
		// If the train is not moving, play the idle sound
		PlaySound(idleSoundFilePathW.c_str(), NULL, SND_ASYNC | SND_LOOP);
	}
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

std::wstring ConvertToWideString(const std::string& str) {
	return std::wstring(str.begin(), str.end());
}

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

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (trainAcceleration < 20)
		{
			trainAcceleration += 0.05;
			if (trainAcceleration > 20) // Clamp to 4
				trainAcceleration = 20;
			std::cout << trainAcceleration << '\n';
		}
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		if (trainAcceleration > 0)
		{
			trainAcceleration -= 0.1;
			if (trainAcceleration < 0) // Clamp to 0
				trainAcceleration = 0;
			std::cout << trainAcceleration << '\n';
		}
	}

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !nightKeyPressed)
	{
		isNight = !isNight;
		nightKeyPressed = true;
		std::cout << "Night mode toggled: " << (isNight ? "ON" : "OFF") << std::endl;
	}
	else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE)
	{
		nightKeyPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		auto now = std::chrono::steady_clock::now();
		if (!hornKeyPressed && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHornTime).count() > hornCooldownMs)
		{
			hornKeyPressed = true;
			lastHornTime = now;

			// Play the horn sound on a separate thread
			std::thread hornThread(PlayTrainSound, hornFilePath, movingTrainSoundFilePath, idleMusicFilePath);
			hornThread.detach();
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
	{
		hornKeyPressed = false;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
}

GLuint CreateDepthMapFBO(GLuint& depthMapTexture, const unsigned int shadowWidth, const unsigned int shadowHeight) {
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// Creați textura pentru hartă de adâncime
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Bordura pentru cazuri unde nu există informație de adâncime
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Atașează textura la framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Depth framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return depthMapFBO;
}

void RenderDepthMap(GLuint& depthMapFBO, Shader& shaderProgram, glm::mat4& lightSpaceMatrix, Camera& camera) {
	glViewport(0, 0, 8192, 8192); // Rezoluția hărții de adâncime
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	shaderProgram.use();

	shaderProgram.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	glm::mat4 trainModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.f));
	trainModelMatrix = glm::translate(trainModelMatrix, trainPos);
	shaderProgram.setMat4("model", trainModelMatrix);
	trainObjModel.Draw(shaderProgram);

	int segmentsX = 3; // Number of grass segments in the X direction (sideways)

	for (int i = -1; i <= 3; ++i) {
		for (int l = 0; l < segmentsX; ++l) {
			glm::mat4 grassLawnModelMatrix_middle = glm::mat4(1.f);

			// Adjust the translation to move in the X direction
			grassLawnModelMatrix_middle = glm::translate(grassLawnModelMatrix_middle, glm::vec3((l - segmentsX / 2) * lawnLength, -0.2f, (currentLawnSegment + i) * lawnLength));

			// Scale the grass model
			grassLawnModelMatrix_middle = glm::scale(grassLawnModelMatrix_middle, glm::vec3(3000.f, 500.f, 3000.f));

			// Set the model matrix and draw the grass lawn
			shaderProgram.setMat4("model", grassLawnModelMatrix_middle);
			grassLawnObjModel.Draw(shaderProgram);

			// Handle railway model rendering
			for (int j = -2; j < 3; ++j) {
				glm::mat4 railwayModelMatrix = glm::mat4(1.f);
				railwayModelMatrix = glm::scale(railwayModelMatrix, glm::vec3(1.f, 1.f, 4.f));
				railwayModelMatrix = glm::translate(railwayModelMatrix, glm::vec3(-0.3f, 0.2f, j * railwayLength + (currentLawnSegment + i) * lawnLength / 4));
				railwayModelMatrix = glm::rotate(railwayModelMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
				shaderProgram.setMat4("model", railwayModelMatrix);
				railwayObjModel.Draw(shaderProgram);
			}

			// Tree and rock rendering
			if ((currentLawnSegment + i + 2) % 4 != 2) {
				for (size_t k = 0; k < treeData.size(); ++k) {
					const auto& tree = treeData[k];

					// Modify X to place trees sideways (in the X direction)
					float offsetX = (l - segmentsX / 2) * lawnLength; // Adjust for side positioning
					glm::mat4 treeMatrix = glm::translate(glm::mat4(1.0f), tree.first + glm::vec3(offsetX, 0.f, (currentLawnSegment + i) * lawnLength));
					treeMatrix = glm::scale(treeMatrix, glm::vec3(treeScales[k])); // Apply scale factor

					// Set the model matrix for the tree
					shaderProgram.setMat4("model", treeMatrix);

					// Choose the tree model and texture
					if (tree.second == 0) {
						tree1ObjModel.Draw(shaderProgram);
						Texture treeTexture = tree1ObjModel.textures_loaded[0];
						glBindTexture(GL_TEXTURE_2D, treeTexture.id);
					}
					else {
						tree2ObjModel.Draw(shaderProgram);
						Texture treeTexture = tree2ObjModel.textures_loaded[0];
						glBindTexture(GL_TEXTURE_2D, treeTexture.id);
					}
				}
			}
			else {
				for (size_t k = 0; k < treeData2.size(); ++k) {
					const auto& tree = treeData2[k];

					float offsetX = (l - segmentsX / 2) * lawnLength;
					glm::mat4 treeMatrix = glm::translate(glm::mat4(1.0f), tree.first + glm::vec3(offsetX, 0.f, (currentLawnSegment + i) * lawnLength));
					treeMatrix = glm::scale(treeMatrix, glm::vec3(treeScales[k])); // Apply scale factor

					// Set the model matrix for the tree
					shaderProgram.setMat4("model", treeMatrix);

					// Choose the tree model and texture
					if (tree.second == 0) {
						tree1ObjModel.Draw(shaderProgram);
						Texture treeTexture = tree1ObjModel.textures_loaded[0];
						glBindTexture(GL_TEXTURE_2D, treeTexture.id);
					}
					else {
						tree2ObjModel.Draw(shaderProgram);
						Texture treeTexture = tree2ObjModel.textures_loaded[0];
						glBindTexture(GL_TEXTURE_2D, treeTexture.id);
					}
				}

				// Train station rendering (accessing sideways segments)
				glm::mat4 trainStationModelMatrix = glm::mat4(1.0f);
				// Adjust the train station's X position using `l`
				trainStationModelMatrix = glm::translate(trainStationModelMatrix, glm::vec3(-11.0f, 0.40f, (currentLawnSegment + i) * lawnLength));
				trainStationModelMatrix = glm::scale(trainStationModelMatrix, glm::vec3(2.0f));
				trainStationModelMatrix = glm::rotate(trainStationModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				shaderProgram.setMat4("model", trainStationModelMatrix);

				// Determine which train station model to use
				int stationIndex = ((currentLawnSegment + i) / 4) % trainStationNames.size();
				trainStationObjModels[stationIndex].Draw(shaderProgram);
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // Revenim la rezoluția normală
}

struct Station {
	glm::vec3 position;  // Position of the station's center
	glm::vec3 size;      // Width, Height, Depth of the station
};

void GenerateLawnSegmentTreePositions(float trainPathWidth, float trainPathHeight, float trainZMin, float trainZMax, int treeCount,
	const glm::vec3& modelMin, const glm::vec3& modelMax,
	const std::vector<glm::vec3>& pathPoints, const Station& station,
	const glm::vec3& lawnSegmentMin, const glm::vec3& lawnSegmentMax)
{
	for (int i = 0; i < treeCount; ++i) {
		glm::vec3 position;
		bool validPosition = false;

		while (!validPosition) {
			// Randomly generate tree position within the LawnSegment boundaries
			position.x = rand() % int(lawnSegmentMax.x - lawnSegmentMin.x) + lawnSegmentMin.x;
			position.y = 0.0f;  // Trees placed at ground level
			position.z = rand() % int(lawnSegmentMax.z - lawnSegmentMin.z) + lawnSegmentMin.z;

			// Check if the position is outside the train's path zone
			if (!(position.x > -trainPathWidth / 2.0f && position.x < trainPathWidth / 2.0f &&
				position.z > trainZMin && position.z < trainZMax)) {
				bool isInStationArea = position.x > (station.position.x - station.size.x / 2.0f) &&
					position.x < (station.position.x + station.size.x / 2.0f) &&
					position.z >(station.position.z - station.size.z / 2.0f) &&
					position.z < (station.position.z + station.size.z / 2.0f);

				if (!isInStationArea) {
					validPosition = true;
				}
			}
		}

		// Store the valid tree position and scale
		treePositionsTrainStation.push_back(position);
		float scale = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.5f + 0.75f; // Random scale between 0.75 and 1.25
		treeScales.push_back(scale);
	}
}

void GenerateTreePositions(float trainPathWidth, float trainPathHeight, float trainZMin, float trainZMax, int treeCount, const glm::vec3& modelMin, const glm::vec3& modelMax, const std::vector<glm::vec3>& pathPoints, const Station station)
{
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
		float scale = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.5f + 0.75f; // Random scale between 0.75 and 1.25
		treeScales.push_back(scale);
	}
}



void loadSounds(std::string currentPath)
{
	movingTrainSoundFilePath = currentPath + "\\Sound\\train_sound.wav";
	idleMusicFilePath = currentPath + "\\Sound\\rain.wav";
	hornFilePath = currentPath + "\\Sound\\horn.wav";
}

void loadModels(std::string currentPath)
{
	grassLawnObjFileName = (currentPath + "\\Models\\GrassLawn\\GrassLawn.obj");
	grassLawnObjModel = Model(grassLawnObjFileName, false);

	allModels.push_back(grassLawnObjModel);

	trainObjFileName = (currentPath + "\\Models\\Train\\thomas_the_tank_engine.obj");
	trainObjModel = Model(trainObjFileName, false);

	allModels.push_back(trainObjModel);

	railwayObjFileName = (currentPath + "\\Models\\Railway\\railway.obj");
	railwayObjModel = Model(railwayObjFileName, false);

	allModels.push_back(railwayObjModel);

	tree1ObjFileName = (currentPath + "\\Models\\Tree1\\Tree1.obj");
	tree1ObjModel = Model(tree1ObjFileName, true);

	allModels.push_back(tree1ObjModel);

	tree2ObjFileName = (currentPath + "\\Models\\Tree2\\Tree2.obj");
	tree2ObjModel = Model(tree2ObjFileName, false);

	allModels.push_back(tree2ObjModel);

	rockObjFileName = (currentPath + "\\Models\\Rock1\\Rock1.obj");
	rockObjModel = Model(rockObjFileName, false);

	allModels.push_back(rockObjModel);

	ApartmentObjFileName = (currentPath + "\\Models\\Apartment_Block\\Apartment.obj");
	ApartmentObjModel = Model(ApartmentObjFileName, false);

	allModels.push_back(ApartmentObjModel);


	for (int i = 0; i < 6; ++i)
	{
		trainStationObjFileName = (currentPath + "\\Models\\Trainstation_" + trainStationNames[i] + "\\train_station.obj");
		trainStationObjModels.push_back(Model(trainStationObjFileName, false));
	}
}

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

	SetVolume(0.05f); // 0.5f means 50% volume

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

	loadModels(currentPath);
	loadSounds(currentPath);

	std::wstring idleMusicFilePathW = std::wstring(idleMusicFilePath.begin(), idleMusicFilePath.end());
	PlaySound(idleMusicFilePathW.c_str(), NULL, SND_ASYNC | SND_LOOP);

	Shader lightingShader((currentPath + "\\Shaders\\PhongLight.vs").c_str(), (currentPath + "\\Shaders\\PhongLight.fs").c_str());
	Shader lightingWithTextureShader((currentPath + "\\Shaders\\PhongLightWithTexture.vs").c_str(), (currentPath + "\\Shaders\\PhongLightWithTexture.fs").c_str());
	Shader lampShader((currentPath + "\\Shaders\\Lamp.vs").c_str(), (currentPath + "\\Shaders\\Lamp.fs").c_str());
	Shader skyboxShader((currentPath + "\\Shaders\\Skybox.vs").c_str(), (currentPath + "\\Shaders\\Skybox.fs").c_str());
	Shader shaderProgram((currentPath + "\\Shaders\\DepthMap.vs").c_str(), (currentPath + "\\Shaders\\DepthMap.fs").c_str());

	//std::vector<std::string> skyPaths = {
	//	currentPath + "\\Models\\Skybox_images\\px.jpg",
	//	currentPath + "\\Models\\Skybox_images\\nx.jpg",
	//	currentPath + "\\Models\\Skybox_images\\py.jpg",
	//	currentPath + "\\Models\\Skybox_images\\ny.jpg",
	//	currentPath + "\\Models\\Skybox_images\\pz.jpg",
	//	currentPath + "\\Models\\Skybox_images\\nz.jpg"
	//};

	std::vector<std::string> skyPaths = {

	currentPath + "\\Models\\Skybox_images\\bluecloud_ft.jpg",
	currentPath + "\\Models\\Skybox_images\\bluecloud_bk.jpg",

	currentPath + "\\Models\\Skybox_images\\bluecloud_up.jpg",
	currentPath + "\\Models\\Skybox_images\\bluecloud_dn.jpg",

	currentPath + "\\Models\\Skybox_images\\bluecloud_rt.jpg",
	currentPath + "\\Models\\Skybox_images\\bluecloud_lf.jpg"
	};


	Skybox skybox(skyPaths);

	//draw trees
	float trainPathWidth = 10.0f;
	float trainPathHeight = 10.0f;
	float trainZMin = -100.0f;
	float trainZMax = 100.0f;
	int treeCount = 50;
	glm::vec3 modelMin(-30.0f, 0.0f, -30.0f);  // Minimum coordinates (example)
	glm::vec3 modelMax(30.0f, 0.0f, 30.0f);    // Maximum coordinates (example)

	std::vector<glm::vec3> pathPoints;
	for (float z = trainZMin; z <= trainZMax; z += 1.0f) {
		pathPoints.push_back(glm::vec3(0.0f, 0.0f, z));
	}

	Station trainStation = {
		glm::vec3(-11.0f, 0.40f, 0.80f),  // Position of the station
		glm::vec3(25.0f, 5.0f, 50.0f)  // Size (width, height, depth)
	};

	GenerateTreePositions(trainPathWidth, trainPathHeight, trainZMin, trainZMax, treeCount, modelMin, modelMax, pathPoints, trainStation);

	; // Position + Type
	for (const auto& pos : treePositions) {
		int randNum = rand() % 2; // Randomly choose 0 or 1
		treeData.emplace_back(pos, randNum); // Store position and type
	}

	treeCount = 50;
	for (float z = trainZMin; z <= trainZMax; z += 1.0f) {
		pathPoints.push_back(glm::vec3(0.0f, 0.0f, z));
	}
	GenerateLawnSegmentTreePositions(trainPathWidth, trainPathHeight, trainZMin, trainZMax, treeCount, modelMin, modelMax, pathPoints, trainStation, modelMin, modelMax);


	std::vector<std::pair<glm::vec3, int>> treeData2; // Position + Type
	for (const auto& pos : treePositionsTrainStation) {
		int randNum = rand() % 2; // Randomly choose 0 or 1
		treeData2.emplace_back(pos, randNum); // Store position and type
	}


	GLuint depthMapFBO, depthMap;
	depthMapFBO = CreateDepthMapFBO(depthMap, 8192, 8192);

	/*glm::mat4 lightProjection, lightView, lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(0.0f, 10.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;*/



	// RENDER LOOP

	bool isMoving = false;

	const float rockScaleFactor = 0.4f; // Adjust this value to make the rocks smaller

	while (!glfwWindowShouldClose(window)) {

		// per-frame time logic
		std::chrono::high_resolution_clock::time_point currentFrame = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrame - lastFrame).count();
		lastFrame = currentFrame;

		HandleInput(window, *pCamera, deltaTime);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL); // Permite desenarea Skybox in spate

		skyboxShader.use();
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(pCamera->GetViewMatrix())));
		skyboxShader.setMat4("projection", pCamera->GetProjectionMatrix());
		skyboxShader.setMat4("model", glm::mat4(1.0f));

		if (!isNight)
		{
			skyboxShader.SetVec4("lightColor", { 1.f, 1.f, 1.f, 1.f });
		}
		else
		{
			skyboxShader.SetVec4("lightColor", { 0.1f, 0.1f, 0.2f, 1.f });
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTextureID());
		skyboxShader.setInt("skybox", 0);

		skybox.draw(skyboxShader);

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		lightPos = trainPos + glm::vec3(0.f, 100.f, 100.f);

		glm::vec3 lightTarget = trainPos;
		glm::vec3 lightDir = glm::normalize(lightPos - lightTarget);
		//glm::mat4 lightProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 0.1f, 300.0f);
		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 300.0f);

		/*glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		if (glm::dot(lightDir, up) > 0.99f) {
			up = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, up);*/
		glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		// Render Depth Map
		RenderDepthMap(depthMapFBO, shaderProgram, lightSpaceMatrix, *pCamera);


		lightingShader.use();
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("objectColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());

		lightingShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.setMat4("view", pCamera->GetViewMatrix());

		if (!isNight)
		{
			lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		}
		else
		{
			lightingShader.SetVec3("lightColor", .1f, .1f, .1f);
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		lightingWithTextureShader.use();
		lightingWithTextureShader.SetVec3("globalAmbient", 0.05f, 0.05f, 0.05f);
		lightingWithTextureShader.SetVec3("objectColor", 1.f, 1.f, 1.f);

		if (!isNight)
		{
			lightingWithTextureShader.SetVec3("lightColor", { 1.f, 1.f, 1.f });
		}
		else
		{
			lightingWithTextureShader.SetVec3("lightColor", { 0.3f, 0.3f, 0.4f });
		}

		lightingWithTextureShader.SetVec3("lightPos", lightPos);
		lightingWithTextureShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingWithTextureShader.setInt("texture_diffuse1", 0);
		lightingWithTextureShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingWithTextureShader.setMat4("view", pCamera->GetViewMatrix());
		lightingWithTextureShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		lightingWithTextureShader.setInt("shadowMap", 1);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		/// Draw TRAIN

		glm::mat4 trainModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.f));
		trainModelMatrix = glm::translate(trainModelMatrix, trainPos);
		lightingWithTextureShader.setMat4("model", trainModelMatrix);
		trainObjModel.Draw(lightingWithTextureShader);

		trainPos.z += trainAcceleration * deltaTime;


		if (trainAcceleration > 0 && !isMoving)
		{
			isMoving = true;
			PlayBackgroundMusic(movingTrainSoundFilePath);
		}
		else if (trainAcceleration == 0 && isMoving)
		{
			isMoving = false;
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

		// Check if the train has passed half of the current lawn
		if (trainPos.z > (currentLawnSegment + 1) * lawnLength + lawnHalfLength) {
			currentLawnSegment++;
		}
		else if (trainPos.z < currentLawnSegment * lawnLength - lawnHalfLength) {
			currentLawnSegment--;
		}
		if (currentLawnSegment == 0)
		{
			firstStation == true;
		}


		int segmentsX = 3; // Number of grass segments in the X direction (sideways)

		for (int i = -1; i <= 3; ++i) {
			for (int l = 0; l < segmentsX; ++l) {
				glm::mat4 grassLawnModelMatrix_middle = glm::mat4(1.f);

				// Adjust the translation to move in the X direction
				grassLawnModelMatrix_middle = glm::translate(grassLawnModelMatrix_middle, glm::vec3((l - segmentsX / 2) * lawnLength, -0.2f, (currentLawnSegment + i) * lawnLength));

				// Scale the grass model
				grassLawnModelMatrix_middle = glm::scale(grassLawnModelMatrix_middle, glm::vec3(3000.f, 500.f, 3000.f));

				// Set the model matrix and draw the grass lawn
				lightingWithTextureShader.setMat4("model", grassLawnModelMatrix_middle);
				grassLawnObjModel.Draw(lightingWithTextureShader);

				// Handle railway model rendering
				for (int j = -2; j < 3; ++j) {
					glm::mat4 railwayModelMatrix = glm::mat4(1.f);
					railwayModelMatrix = glm::scale(railwayModelMatrix, glm::vec3(1.f, 1.f, 4.f));
					railwayModelMatrix = glm::translate(railwayModelMatrix, glm::vec3(-0.3f, 0.2f, j * railwayLength + (currentLawnSegment + i) * lawnLength / 4));
					railwayModelMatrix = glm::rotate(railwayModelMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
					lightingWithTextureShader.setMat4("model", railwayModelMatrix);
					railwayObjModel.Draw(lightingWithTextureShader);
				}

				// Tree and rock rendering
				if ((currentLawnSegment + i + 2) % 4 != 2) {
					for (size_t k = 0; k < treeData.size(); ++k) {
						const auto& tree = treeData[k];

						// Modify X to place trees sideways (in the X direction)
						float offsetX = (l - segmentsX / 2) * lawnLength; // Adjust for side positioning
						glm::mat4 treeMatrix = glm::translate(glm::mat4(1.0f), tree.first + glm::vec3(offsetX, 0.f, (currentLawnSegment + i) * lawnLength));
						treeMatrix = glm::scale(treeMatrix, glm::vec3(treeScales[k])); // Apply scale factor

						// Set the model matrix for the tree
						lightingWithTextureShader.setMat4("model", treeMatrix);

						// Choose the tree model and texture
						if (tree.second == 0) {
							tree1ObjModel.Draw(lightingWithTextureShader);
							Texture treeTexture = tree1ObjModel.textures_loaded[0];
							glBindTexture(GL_TEXTURE_2D, treeTexture.id);
						}
						else {
							tree2ObjModel.Draw(lightingWithTextureShader);
							Texture treeTexture = tree2ObjModel.textures_loaded[0];
							glBindTexture(GL_TEXTURE_2D, treeTexture.id);
						}
					}
				}
				else {
					for (size_t k = 0; k < treeData2.size(); ++k) {
						const auto& tree = treeData2[k];

						float offsetX = (l - segmentsX / 2) * lawnLength;
						glm::mat4 treeMatrix = glm::translate(glm::mat4(1.0f), tree.first + glm::vec3(offsetX, 0.f, (currentLawnSegment + i) * lawnLength));
						treeMatrix = glm::scale(treeMatrix, glm::vec3(treeScales[k])); // Apply scale factor

						// Set the model matrix for the tree
						lightingWithTextureShader.setMat4("model", treeMatrix);

						// Choose the tree model and texture
						if (tree.second == 0) {
							tree1ObjModel.Draw(lightingWithTextureShader);
							Texture treeTexture = tree1ObjModel.textures_loaded[0];
							glBindTexture(GL_TEXTURE_2D, treeTexture.id);
						}
						else {
							tree2ObjModel.Draw(lightingWithTextureShader);
							Texture treeTexture = tree2ObjModel.textures_loaded[0];
							glBindTexture(GL_TEXTURE_2D, treeTexture.id);
						}
					}

					// Train station rendering (accessing sideways segments)
					glm::mat4 trainStationModelMatrix = glm::mat4(1.0f);
					// Adjust the train station's X position using `l`
					trainStationModelMatrix = glm::translate(trainStationModelMatrix, glm::vec3(-11.0f, 0.40f, (currentLawnSegment + i) * lawnLength));
					trainStationModelMatrix = glm::scale(trainStationModelMatrix, glm::vec3(2.0f));
					trainStationModelMatrix = glm::rotate(trainStationModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					lightingWithTextureShader.setMat4("model", trainStationModelMatrix);

					// Determine which train station model to use
					int stationIndex = ((currentLawnSegment + i) / 4) % trainStationNames.size();
					trainStationObjModels[stationIndex].Draw(lightingWithTextureShader);
				}
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