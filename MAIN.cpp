



#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <lowlevel\WINDOW.h>
#include <learnopengl/camera.h>
#include <iostream>
#include <vector>


#include <highlevel/Panel.h>
#include <highlevel/Slider.h>
#include <lowlevel/SHADER.h>
///own made //////////////////////////

#include <lowlevel/TEXTURE.h>
#include <learnopengl/shader_m.h>
#include <lowlevel/UTILITY.h>
//#define FRAMEBUFFER_MENU
#define FRAMEBUFFER_DRAW
#define __VAO



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 600;
int useWireframe = 0;
int displayGrayscale = 0;
int displayHalfMode = 0;



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

int main()
{
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
    WINDOW* _windowobj = new WINDOW(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: Terrain CPU");

    if (_windowobj->window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(_windowobj->window);
    glfwSetFramebufferSizeCallback(_windowobj->window, framebuffer_size_callback);
    glfwSetKeyCallback(_windowobj->window, key_callback);
    glfwSetCursorPosCallback(_windowobj->window, mouse_callback);
    glfwSetScrollCallback(_windowobj->window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(_windowobj->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state and create objects
    // -----------------------------
   // glEnable(GL_DEPTH_TEST);
   /// glViewport(0, 0, 800, 800);

    // build and compile our shader program
    // ------------------------------------
  
    Shader* texture_shader = new Shader("resources/shaders/texture_vertex.glsl", "resources/shaders/texture_frag.glsl");
    texture_shader->NORMALIZE_VALUES();
    Slider* slider = new Slider(texture_shader, _windowobj, "resources/textures/gray.png", 1025, 550, 0.1f);
    __gl::Shader screenShader("resources/shaders/framebuffer_screen_vertex.glsl",
                          "resources/shaders/framebuffer_screen_frag.glsl");
    __gl::Shader heightMapShader("resources/shaders/height_mapper_vertex.glsl",
        "resources/shaders/height_mapper_frag.glsl");



    heightMapShader.use();
    heightMapShader.setInt("texture1", 0);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);



    // load and create a texture
    // -------------------------
    // load image, create texture and generate mipmaps
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("resources/heightmaps/iceland_heightmap.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<float> vertices;
    float yScale = 64.0f / 256.0f, yShift = 16.0f;
    int rez = 1;
    unsigned bytePerPixel = nrChannels;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
            unsigned char y = pixelOffset[0];

            // vertex
            vertices.push_back(-height / 2.0f + height * i / (float)height);   // vx
            vertices.push_back((int)y * yScale - yShift);   // vy
            vertices.push_back(-width / 2.0f + width * j / (float)width);   // vz
        }
    }
    std::cout << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
    stbi_image_free(data);

    std::vector<unsigned> indices;
    for (unsigned i = 0; i < height - 1; i += rez)
    {
        for (unsigned j = 0; j < width; j += rez)
        {
            for (unsigned k = 0; k < 2; k++)
            {
                indices.push_back(j + width * (i + k * rez));
            }
        }
    }
    std::cout << "Loaded " << indices.size() << " indices" << std::endl;

    const int numStrips = (height - 1) / rez;
    const int numTrisPerStrip = (width / rez) * 2 - 2;
    std::cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << std::endl;
    std::cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << std::endl;

    // first, configure the cube's VAO (and terrainVBO + terrainIBO)
    unsigned int terrainVAO, terrainVBO, terrainIBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrainIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);




    float hfquadVertices[] = { // vertex attributes for a quad that fills the half screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         HALF_SIZE, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         HALF_SIZE, -1.0f,  1.0f, 0.0f,
         HALF_SIZE,  1.0f,  1.0f, 1.0f
    };
    float ohfquadVertices[] = { // vertex attributes for a quad that fills the other  half screen in Normalized Device Coordinates.
        // positions   // texCoords
         HALF_SIZE,  1.0f,  0.0f, 1.0f,
         HALF_SIZE, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

         HALF_SIZE,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    float fquadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };




#ifdef __VAO
    unsigned int ohfquadVAO, ohfquadVBO;
    glGenVertexArrays(1, &ohfquadVAO);
    glGenBuffers(1, &ohfquadVBO);
    glBindVertexArray(ohfquadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ohfquadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ohfquadVertices), &ohfquadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    unsigned int hfquadVAO , hfquadVBO;
    glGenVertexArrays(1, &hfquadVAO);
    glGenBuffers(1, &hfquadVBO);
    glBindVertexArray(hfquadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hfquadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hfquadVertices), &hfquadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));




    unsigned int fquadVAO, fquadVBO;
    glGenVertexArrays(1, &fquadVAO);
    glGenBuffers(1, &fquadVBO);
    glBindVertexArray(fquadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fquadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fquadVertices), &fquadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
