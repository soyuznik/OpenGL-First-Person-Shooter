#pragma once

#include "WINDOW.h"
#include "stb_image.h"
class TEXTURE
{
public:
	unsigned int ID;
	TEXTURE(std::string path);
	void use();
};
