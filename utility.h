#pragma once
// Including glad to setup OpenGL , GLFW to create the context (window) , and Stb_image to read image/texture files
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

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

// utility function for loading a 2D texture from file
// ---------------------------------------------------
inline unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

bool ShootingFinished;
class Sound {
public:
    //load sounds
    FMOD_RESULT result;
    FMOD::System* fmodsystem;
    Sound();
    FMOD::Sound* createSound(std::string path);
    FMOD::ChannelGroup* __main = nullptr;
    void update();
    void stop();
    void play(FMOD::Sound* sound);

};
void Thread__play(FMOD::ChannelGroup* channelGroup, FMOD::Sound* sound, FMOD::System* system) {
    FMOD::Channel* channel;
    system->playSound(sound, channelGroup, false, &channel);
    Sleep(110);
    ShootingFinished = true;
}
void Sound::play(FMOD::Sound* sound) {
    std::thread ShootingPlayback(&Thread__play, __main, sound, fmodsystem);
    ShootingFinished = false;
    ShootingPlayback.detach();
}
void Sound::stop() {
    __main->stop();
}
void Sound::update() {
    fmodsystem->update();
}
Sound::Sound() {
    fmodsystem->createChannelGroup("General", &__main);
    result = FMOD::System_Create(&fmodsystem);		// Create the main system object.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    result = fmodsystem->init(100, FMOD_INIT_NORMAL, 0);	// Initialize FMOD.

    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

}
FMOD::Sound* Sound::createSound(std::string path) {
    // create FMOD sounds
    FMOD::Sound* sound;
    fmodsystem->createSound(path.c_str(), FMOD_DEFAULT, FMOD_DEFAULT, &sound);
    return sound;
}