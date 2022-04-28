#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>



#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <algorithm>

#include <fstream>
#include <sstream>

#include <FMOD/fmod.h>
#include <FMOD/fmod_studio.hpp>
#include <FMOD/fmod_errors.h>

#include <random>
#include <thread>
using glm::vec3;
using glm::vec4;
using glm::vec2;
using std::min;
using std::max;

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
bool Intersect(Sphere& s,AABB& box)
{
    float distSq = box.MinDistSq(s.mCenter);
    return distSq <= (s.mRadius * s.mRadius);
}

////////////////////////////// Utility ///////////////////////////////////////
float s_rand(float _min, float _max)
{
    std::uniform_real_distribution<float> randomNum(_min, _max);
    return randomNum(std::mt19937(time(NULL)));
}

///////////////////////////// Sound /////////////////////////////////////////

bool ShootingFinished = true;
void playShooting(FMOD::ChannelGroup* channelGroup , FMOD::Sound* sound , FMOD::System* system) {
    Sleep(110);
    FMOD::Channel* channel;
    system->playSound(sound, channelGroup, false, &channel);
    ShootingFinished = true;
}




















void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


bool Shooting = false;
bool ADS = false;



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
    Shader ourShader("resources/shaders/7.4.camera.vs", "resources/shaders/7.4.camera.fs");

    
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
    Model gun("resources/m4a1.obj");
    
    //create plane
    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
        // 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
       // -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
       // -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

       //  5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
       // -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
       //  5.0f, -0.5f, -5.0f,  2.0f, 2.0f


        -5.5f, -0.2f, -5.5f,  0.0f, 0.0f,
         5.5f, -0.2f, -5.5f,  1.0f, 0.0f,
         5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
         5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
        -5.5f,  0.0f, -5.5f,  0.0f, 1.0f,
        -5.5f, -0.2f, -5.5f,  0.0f, 0.0f,

        -5.5f, -0.2f,  5.5f,  0.0f, 0.0f,
         5.5f, -0.2f,  5.5f,  1.0f, 0.0f,
         5.5f,  0.0f,  5.5f,  1.0f, 1.0f,
         5.5f,  0.0f,  5.5f,  1.0f, 1.0f,
        -5.5f,  0.0f,  5.5f,  0.0f, 1.0f,
        -5.5f, -0.2f,  5.5f,  0.0f, 0.0f,

        -5.5f,  0.0f,  5.5f,  1.0f, 0.0f,
        -5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
        -5.5f, -0.2f, -5.5f,  0.0f, 1.0f,
        -5.5f, -0.2f, -5.5f,  0.0f, 1.0f,
        -5.5f, -0.2f,  5.5f,  0.0f, 0.0f,
        -5.5f,  0.0f,  5.5f,  1.0f, 0.0f,

         5.5f,  0.0f,  5.5f,  1.0f, 0.0f,
         5.5f,  0.0f, -5.5f,  1.0f, 1.0f,
         5.5f, -0.2f, -5.5f,  0.0f, 1.0f,
         5.5f, -0.2f, -5.5f,  0.0f, 1.0f,
         5.5f, -0.2f,  5.5f,  0.0f, 0.0f,
         5.5f,  0.0f,  5.5f,  1.0f, 0.0f,

        -5.5f, -0.2f, -5.5f,  0.0f, 1.0f,
         5.5f, -0.2f, -5.5f,  1.0f, 1.0f,
         5.5f, -0.2f,  5.5f,  1.0f, 0.0f,
         5.5f, -0.2f,  5.5f,  1.0f, 0.0f,
        -5.5f, -0.2f,  5.5f,  0.0f, 0.0f,
        -5.5f, -0.2f, -5.5f,  0.0f, 1.0f,

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
    AABB plane(ppoints[0], ppoints[0]);
    for (size_t i = 1; i < ppoints.size(); i++)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.51f, 0.0f));
        glm::vec4 point = (model * glm::vec4(ppoints[i].x, ppoints[i].y, ppoints[i].z, 1.0f));
        plane.UpdateMinMax(glm::vec3(point.x , point.y , point.z));

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






    //load gun texture
    unsigned int texture1;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("resources/gray.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);



    //load box texture
    unsigned int texture2;
    // texture 1
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data_ = stbi_load("resources/textures/container.jpg", &width, &height, &nrChannels, 0);
    if (data_)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_);


    //load box texture
    unsigned int texture3;
    // texture 1
    // ---------
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data_1 = stbi_load("resources/textures/brickwall.jpg", &width, &height, &nrChannels, 0);
    if (data_1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_1);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data_1);
   
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

    result = system->init(100 , FMOD_INIT_NORMAL, 0);	// Initialize FMOD.

    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }

    FMOD::Sound* sound;
    result = system->createSound("resources/m4a1_ff.mp3", FMOD_DEFAULT, FMOD_DEFAULT, &sound);		// FMOD_DEFAULT uses the defaults.  These are the same as FMOD_LOOP_OFF | FMOD_2D | FMOD_HARDWARE.
    
    // Create the channel group.
    FMOD::ChannelGroup* channelGroup = nullptr;
    result = system->createChannelGroup("Shooting", &channelGroup);
#pragma endregion FMOD
   
    

    vec3 saveLastPostion = camera.Position;
    bool should_move = true;
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
        Sphere cameraSphere(camera.Position, 0.2f);
        
        if (Intersect(cameraSphere , box)) {
           camera.Position = saveLastPostion;

        }
        else if (Intersect(cameraSphere, plane)) {
            camera.Position = saveLastPostion;
        }
        else saveLastPostion = camera.Position;
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
       

        // activate shader
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);

        //draw box 

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glBindVertexArray(boxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        //draw plane
        model = glm::translate(glm::mat4(1.0f) , glm::vec3(0.0f , -0.51f , 0.0f));
        ourShader.setMat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture3);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //update state
        if (Shooting && ShootingFinished) {
            std::thread ShootingPlayback(&playShooting, channelGroup, sound, system);
            ShootingFinished = false;
            ShootingPlayback.detach();
        }
        else if (!Shooting) {
            channelGroup->stop();
            
        }
        system->update();


        // draw fps gun
        // bind textures on corresponding texture units
        //glDisable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glm::mat4 gunView = glm::lookAt(glm::vec3{ 0 }, glm::vec3{ 0, 0, -1 }, glm::vec3{ 0, 1, 0 });
        gunView = glm::rotate(gunView, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::mat4(1.0f);
        if (Shooting && should_move) {
            model = glm::translate(model, glm::vec3(0.0f, 0.1f, s_rand(0.0f, 1.0f)));
            should_move = false;
        }
        else if (!should_move && Shooting) {
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.1f));
            should_move = true;
        }
        ourShader.setMat4("model", model);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", gunView);
        gun.Draw(ourShader);
       






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
    static bool should_play = true;
    double speed = 1.0;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        speed = 5;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
        Shooting = true;
    }
    else {
        Shooting = false;
    }


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

