#pragma once
// including glad used for shaders etc to configure them
#include <glad/glad.h>
// iostream idk for strings maybe or "cout"
//glm for mat4... etc
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// cout , I/O
#include <iostream>
// glfw for context which openGL need to draw in like a canvas
#include <GLFW/glfw3.h>
#include <fstream> // I/O
#include <sstream> // stringstream for text splitting
#include <vector>
#include "SHADER.h"


#define HALF_SIZE 0.4f
class WINDOW {
public:
	
	// window size
	int windowSizeW = 640, windowSizeH = 480;
	GLFWwindow* window;
	WINDOW(int width, int height, std::string name);
		//SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: Terrain CPU", NULL, NULL


};
