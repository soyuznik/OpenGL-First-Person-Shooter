


#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader_t.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>
#include <iterator>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int NUM_PATCH_PTS = 4;







int VERTEX_COUNT;
// camera - give pretty starting point
Camera camera(glm::vec3(67.0f, 627.5f, 169.9f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -128.1f, -42.4f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
/*
float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {
    float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
    float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
    float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
    float l3 = 1.0f - l1 - l2;
    return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}

float GetCurrentHeight(std::vector<float>* vertices , float width , float height) {
     
    
   

    float gridWidth = width / VERTEX_COUNT;
    float gridHeight = height / VERTEX_COUNT;
    //   camera.Position ==> x,y,z
    int gridX = camera.Position.x / ((float)sizeof(heights) / sizeof(heights[0]));
    int gridZ = camera.Position.x / ((float)sizeof(heights) / sizeof(heights[0]));

    if (gridX > (gridWidth - 1) or gridZ > (gridHeight - 1) or gridX < 0 or gridZ < 0) {
        gridX = 0; gridZ = 0;
    }

    float xCoord = (camera.Position.x / gridWidth) / gridWidth;
    float zCoord = (camera.Position.y / gridHeight) / gridHeight;
    float answer;
    if (xCoord <= (1 - zCoord)) {
        answer = barryCentric(glm::vec3(0, heights[gridX][gridZ], 0), glm::vec3(1,
            heights[gridX + 1][gridZ], 0), glm::vec3(0,
                heights[gridX][gridZ + 1], 1), glm::vec2(xCoord, zCoord));
    }
    else {
        answer = barryCentric(glm::vec3(1, heights[gridX + 1][gridZ], 0), glm::vec3(1,
            heights[gridX + 1][gridZ + 1], 1), glm::vec3(0,
                heights[gridX][gridZ + 1], 1), glm::vec2(xCoord, zCoord));
    }
    //deallocate the array
    for (int i = 0; i < VERTEX_COUNT; i++)
        delete[] heights[i];
    delete[] heights;
    return answer;

}
*/

glm::vec4 __GetPixel(stbi_uc* image, size_t imageWidth, size_t x, size_t y) {
    const stbi_uc* p = image + (4 * (y * imageWidth + x));
    int r = p[0];
    int g = p[1];
    int b = p[2];
    int a = p[3];
    return glm::vec4(r, g, b, a);
}



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: Terrain GPU", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
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

GLint maxTessLevel;
glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
std::cout << "Max available tess level: " << maxTessLevel << std::endl;

// configure global opengl state
// -----------------------------
glEnable(GL_DEPTH_TEST);

// build and compile our shader program
// ------------------------------------
__gl::Shader tessHeightMapShader("resources/shaders/gpuheight_vertex.glsl",
    "resources/shaders/gpuheight_frag.glsl", nullptr,            // if wishing to render as is
    "resources/shaders/gpuheight_tcs.glsl",
    "resources/shaders/gpuheight_tes.glsl");

// load and create a texture
// -------------------------
unsigned int texture;
glGenTextures(1, &texture);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
// set the texture wrapping parameters
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// set texture filtering parameters
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// load image, create texture and generate mipmaps
int width, height, nrChannels;
// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
unsigned char* data = stbi_load("resources/heightmaps/iceland_heightmap.png", &width, &height, &nrChannels, 4);
if (data)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    tessHeightMapShader.setInt("heightMap", 0);
    std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
}
else
{
    std::cout << "Failed to load texture" << std::endl;
}


// set up vertex data (and buffer(s)) and configure vertex attributes
// ------------------------------------------------------------------
std::vector<float> vertices;

unsigned rez = 20;
//allocate the array

for (unsigned i = 0; i <= rez - 1; i++)
{
    for (unsigned j = 0; j <= rez - 1; j++)
    {
        vertices.push_back(-width / 2.0f + width * i / (float)rez); // v.x
        vertices.push_back(0.0f); // v.y
        vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
        vertices.push_back(i / (float)rez); // u
        vertices.push_back(j / (float)rez); // v

        vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez); // v.x
        vertices.push_back(0.0f); // v.y
        vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
        vertices.push_back((i + 1) / (float)rez); // u
        vertices.push_back(j / (float)rez); // v

        vertices.push_back(-width / 2.0f + width * i / (float)rez); // v.x
        vertices.push_back(0.0f); // v.y
        vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
        vertices.push_back(i / (float)rez); // u
        vertices.push_back((j + 1) / (float)rez); // v

        vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez); // v.x
        vertices.push_back(0.0f); // v.y
        vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
        vertices.push_back((i + 1) / (float)rez); // u
        vertices.push_back((j + 1) / (float)rez); // v


        
    }
}
    VERTEX_COUNT =  vertices.size() / 5;
    std::cout << "Loaded " << rez * rez << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << rez * rez * 4 << " vertices in vertex shader" << std::endl;

    // first, configure the cube's VAO (and terrainVBO)
    unsigned int terrainVAO, terrainVBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);







    /////////////////////////////////////////////////////////////////////////////////////////
    // lighting
    glm::vec3 lightPos(67.0f, 300.5f, 169.9f);


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    __gl::Shader lightingShader("resources/shaders/basic_light_vertex.glsl", "resources/shaders/basic_light_frag.glsl");
    __gl::Shader lightCubeShader("resources/shaders/cube_light_vertex.glsl", "resources/shaders/cube_light_frag.glsl");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float fvertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fvertices), fvertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);















    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
       // float crtHeight = GetCurrentHeight(&vertices, width, height);
     //   if (camera.Position.y < crtHeight) {
    //        camera.Position.y = crtHeight;
    //    }
        glm::vec4 RGBA = __GetPixel(data, width, 627, 627);
        std::cout << std::to_string(RGBA.r) << std::endl;
        std::cout << std::to_string(RGBA.g) << std::endl;
        std::cout << std::to_string(RGBA.b) << std::endl;
        std::cout << std::to_string(RGBA.a) << std::endl << std::endl;
        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // render the cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // also draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // be sure to activate shader when setting uniforms/drawing objects
        tessHeightMapShader.use();

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
        view = camera.GetViewMatrix();
        tessHeightMapShader.setMat4("projection", projection);
        tessHeightMapShader.setMat4("view", view);

        // world transformation
        model = glm::mat4(1.0f);
        tessHeightMapShader.setMat4("model", model);

        // render the terrain
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS * rez * rez);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    stbi_image_free(data);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    static int press = 1;
    double speed = 1;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        press = 1 - press;
        if (press == 0) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (press == 1) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        speed = 20;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

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

// glfw: whenever a key event occurs, this callback is called
// ---------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        default:
            break;
        }
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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
    camera.ProcessMouseScroll(yoffset);
}