

// GLM is a library for Opengl math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Some Code samples from learnopengl.com , modified by me.
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

// iostream for utility like std::cout
#include <iostream>

// FMOD is a C++ sound playing library (www.fmod.com)
#include <FMOD/fmod.h>
#include <FMOD/fmod_studio.hpp>
#include <FMOD/fmod_errors.h>

// random for random gun movement , etc.
#include <random>
// std::thread for sound playing , we need it async
#include <thread>

#include "utility.h"




// some namespace using to write things easier
using glm::vec3;
using glm::vec4;
using glm::vec2;
using std::min;
using std::max;

/*
		AABB collision & others collision mechanics inspired from
			 Game Programming in C++: Creating 3D Games
					  Book by Sanjay Madhav

*/
//////////////////////////////// Square Collision ////////////////////////////////////////////
class AABB {
public:
	AABB(const vec3& min, const vec3& max);
	vec3 mMin;
	vec3 mMax;
	void UpdateMinMax(vec3& point);
	bool Contains(vec3& point) const;
	float MinDistSq(vec3& point) const;
};
void AABB::UpdateMinMax(vec3& point)
{
	// Update each component separately
	mMin.x = min(mMin.x, point.x);
	mMin.y = min(mMin.y, point.y);
	mMin.z = min(mMin.z, point.z);
	mMax.x = max(mMax.x, point.x);
	mMax.y = max(mMax.y, point.y);
	mMax.z = max(mMax.z, point.z);
}
AABB::AABB(const vec3& min, const vec3& max)
	: mMin(min)
	, mMax(max)
{
}
bool AABB::Contains(vec3& point) const
{
	bool outside = point.x < mMin.x ||
		point.y < mMin.y ||
		point.z < mMin.z ||
		point.x > mMax.x ||
		point.y > mMax.y ||
		point.z > mMax.z;
	// If none of these are true, the point is inside the box
	return !outside;
}
float AABB::MinDistSq(vec3& point) const
{
	// Compute differences for each axis
	float dx = max(mMin.x - point.x, 0.0f);
	dx = max(dx, point.x - mMax.x);
	float dy = max(mMin.y - point.y, 0.0f);
	dy = max(dy, point.y - mMax.y);
	float dz = max(mMin.z - point.z, 0.0f);
	dz = max(dz, point.z - mMax.z);
	// Distance squared formula
	return dx * dx + dy * dy + dz * dz;
}

/////////////////////////////////// Sphere Collision ///////////////////////////////////////////////

class Sphere {
public:
	Sphere(const vec3& center, float radius);
	vec3 mCenter;
	float mRadius;
	bool Contains(vec3& point) const;
};
Sphere::Sphere(const vec3& center, float radius)
	:mCenter(center)
	, mRadius(radius)
{
}
bool Sphere::Contains(vec3& point) const
{
	// Get distance squared between center and point
	float distance = glm::length(mCenter - point);
	return (distance * distance) <= (mRadius * mRadius);
}

////////////////////////////////////// General Collision ////////////////////////////////
bool Intersect(Sphere& s, AABB& box)
{
	float distSq = box.MinDistSq(s.mCenter);
	return distSq <= (s.mRadius * s.mRadius);
}

////////////////////////////// Utility ///////////////////////////////////////

// generate random number with range
float s_rand(float _min, float _max)
{
	std::uniform_real_distribution<float> randomNum(_min, _max);
	return randomNum(std::mt19937(time(NULL)));
}

///////////////////////////// Sound /////////////////////////////////////////

bool ShootingFinished = true;
/// <summary>
///  Play sound
/// </summary>
/// <param name="channelGroup"> just the channel group</param>
/// <param name="sound"> the FMOD::Sound u want to play</param>
/// <param name="system"> the FMOD::System</param>
void playShooting(FMOD::ChannelGroup* channelGroup, FMOD::Sound* sound, FMOD::System* system) {
	FMOD::Channel* channel;
	system->playSound(sound, channelGroup, false, &channel);
	Sleep(110);
	ShootingFinished = true;
}

