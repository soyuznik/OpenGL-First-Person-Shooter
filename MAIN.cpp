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

////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////// .obj file loader ////////////////////////////////////////////////


class ObjLoader {
public:
    std::vector<vec4> vertices; //v
    std::vector<vec2> texture_coordinates; //vt
    
    ObjLoader();
    void LoadObjFile(std::string filepath);
    void Clear();
    bool Contains(std::string WhichToSearch, std::string WhatToSearch);

};
ObjLoader::ObjLoader(){}


bool ObjLoader::Contains(std::string WhichToSearch, std::string WhatToSearch) {
    if (WhichToSearch.find(WhatToSearch) != std::string::npos) {
        return true;
    }
    return false;
}

void ObjLoader::LoadObjFile(std::string filepath) {
    ifstream file;
    file.open(filepath);
    std::string line;
    while (std::getline(file, line)) {
        if (line == "") continue;
        if (Contains(line, "#")) {
            continue;
        }
        if (Contains(line, "v ")) {
            using namespace std;
            stringstream _line(line);
            string invalid , x, y, z, w;
            _line >> invalid >> x >> y >> z >> w;
            if (x.empty() || y.empty() || z.empty()) {
                throw("Invalid Position");
            }
            if (w == "") {
                w = "1.0";
            }
            vertices.push_back(vec4(stof(x.c_str()), stof(y.c_str()), stof(z.c_str()), stof(w.c_str())));
            
        }
        if (Contains(line, "vt ")) {
            using namespace std;
            stringstream _line(line);
            string invalid , u , v;
            _line >> invalid >> u >> v ;
            if (u.empty() || v.empty()) {
                throw("Invalid Position");
            }
            texture_coordinates.push_back(vec2(stof(u.c_str()), stof(v.c_str())));
            
        }

    }

}
void ObjLoader::Clear() {
    vertices.clear();
    texture_coordinates.clear();
}


















void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
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

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("resources/shaders/7.4.camera.vs", "resources/shaders/7.4.camera.fs");

    
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices1[] = {
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
    ObjLoader loader;
    loader.LoadObjFile("resources/m4a1.obj");
    std::vector<vec4> VerVec = loader.vertices;
    std::vector<vec2> VtVec = loader.texture_coordinates;
    std::vector<float> vertices;
    for (int i = 0; i < VerVec.size(); i++) {
        vertices.push_back(VerVec[i].x);
        vertices.push_back(VerVec[i].y);
        vertices.push_back(VerVec[i].z);
        vertices.push_back(VtVec[i].x);
        vertices.push_back(VtVec[i].y);
    }
    std::vector<vec3> points;
    for (unsigned int a = 0; a < vertices.size(); a = a + 5) {
        points.push_back(vec3(vertices[a], vertices[a + 1], vertices[a + 2]));

    }
    AABB box(points[0], points[0]);
    for (size_t i = 1; i < points.size(); i++)
    {
        box.UpdateMinMax(points[i]);
    }

    
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //std::cout << glGetError();

    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2;
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
    // texture 2
    // ---------
    //glGenTextures(1, &texture2);
   // glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    vec3 saveLastPostion = camera.Position;
    
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
       // Sphere cameraSphere(camera.Position, 0.2f);
        
     //   if (Intersect(cameraSphere , box)) {
     //      camera.Position = saveLastPostion;
    //    }
    //    else saveLastPostion = camera.Position;
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
       

        // activate shader
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // render boxes
        glBindVertexArray(VAO);
        // calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 model = glm::mat4(0.1f); // make sure to initialize matrix to identity matrix first
        ourShader.setMat4("model", model);
        
        glDrawArrays(GL_TRIANGLES, 0, vertices.size()/5);
            
       

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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

