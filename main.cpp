#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <FreeImage.h>

GLFWwindow* window;
GLuint VAO[2], VBO[2];
Shader shaderProgram = Shader();
Shader transformShaderProgram = Shader();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool isFirstMouse = true;
float prevX = SCR_WIDTH / 2.0f;
float prevY = SCR_HEIGHT / 2.0f;
double dt       = 0;
double prevTime = 0;

bool saveToImage = false;
int renderCount = 1;
int maxRenderCount = 100;
int numParticles = 2;
double G = 1;

void saveImage(int renderCount, bool saveAnimation)
{
    int img_width = SCR_WIDTH;
    int img_height = SCR_HEIGHT;
    BYTE* pixels = new BYTE[3 * img_width * img_height];
    glReadPixels(0, 0, img_width, img_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, img_width, img_height, 3 * img_width, 24, 0x0000FF, 0xFF0000, 0x00FF00, false);
    
    char filename[13];
    sprintf(filename, "img/%05d.bmp", renderCount);
    FreeImage_Save(FIF_BMP, image, filename, 0);
    FreeImage_Unload(image);
    delete [] pixels;
    
    if (saveAnimation)
        {
            glfwSetWindowShouldClose(window, true);
            std::system("ffmpeg -y -f image2 -i 'img/%05d.bmp' animation.gif");
        }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isFirstMouse)
        {
            prevX = xpos;
            prevY = ypos;
            isFirstMouse = false;
        }
    
    float xoffset = xpos - prevX;
    float yoffset = prevY - ypos; 
    prevX = xpos;
    prevY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        saveImage(renderCount, true);
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, dt);
}

void init()
{
    srand(time(NULL));
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);
    
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "N-Body", NULL, NULL);
    glfwMakeContextCurrent(window);
    
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    
    shaderProgram = Shader("shaders/render.vs", "shaders/render.fs");
    const char *varyings[] = {"outPos", "outVel"};
    transformShaderProgram = Shader("shaders/transform.vs", varyings, 2);
}

std::vector<glm::vec3> genData()
{
    std::vector<glm::vec3> vertices(2*numParticles);
    vertices[0] = glm::vec3(1,0,0);
    vertices[1] = glm::vec3(-0.5,0,0);
    vertices[2] = -vertices[0];
    vertices[3] = -vertices[1];

    // double radius = 1.0;
    // for (int i=0; i<numParticles; i++) {
    //     double rands[2];
    //     for (int j=0; j<2; j++) {
    //         rands[j] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //     }
    //     double phi = rands[0] * M_PI;
    //     double theta = rands[1] * M_PI * 2;
    //     double m = 1;
    //     Eigen::Vector3d r = { radius * std::sin(phi) * std::cos(theta),
    //         radius * std::sin(phi) * std::sin(theta),
    //         radius * std::cos(phi) };
    //     Eigen::Vector3d v = {0,0,0};
    //     Particle p = Particle(m, r, v);
    //     universe.addParticle(p);
    // }
    
    return vertices;
}

void setup()
{
    std::vector<glm::vec3> vertices = genData();
    
    glPointSize(10);
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    
    for (int i=0; i<2; i++) {
        glBindVertexArray(VAO[i]);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(0);
}

void render(double dt, int currBuffer)
{
    // Transform Feedback
    transformShaderProgram.use();
    transformShaderProgram.setFloat("dt", dt);
    
    glBindVertexArray(VAO[(currBuffer+1) % 2]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO[currBuffer]);
    
    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, numParticles);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
    
    // Render
    glClear(GL_COLOR_BUFFER_BIT);
    
    shaderProgram.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    shaderProgram.setMat4("mvp", projection * view * model);
    
    glBindVertexArray(VAO[currBuffer]);
    glDrawArrays(GL_POINTS, 0, numParticles);
}

void run()
{
    if (saveToImage)
        std::system("rm img/*");
    
    int currBuffer = 0;
    while (!glfwWindowShouldClose(window))
        {
            //std::cout << (float)renderCount/(float)maxRenderCount << "\r" << std::flush;
            
            double time = glfwGetTime();
            dt = time - prevTime;
            prevTime = time;
            
            processInput(window);
            render(dt, currBuffer);
            
            if (saveToImage)
                saveImage(renderCount, renderCount==maxRenderCount);
            renderCount++;
            
            glfwSwapBuffers(window);
            glfwPollEvents();
            
            currBuffer = (currBuffer + 1) % 2;
        }
}

int main()
{
    clock_t t1, t2;
    t1 = clock();
    
    init();
    setup();
    run();
    
    glDeleteVertexArrays(1, VAO);
    glDeleteBuffers(1, VBO);
    glfwTerminate();
    
    t2 = clock();
    float secs = ((float)t2-(float)t1)/CLOCKS_PER_SEC;
    std::cout << "Run time: " << secs << std::endl;
    
    return 0;
}