/// <summary>
/// function to update the ViewPort based on the Window size
/// </summary>
/// <param name="window"></param>
/// <param name="width"></param>
/// <param name="height"></param>
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//------------------------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// Booleans , mostly for gun management & others
bool shoot_cooldown = true;
bool Shooting = false;
bool ADS = false;
bool should_jump = false;
bool display_gun = true;
bool display_deagle = false;

int main()
{
#pragma region init
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// infinite frames
	glfwSwapInterval(0);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
#pragma endregion Setting OpenGL State
#pragma region Defining_data
	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("resources/shaders/7.4.camera_vertex.glsl", "resources/shaders/7.4.camera_frag.glsl");
	Shader modelShader("resources/shaders/1.model_vertex.glsl", "resources/shaders/1.model_frag.glsl");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	// AABB collision
	std::vector<vec3> points;
	for (unsigned int a = 0; a < sizeof(vertices) / sizeof(vertices[0]); a = a + 5) {
		points.push_back(vec3(vertices[a], vertices[a + 1], vertices[a + 2]));
	}
	AABB box(points[0], points[0]);
	for (size_t i = 1; i < points.size(); i++)
	{
		box.UpdateMinMax(points[i]);
	}
	//create box
	unsigned int VBO, boxVAO;
	glGenVertexArrays(1, &boxVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(boxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//create gun
	Model gun("resources/m4a1.obj" , "resources/gray.png");
	Model deagle("resources/deagle.obj" , "resources/gray.png");


	//create building
	Model cottage("resources/models/cottage.obj" , "resources/models/cottage.png");
	Model church("resources/models/untitled.obj", "resources/models/church.jpg");


	//create plane
	float planeVertices[] = {
		// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
		// 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
	   // -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
	   // -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

	   //  5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
	   // -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
	   //  5.0f, -0.5f, -5.0f,  2.0f, 2.0f

		-5.5f, -0.1f, -5.5f,  0.0f, 0.0f,
		 5.5f, -0.1f, -5.5f,  1.0f, 0.0f,
		 5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
		 5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
		-5.5f,  0.0f, -5.5f,  0.0f, 1.0f,
		-5.5f, -0.2f, -5.5f,  0.0f, 0.0f,

		-5.5f, -0.1f,  5.5f,  0.0f, 0.0f,
		 5.5f, -0.1f,  5.5f,  1.0f, 0.0f,
		 5.5f,  0.0f,  5.5f,  1.0f, 1.0f,
		 5.5f,  0.0f,  5.5f,  1.0f, 1.0f,
		-5.5f,  0.0f,  5.5f,  0.0f, 1.0f,
		-5.5f, -0.1f,  5.5f,  0.0f, 0.0f,

		-5.5f,  0.0f,  5.5f,  1.0f, 0.0f,
		-5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
		-5.5f, -0.1f, -5.5f,  0.0f, 1.0f,
		-5.5f, -0.1f, -5.5f,  0.0f, 1.0f,
		-5.5f, -0.1f,  5.5f,  0.0f, 0.0f,
		-5.5f,  0.0f,  5.5f,  1.0f, 0.0f,

		 5.5f,  0.0f,  5.5f,  1.0f, 0.0f,
		 5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
		 5.5f, -0.1f, -5.5f,  0.0f, 1.0f,
		 5.5f, -0.1f, -5.5f,  0.0f, 1.0f,
		 5.5f, -0.1f,  5.5f,  0.0f, 0.0f,
		 5.5f,  0.0f,  5.5f,  1.0f, 0.0f,

		-5.5f, -0.1f, -5.5f,  0.0f, 1.0f,
		 5.5f, -0.1f, -5.5f,  1.0f, 1.0f,
		 5.5f, -0.1f,  5.5f,  1.0f, 0.0f,
		 5.5f, -0.1f,  5.5f,  1.0f, 0.0f,
		-5.5f, -0.1f,  5.5f,  0.0f, 0.0f,
		-5.5f, -0.1f, -5.5f,  0.0f, 1.0f,

		-5.5f,  0.0f, -5.5f,  0.0f, 1.0f,
		 5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
		 5.5f,  0.0f,  5.5f,  1.0f, 0.0f,
		 5.5f,  0.0f,  5.5f,  1.0f, 0.0f,
		-5.5f,  0.0f,  5.5f,  0.0f, 0.0f,
		-5.5f,  0.0f, -5.5f,  0.0f, 1.0f
	};




	// AABB collision
	std::vector<vec3> ppoints;
	for (unsigned int a = 0; a < sizeof(planeVertices) / sizeof(planeVertices[0]); a = a + 5) {
		ppoints.push_back(vec3(planeVertices[a], planeVertices[a + 1], planeVertices[a + 2]));
	}
	// create a plane with 36x36 smaller planes
	// update the Min/Max points accordingly
	AABB plane(ppoints[0], ppoints[0]);
	for (size_t i = 1; i < ppoints.size(); i++)
	{
		for (int i = 1; i < 36; i++) {
			for (int j = 1; j < 36; j++) {
				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(11.0f * i, -0.51f, 11.0f * j));
				glm::vec4 point = (model * glm::vec4(ppoints[i].x, ppoints[i].y, ppoints[i].z, 1.0f));
				plane.UpdateMinMax(glm::vec3(point.x, point.y, point.z));
			}
		}
	}

	//create box
	unsigned int VBO1, planeVAO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &VBO1);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	//square to draw shoot effect
	float sqVertices[] = {
		 -0.5f, -0.5f,     0.0f,    0.0f,  0.0f,
		  0.5f, -0.5f,     0.0f,    1.0f,  0.0f,
		  0.5f,  0.5f,     0.0f,    1.0f,  1.0f,
		  0.5f,  0.5f,     0.0f,    1.0f,  1.0f,
		 -0.5f,  0.5f,     0.0f,    0.0f,  1.0f,
		 -0.5f, -0.5f,     0.0f,    0.0f,  0.0
	};
	//create square
	unsigned int VBO2, sqVAO;
	glGenVertexArrays(1, &sqVAO);
	glGenBuffers(1, &VBO2);
	glBindVertexArray(sqVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sqVertices), sqVertices, GL_STATIC_DRAW);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);







	//load gun texture
	unsigned int texture1 = loadTextureX("resources/gray.png");
	//load box texture
	unsigned int texture2 = loadTextureX("resources/textures/container.jpg");
	//load plane texture
	unsigned int texture3 = loadTextureX("resources/textures/brickwall.jpg");
	//load explosion texture
	unsigned int texture4 = loadTextureY("resources/shoot_eff_.png");
	//load crosshair texture
	unsigned int texture5 = loadTextureY("resources/crosshair.png");



	




	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use();
	ourShader.setInt("texture1", 0);
	ourShader.setInt("texture2", 1);

