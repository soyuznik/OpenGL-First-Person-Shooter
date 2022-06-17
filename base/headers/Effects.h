#pragma once
#include "Texture.h"
#include "GLEW/glew.h"
#include <string>
#include <vector>
#include <time.h>
class Effect {
public:
	unsigned int sqVAO;
	int current_index = -1;
	time_t current_time;
	std::vector<Texture*> eff;
	bool should_start = false;
	Effect(std::vector<std::string> paths);
	void start() { should_start = true; };
	void render();
};