#endif
#ifdef FRAMEBUFFER_DRAW
    // framebuffer draw context configuration
    // -------------------------
    unsigned int dframebuffer;
    glGenFramebuffers(1, &dframebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, dframebuffer);
    // create a color attachment texture
    unsigned int dtextureColorbuffer;
    glGenTextures(1, &dtextureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, dtextureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dtextureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int drbo;
    glGenRenderbuffers(1, &drbo);
    glBindRenderbuffer(GL_RENDERBUFFER, drbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, drbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#endif
#ifdef FRAMEBUFFER_MENU
    // framebuffer menu  configuration
    // -------------------------
    unsigned int mframebuffer;
    glGenFramebuffers(1, &mframebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mframebuffer);
    // create a color attachment texture
    unsigned int mtextureColorbuffer;
    glGenTextures(1, &mtextureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, mtextureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mtextureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int mrbo;
    glGenRenderbuffers(1, &mrbo);
    glBindRenderbuffer(GL_RENDERBUFFER, mrbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mrbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif




    // render loop
    // -----------
    while (!glfwWindowShouldClose(_windowobj->window))
    {
        if (useWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //        std::cout << deltaTime << "ms (" << 1.0f / deltaTime << " FPS)" << std::endl;

                // input
                // -----
        processInput(_windowobj->window);

#ifdef FRAMEBUFFER_DRAW
        glBindFramebuffer(GL_FRAMEBUFFER, dframebuffer);
        glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        // be sure to activate shader when setting uniforms/drawing objects
        heightMapShader.use();
        glm::mat4 projection;
        // view/projection transformations
        if (displayHalfMode) {
           projection = glm::perspective(glm::radians(camera.Zoom), (float)(SCR_WIDTH * 1.435) / (float)SCR_HEIGHT, 0.1f, 100000.0f);
        }
        else projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        heightMapShader.setMat4("projection", projection);
        heightMapShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        heightMapShader.setMat4("model", model);

        // render the cube
        
        glBindVertexArray(terrainVAO);
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for (unsigned strip = 0; strip < numStrips; strip++)
        {
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                numTrisPerStrip + 2,   // number of indices to render
                GL_UNSIGNED_INT,     // index data type
                (void*)(sizeof(unsigned) * (numTrisPerStrip + 2) * strip)); // offset to starting index
        }
        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);
#endif
#ifdef FRAMEBUFFER_MENU

        glBindFramebuffer(GL_FRAMEBUFFER, mframebuffer);
        glDisable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        slider->render();

        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);
#endif



       
        if (displayHalfMode) { 
            glfwSetWindowSize(_windowobj->window, SCR_WIDTH * 1.435, SCR_HEIGHT);
            glfwSetInputMode(_windowobj->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#ifdef FRAMEBUFFER_MENU
            screenShader.use();
            glBindVertexArray(ohfquadVAO);
            glDisable(GL_DEPTH_TEST);
            glBindTexture(GL_TEXTURE_2D, mtextureColorbuffer);
            glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
            //render menu /////////////////////////////////////////
            glBindVertexArray(hfquadVAO);
        }
        else { 
            glfwSetWindowSize(_windowobj->window, SCR_WIDTH, SCR_HEIGHT);
            glBindVertexArray(fquadVAO); 
            glfwSetInputMode(_windowobj->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        screenShader.use();
#ifdef FRAMEBUFFER_DRAW
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, dtextureColorbuffer);
#endif




        glDrawArrays(GL_TRIANGLES, 0, 6);
        if (displayHalfMode) {
            slider->render();
            slider->accept_input(return_ndc_cursor(_windowobj->window));
        }


        



     


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(_windowobj->window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteBuffers(1, &terrainIBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    double speed = 1;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        speed = 40;
    else speed = 1;
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
        case GLFW_KEY_SPACE:
            useWireframe = 1 - useWireframe;
            break;
        case GLFW_KEY_G:
            displayGrayscale = 1 - displayGrayscale;
            break;
        case GLFW_KEY_F:
            displayHalfMode = 1 - displayHalfMode;
            break;
        default:
            break;
        }
    }
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

    if (!displayHalfMode) {
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
    else firstMouse = true;
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}