#pragma endregion Defining_data
#pragma region Load_Sounds
	//load sounds
	FMOD_RESULT result;
	FMOD::System* system;

	result = FMOD::System_Create(&system);		// Create the main system object.
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}

	result = system->init(100, FMOD_INIT_NORMAL, 0);	// Initialize FMOD.

	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
	// create FMOD sounds
	FMOD::Sound* m4_sound;
	result = system->createSound("resources/m4a1_ff.mp3", FMOD_DEFAULT, FMOD_DEFAULT, &m4_sound);		// FMOD_DEFAULT uses the defaults.  These are the same as FMOD_LOOP_OFF | FMOD_2D | FMOD_HARDWARE.

	FMOD::Sound* deag_sound;
	result = system->createSound("resources/desert_s.mp3", FMOD_DEFAULT, FMOD_DEFAULT, &deag_sound);

	FMOD::Sound* mainmusic;
	result = system->createSound("resources/music.mp3", FMOD_DEFAULT, FMOD_DEFAULT, &mainmusic);

	// Create the channel group. ( easier management )
	FMOD::ChannelGroup* channelGroup = nullptr;
	result = system->createChannelGroup("Shooting", &channelGroup);

	FMOD::ChannelGroup* musicGroup = nullptr;
	result = system->createChannelGroup("Music", &musicGroup);


	FMOD::Channel* mchannel;
	//system->playSound(mainmusic, musicGroup, false, &mchannel);
