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
GLuint VAO, VBO, QuadVAO, QuadVBO, FBO;
GLuint positions[2], velocities[2];
Shader shaderProgram = Shader();
Shader updateTextureProgram = Shader();

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool isFirstMouse = true;
float prevX = SCR_WIDTH / 2.0f;
float prevY = SCR_HEIGHT / 2.0f;
double dt       = 0;
double prevTime = 0;

bool saveToImage = false;
int frameCount = 1;
int maxFrameCount = 200;
int N = 10000;
float timestepFactor = 700;
float pointSize = 1.1;
float G = 0.5;

void saveImage(int frameCount, bool saveAnimation)
{
    int img_width = SCR_WIDTH;
    int img_height = SCR_HEIGHT;
    BYTE* pixels = new BYTE[3 * img_width * img_height];
    glReadPixels(0, 0, img_width, img_height, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, img_width, img_height, 3 * img_width, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
    
    char filename[13];
    sprintf(filename, "img/%05d.bmp", frameCount);
    FreeImage_Save(FIF_BMP, image, filename, 0);
    FreeImage_Unload(image);
    delete [] pixels;
    
    if (saveAnimation)
        {
            glfwSetWindowShouldClose(window, true);
            std::system("ffmpeg -y -f image2 -i 'img/%05d.bmp' animation.gif");
        }
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, dt);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (saveToImage)
            saveImage(frameCount, true);
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_J) {
                G *= 5.0;
                std::cout << "G = " << G << std::endl;
            } else if (key == GLFW_KEY_K) {
                G *= 0.8;
                std::cout << "G = " << G << std::endl;
            } else if (key == GLFW_KEY_SPACE) {
                camera.MovementSpeed *= 1.2;
                std::cout << "Cam Speed = " << camera.MovementSpeed << std::endl;
            } else if (key == GLFW_KEY_LEFT_SHIFT) {
                camera.MovementSpeed *= 0.8;
                std::cout << "Cam Speed = " << camera.MovementSpeed << std::endl;
            }
        }
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
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    
    shaderProgram = Shader("shaders/draw.vs", "shaders/draw.fs");
    updateTextureProgram = Shader("shaders/update.vs", "shaders/update.fs");
}

void genData(float initPositions[], float initVelocities[])
{
    double radius = 0.5, v0 = 40;
    for (int i = 0; i < N/2; i++) {
        double rands[2];
        for (int j = 0; j < 2; j++) {
            rands[j] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        }
        float theta = rands[0] * M_PI;
        float phi = rands[1] * 2*M_PI;

        initPositions[4*i] = radius * glm::sin(theta) * glm::cos(phi) - 2*radius;
        initPositions[4*i+1] = radius * glm::sin(theta) * glm::sin(phi);
        initPositions[4*i+2] = radius * glm::cos(theta);
        initPositions[4*i+3] = 1;

        initVelocities[4*i] = v0;
        initVelocities[4*i+1] = 0;
        initVelocities[4*i+2] = 0;
        initVelocities[4*i+3] = 1;
    }
    for (int i = N/2; i < N; i++) {
        double rands[2];
        for (int j = 0; j < 2; j++) {
            rands[j] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        }
        float theta = rands[0] * M_PI;
        float phi = rands[1] * 2*M_PI;

        initPositions[4*i] = radius * glm::sin(theta) * glm::cos(phi) + 2*radius;
        initPositions[4*i+1] = radius * glm::sin(theta) * glm::sin(phi);
        initPositions[4*i+2] = radius * glm::cos(theta);
        initPositions[4*i+3] = 1;

        initVelocities[4*i] = -v0;
        initVelocities[4*i+1] = 0;
        initVelocities[4*i+2] = 0;
        initVelocities[4*i+3] = 1;
    }
}

void setup()
{
    float initPositions[4*N], initVelocities[4*N];
    genData(initPositions, initVelocities);
    int indices[N];
    for (int i = 0; i < N; i++) {
        indices[i] = i;
    }
    
    glPointSize(pointSize);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, 0, (void*)0);
    glBindVertexArray(0);
    
    // Quad VAO
    const GLfloat vertices[] = { -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f };
    glGenVertexArrays(1, &QuadVAO);
    glGenBuffers(1, &QuadVBO);
    
    glBindVertexArray(QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
    
    // Textures
    glGenTextures(2, positions);
    glGenTextures(2, velocities);
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, positions[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
        
        glBindTexture(GL_TEXTURE_2D, velocities[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, positions[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, 1, 0, GL_RGBA, GL_FLOAT, initPositions);
    glBindTexture(GL_TEXTURE_2D, velocities[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, 1, 0, GL_RGBA, GL_FLOAT, initVelocities);
    
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    for (int i = 0; i < 2; i++) {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2*i, GL_TEXTURE_2D, positions[i], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 + 2*i, GL_TEXTURE_2D, velocities[i], 0);
    }
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    shaderProgram.use();
    shaderProgram.setInt("positions", 0);
    shaderProgram.setInt("velocities", 1);
    
    updateTextureProgram.use();
    updateTextureProgram.setInt("N", N);
    updateTextureProgram.setInt("positions", 0);
    updateTextureProgram.setInt("velocities", 1);
}

void render(double dt, int k)
{
    // Update positions, velocities
    updateTextureProgram.use();
    updateTextureProgram.setFloat("G", G);
    updateTextureProgram.setFloat("dt", dt / timestepFactor);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    
    GLenum DrawBuffers[2];
    if (k == 0) {
        DrawBuffers[0] = GL_COLOR_ATTACHMENT2;
        DrawBuffers[1] = GL_COLOR_ATTACHMENT3;
    } else if (k == 1) {
        DrawBuffers[0] = GL_COLOR_ATTACHMENT0;
        DrawBuffers[1] = GL_COLOR_ATTACHMENT1;
    }
    glDrawBuffers(2, DrawBuffers);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positions[k]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocities[k]);
    
    glViewport(0, 0, N, 1);
    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    
    // Draw
    glClear(GL_COLOR_BUFFER_BIT);
    
    shaderProgram.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.0f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    shaderProgram.setMat4("mvp", projection * view * model);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positions[k]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocities[k]);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, N);
    glBindVertexArray(0);
}

void run()
{
    if (saveToImage)
        std::system("rm img/*");
    
    int k = 1;
    while (!glfwWindowShouldClose(window))
        {
            std::cout << (float)frameCount/(float)maxFrameCount << "\r" << std::flush;
            
            double time = glfwGetTime();
            dt = time - prevTime;
            prevTime = time;
            
            processInput(window);
            render(dt, k);
            
            if (saveToImage)
                saveImage(frameCount, frameCount==maxFrameCount);
            frameCount++;
            
            glfwSwapBuffers(window);
            glfwPollEvents();

            k = (k + 1) % 2;
        }
}

int main()
{
    init();
    setup();
    run();
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &QuadVBO);
    glDeleteFramebuffers(1, &FBO);
    glfwTerminate();

    std::cout << "frameCount = " << frameCount << std::endl;
    
    return 0;
}