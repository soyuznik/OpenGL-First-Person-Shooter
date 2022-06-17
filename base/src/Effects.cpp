#include "Effects.h"


void Effect::render() {
	if ((time(NULL) - current_time) > 0.5f && should_start) {
		current_time = time(NULL);
		current_index++;
		glBindVertexArray(sqVAO);
		glBindTexture(GL_TEXTURE_2D , eff[current_index]->mTextureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	
}
Effect::Effect(std::vector<std::string> paths) {
	current_time = time(NULL);
	float sqVertices[] = {
		 -0.5f, -0.5f,     0.0f,    0.0f,  0.0f,
		  0.5f, -0.5f,     0.0f,    1.0f,  0.0f,
		  0.5f,  0.5f,     0.0f,    1.0f,  1.0f,
		  0.5f,  0.5f,     0.0f,    1.0f,  1.0f,
		 -0.5f,  0.5f,     0.0f,    0.0f,  1.0f,
		 -0.5f, -0.5f,     0.0f,    0.0f,  0.0
	};
	//create square
	unsigned int VBO2;
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

	for (int i = 0; i < paths.size(); i++) {
		Texture* texture = new Texture();
		texture->Load(paths[i]);
		eff.push_back(texture);
	}
}