#pragma endregion FMOD

	// save Last Position where the camera didint collide anything
	vec3 saveLastPostion = camera.Position;

	// Automatic gun boolean , means if the gun should move Back or Forth>..XD
	bool should_move = true;
	// if gravity is enabled
	bool should_fall = true;
	// jump_cooldown so you cant fly;;;;
	float jump_cooldown = 0.0f;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if the camera should jump anymore...
		if (camera.should_jump && jump_cooldown > 1.0f) {
			jump_cooldown = 0.0f;
			camera.should_jump = false;
			camera.Position.y += 0.9f;
		}
		else {
			jump_cooldown += deltaTime;
		}

		// Create a sphere as the camera , because if it would be a point it would get a little bit inside objects
		Sphere cameraSphere(camera.Position, 0.2f);
		if (should_fall) { // gravity
			camera.Position = glm::vec3(camera.Position.x, camera.Position.y - 0.002f, camera.Position.z);
		}
		if (Intersect(cameraSphere, box)) { // check intersection with boxes
			camera.Position = saveLastPostion;
		}
		else if (Intersect(cameraSphere, plane)) { // check intersection with the plane
			camera.Position = saveLastPostion;
		}
		else saveLastPostion = camera.Position;  // if not colliding save possition when its safe

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // clear viewport with a blue color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear depth
		glEnable(GL_DEPTH_TEST); // enable depth testing

		// activate shader
		ourShader.use();
		
		
		//draw crosshair
		glm::mat4 projection = glm::mat4(1.0f);
		ourShader.setMat4("projection", projection);
		// camera/view transformation
		glm::mat4 view = glm::mat4(1.0f);
		ourShader.setMat4("view", view);

		// CAST RAY TO 0,0,0
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		ourShader.setMat4("model", model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture5);
		glBindVertexArray(sqVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);




		/*
		-
		 -          USE MODEL SHADER FOR RENDERING MODELS
		-
		*/

		modelShader.use();
		
		// pass projection matrix to shader (note that in this case it could change every frame)
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		modelShader.setMat4("projection", projection);

		// camera/view transformation
		view = camera.GetViewMatrix();
		modelShader.setMat4("view", view);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -0.0f, 20.0f));
		model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		modelShader.setMat4("model", model);
		cottage.Draw(modelShader);


		// pass projection matrix to shader (note that in this case it could change every frame)
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		modelShader.setMat4("projection", projection);

		// camera/view transformation
		view = camera.GetViewMatrix();
		modelShader.setMat4("view", view);

		//model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -0.5f, 20.0f));




		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 6.5f, 20.0f));
		model = glm::rotate(model, glm::radians(10.0f) , glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
		//rot = glm::rotate(rot, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		
		modelShader.setMat4("model", model);

		church.Draw(modelShader);


		// back to old shader

		ourShader.use();
		// pass projection matrix to shader (note that in this case it could change every frame)
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);

		// camera/view transformation
		view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -0.0f, 20.0f));
		model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		ourShader.setMat4("model", model);

		//draw plane 36 x 36 small planes
		for (int i = 1; i < 36; i++) {
			for (int j = 1; j < 36; j++) {
				model = glm::translate(glm::mat4(1.0f), glm::vec3(11.0f * i, -0.51f, 11.0f * j));
				ourShader.setMat4("model", model);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture3);
				glBindVertexArray(planeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		// update FMOD system
		system->update();

		//check if should apply effect.
		glClear(GL_DEPTH_BUFFER_BIT);
		
		if (Shooting && ADS && display_deagle) {
			glEnable(GL_BLEND);
			//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			// pass projection matrix to shader 
			glm::mat4 projection = glm::mat4(1.0f);
			ourShader.setMat4("projection", projection);
			// camera/view transformation
			glm::mat4 view = glm::mat4(1.0f);
			ourShader.setMat4("view", view);

			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			ourShader.setMat4("model", model);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture4);
			glBindVertexArray(sqVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisable(GL_BLEND);
		}
		if (Shooting && !ADS && display_deagle) {
			glEnable(GL_BLEND);
			//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			// pass projection matrix to shader 
			glm::mat4 projection = glm::mat4(1.0f);
			ourShader.setMat4("projection", projection);
			// camera/view transformation
			glm::mat4 view = glm::mat4(1.0f);
			ourShader.setMat4("view", view);

			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.241f, 0.3f, 0.4f));
			ourShader.setMat4("model", model);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture4);
			glBindVertexArray(sqVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisable(GL_BLEND);
		}




		//check which gun to dispaly
		if (display_gun) {
			//update state
			if (Shooting && ShootingFinished) {
				std::thread ShootingPlayback(&playShooting, channelGroup, m4_sound, system);
				ShootingFinished = false;
				ShootingPlayback.detach();
			}
			else if (!Shooting) {
				channelGroup->stop();
			}

			// draw fps gun
			// bind textures on corresponding texture units
			//glDisable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);

			// here just a bunch of view , projection & model transformation so the GunModel appears on screen...
			glm::mat4 gunView = glm::lookAt(glm::vec3{ 0 }, glm::vec3{ 0, 0, -1 }, glm::vec3{ 0, 1, 0 });
			gunView = glm::rotate(gunView, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::mat4(1.0f);

			// AUTO FIRE
			if (ADS) {// if ADS enabled move gun to the middle instead
				glm::mat4 gunView = glm::lookAt(glm::vec3{ 0 }, glm::vec3{ 0, 0, -1 }, glm::vec3{ 0, 1, 0 });
				gunView = glm::rotate(gunView, glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(2.0f, -2.5f, 0.0f));
			}
			// Automatic fire , move gun back and forth , randomly/.....
			if (Shooting && should_move) {
				model = glm::translate(model, glm::vec3(0.0f, 0.1f, s_rand(0.0f, 1.0f)));
				should_move = false;
			}
			else if (!should_move && Shooting) {
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.1f));
				should_move = true;
			}
			// set the matrices and draw the model...
			ourShader.setMat4("model", model);
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", gunView);
			gun.Draw(ourShader);
		}
		else if (display_deagle) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
			glClear(GL_DEPTH_BUFFER_BIT);

			//update state
			// the deagle has SINGLE fire so it should fire just when clicked
			if (Shooting && ShootingFinished && !shoot_cooldown) {
				shoot_cooldown = true;
				std::thread ShootingPlayback(&playShooting, channelGroup, deag_sound, system);
				ShootingFinished = false;
				ShootingPlayback.detach();
			}
			// move gun in the right corener
			glm::mat4 gunView = glm::lookAt(glm::vec3{ 0 }, glm::vec3{ 0, 0, -1 }, glm::vec3{ 0, 1, 0 });
			model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::translate(model, glm::vec3(-1.0f, 4.5f, 0.0f));
			// SINGLE FIRE
			if (ADS) {// if ADS move gun in the middle insead
				glm::mat4 gunView = glm::lookAt(glm::vec3{ 0 }, glm::vec3{ 0, 0, -1 }, glm::vec3{ 0, 1, 0 });
				model = glm::mat4(1.0f);
				model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::translate(model, glm::vec3(1.0f, 5.5f, 0.0f));
			}
			if (Shooting) { // if shooting move gun up ...
				model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(1.0f, 0.1f, 0.0f));
				should_move = false;
			}
			// Draw gun Model
			ourShader.setMat4("model", model);
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", gunView);
			deagle.Draw(ourShader);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	// if X is pressed move 5x faster
	double speed = 1.0;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		speed = 5;

	// if 1 has been pressesd display M4 insdead of deagle
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		display_gun = true;
		display_deagle = false;
	}
	// else if 2 dispaly deagle
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		display_gun = false;
		display_deagle = true;
	}
	// check mouse buttons LMB shoot and RMB -> ADS
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) { ADS = true; }
	else { ADS = false; }
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) { Shooting = true; }
	else {
		shoot_cooldown = false;
		Shooting = false;
	}

	// if escape has been pressed quit
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// if space pressed jump.
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		should_jump = true;

	//  CHECK for wasd MOVEMENT
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime * speed);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}