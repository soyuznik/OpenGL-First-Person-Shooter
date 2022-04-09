#include "WINDOW.h"


WINDOW::WINDOW(int width, int height, std::string name) {
	windowSizeW = width;
	windowSizeH = height;
	window = glfwCreateWindow(width , height, name.c_str(), NULL, NULL);
}