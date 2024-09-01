#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <cstdlib>
#include <list>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow* window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(char const* path, bool gammaCorrection);
void renderQuad();
void renderCube();



//bools
bool hdr = false;
bool bloom = true;
bool konfeti = true;

//floats
float exposure = 0.5f;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 color;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 color;

};

struct Particle
{
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 speed;

    float time_left;

};

void genParticles(list<Particle>* particles, glm::vec3 position);
void updateParticles(list<Particle>* particles, float deltaTime, glm::vec3 gForce, unsigned int* amount);



struct ModelInfo {
    Model model;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    float angle = 0.0f;
    //glm::vec3 scale = glm::vec3(0.0f, 0.0f, 0.0f);
    float scale = 0.05f;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    PointLight pointLight;
    int index = 0;
    int indexPointLight = 0;
    int indexSpotLight = 0;
    vector<ModelInfo> models;
    vector<PointLight> pointLights;
    vector<SpotLight> spotLights;

    ProgramState()
        : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);

    void UpdatedSaveToFile(std::string filename, unsigned numModels, unsigned numPointLights, unsigned numSpotLights);

    void UpdatedLoadFromFile(std::string filename, unsigned numModels, unsigned numPointLights, unsigned numSpotLights);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::UpdatedSaveToFile(std::string filename, unsigned numModels, unsigned numPointLights, unsigned numSpotLights) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
    for (int i = 0; i < numModels; i++)
    {
        out << models[i].position.x << ' '
            << models[i].position.y << ' '
            << models[i].position.z << '\n'
            << models[i].rotationAxis.x << ' '
            << models[i].rotationAxis.y << ' '
            << models[i].rotationAxis.z << '\n'
            << models[i].angle << '\n'
            << models[i].scale << '\n';
    }

    for (int i = 0; i < numPointLights; i++)
    {
        out << pointLights[i].position.x << ' '
            << pointLights[i].position.y << ' '
            << pointLights[i].position.z << '\n'
            << pointLights[i].color.x << ' '
            << pointLights[i].color.y << ' '
            << pointLights[i].color.z << '\n';

    }

    for (int i = 0; i < numSpotLights; i++)
    {
        out << spotLights[i].position.x << ' '
            << spotLights[i].position.y << ' '
            << spotLights[i].position.z << '\n'
            << spotLights[i].color.x << ' '
            << spotLights[i].color.y << ' '
            << spotLights[i].color.z << '\n';

    }


}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
            >> clearColor.g
            >> clearColor.b
            >> ImGuiEnabled
            >> camera.Position.x
            >> camera.Position.y
            >> camera.Position.z
            >> camera.Front.x
            >> camera.Front.y
            >> camera.Front.z;
    }
}

void ProgramState::UpdatedLoadFromFile(std::string filename, unsigned numModels, unsigned numPointLights, unsigned numSpotLights) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
            >> clearColor.g
            >> clearColor.b
            >> ImGuiEnabled
            >> camera.Position.x
            >> camera.Position.y
            >> camera.Position.z
            >> camera.Front.x
            >> camera.Front.y
            >> camera.Front.z;
        for (int i = 0; i < numModels; i++)
        {
            in >> models[i].position.x
                >> models[i].position.y
                >> models[i].position.z
                >> models[i].rotationAxis.x
                >> models[i].rotationAxis.y
                >> models[i].rotationAxis.z
                >> models[i].angle
                >> models[i].scale;
        }

        for (int i = 0; i < numPointLights; i++)
        {
            in >> pointLights[i].position.x
                >> pointLights[i].position.y
                >> pointLights[i].position.z
                >> pointLights[i].color.x
                >> pointLights[i].color.y
                >> pointLights[i].color.z;

        }

        for (int i = 0; i < numSpotLights; i++)
        {
            in >> spotLights[i].position.x
                >> spotLights[i].position.y
                >> spotLights[i].position.z
                >> spotLights[i].color.x
                >> spotLights[i].color.y
                >> spotLights[i].color.z;

        }

    }
}
ProgramState* programState;

void DrawImGui(ProgramState* programState);

int main() {
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RGProjekat", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox1.vs", "resources/shaders/skybox1.fs");
    Shader shaderTransparent("resources/shaders/3.2.blending.vs", "resources/shaders/3.2.blending.fs");

    Shader shaderGeometryPass("resources/shaders/8.2.g_buffer.vs", "resources/shaders/8.2.g_buffer.fs");
    Shader shaderLightingPass("resources/shaders/8.2.deferred_shading.vs", "resources/shaders/8.2.deferred_shading.fs");
    Shader shaderLightBox("resources/shaders/8.2.deferred_light_box.vs", "resources/shaders/8.2.deferred_light_box.fs");

    Shader shaderBloomFinal("resources/shaders/7.bloom_final.vs", "resources/shaders/7.bloom_final.fs");
    Shader shaderBlur("resources/shaders/7.blur.vs", "resources/shaders/7.blur.fs");

    Shader shaderBlend("resources/shaders/7.bloom_final.vs", "resources/shaders/3.2.blending.fs");
    Shader particleShader("resources/shaders/particle.vs", "resources/shaders/particle.fs");

    float transparentVertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f,  0.0f,  0.5f,  0.0f,  0.0f,
        0.0f,  0.0f,  -0.5f,  0.0f,  1.0f,
        1.0f,  0.0f,  -0.5f,  1.0f,  1.0f,

        0.0f,  0.0f,  0.5f,  0.0f,  0.0f,
        1.0f,  0.0f,  -0.5f,  1.0f,  1.0f,
        1.0f,  0.0f,  0.5f,  1.0f,  0.0f
    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };


    vector<std::string> faces
    {
            "resources/textures/skybox/right.jpg",
            "resources/textures/skybox/left.jpg",
            "resources/textures/skybox/top.jpg",
            "resources/textures/skybox/bottom.jpg",
            "resources/textures/skybox/front.jpg",
            "resources/textures/skybox/back.jpg"
    };


    stbi_set_flip_vertically_on_load(false);

    // load models
    // -----------
    //Model backpack("resources/objects/backpack/backpack.obj");
    //programState->models.push_back({ backpack });
    Model crystal("resources/objects/crystal/untitled.obj", hdr);
    programState->models.push_back({ crystal });
    Model temple("resources/objects/temple/scene.gltf", hdr);
    programState->models.push_back({ temple });
    Model chest("resources/objects/chest/scene.gltf", hdr);
    programState->models.push_back({ chest });
    //ourModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;


    unsigned int cubemapTexture = loadCubemap(faces);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //transparent
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int transparentTexture = loadTexture("resources/textures/stars.png", hdr);


    vector<glm::vec3> stars;
    glm::vec3 base = glm::vec3(-15.0f, 40.0f, -11.0f);
    stars.push_back(base);

    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            int x;
            int z;
            if (i % 2 == 0)
                x = -i / 2;
            else
                x = i / 2;
            if (j % 2 == 0)
                z = -j / 2;
            else
                z = j / 2;
            stars.push_back(glm::vec3(base.x + 9.2 * x, base.y + rand() % 5, base.z + 9.2 * z));

        }
    }


    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth1;
    glGenRenderbuffers(1, &rboDepth1);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth1);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments1[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments1);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    const unsigned int NR_LIGHTS = 16;

    srand(13);


    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
        float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);


        // also calculate random color
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        PointLight light;
        light.position = glm::vec3(xPos, yPos, zPos);
        light.color = glm::vec3(rColor, gColor, bColor);
        programState->pointLights.push_back(light);
    }

    for (int j = 0; j < 4; j++)
    {
        for (int i = 0; i < 3; i++)
        {


            glm::vec3 translateVector = glm::vec3(j * 0.5f, 0.0f, i * 0.5f);
            //translateVector = translateVector + glm::vec3(0.0f, 2.5f, 0.0f);

            float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
            float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
            float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0

            SpotLight light;
            light.position = translateVector;
            light.color = glm::vec3(rColor, gColor, bColor);
            programState->spotLights.push_back(light);

        }
    }
    SpotLight light;
    light.position = glm::vec3(0.15f, 15.0f, 0.0f);
    light.color = glm::vec3(1.0f, 0.7f, 0.5f);
    programState->spotLights.push_back(light);

 
    const unsigned int maxAmount = 20000;

    unsigned int colorVBO;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, maxAmount * 3 * sizeof(float), NULL, GL_STREAM_DRAW);

    unsigned int positionVBO;
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, maxAmount * 3 * sizeof(float), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles

    unsigned int scaleVBO;
    glGenBuffers(1, &scaleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, maxAmount * 3 * sizeof(float), NULL, GL_STREAM_DRAW);

    float instanceVertex[] = {
         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
    };
    unsigned int vertexVBO;
    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(instanceVertex), instanceVertex, GL_STATIC_DRAW);
    unsigned int instanceVAO;
    glGenVertexArrays(1, &instanceVAO);


    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    shaderTransparent.use();
    shaderTransparent.setInt("texture1", 0);

    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedoSpec", 2);

    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);

    shaderBlend.use();
    shaderBlend.setInt("texture1", 0);


    float positionBuffer[maxAmount * 3];
    float colorBuffer[maxAmount * 3];


    programState->camera.Position.x = 0.0f;
    programState->camera.Position.y = 0.0f;
    programState->camera.Position.z = 0.0f;

    if (1)
    {
        programState->UpdatedLoadFromFile("resources/program_state1.txt", programState->models.size(), programState->pointLights.size(), programState->spotLights.size());
    }

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    list<Particle> particles;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);
        unsigned int currentAmount;

        

        // render
        // ------

        
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // view/projection transformations
        shaderGeometryPass.use();
        glEnable(GL_CULL_FACE);
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        shaderGeometryPass.setMat4("projection", projection);
        shaderGeometryPass.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);



        for (unsigned int i = 0; i < programState->models.size(); i++) {
            ModelInfo* currentModel = &programState->models[i];
            model = glm::mat4(1.0f);

            model = glm::rotate(model, glm::radians(currentModel->angle), currentModel->rotationAxis);
            model = glm::translate(model, currentModel->position);
            model = model = glm::scale(model, glm::vec3(currentModel->scale));
            shaderGeometryPass.setMat4("model", model);
            currentModel->model.Draw(shaderGeometryPass);

        }


        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        // send light relevant uniforms

        for (unsigned int i = 0; i < programState->pointLights.size(); i++)
        {
            shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position",
                programState->pointLights[i].position);
            shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color",
                programState->pointLights[i].color);
            // update attenuation parameters and calculate radius
            const float constant = 1.0f;
            const float linear = 0.7f;
            const float quadratic = 0.8f;
            shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
            shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
            const float maxBrightness = std::fmaxf(std::fmaxf(programState->pointLights[i].color.r,
                programState->pointLights[i].color.g), programState->pointLights[i].color.b);
            float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
            shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Radius", radius);
        }


        for (unsigned int i = 0; i < programState->spotLights.size(); i++)
        {
            shaderLightingPass.setVec3("spotlights[" + std::to_string(i) + "].color",
                glm::vec3(programState->spotLights[i].color));
            shaderLightingPass.setVec3("spotlights[" + std::to_string(i) + "].position",
                glm::vec3(programState->spotLights[i].position));
            shaderLightingPass.setVec3("spotlights[" + std::to_string(i) + "].direction", 0.0f, -1.0f, 0.0f);

            shaderLightingPass.setVec3("spotlights[" + std::to_string(i) + "].ambient", 0.1f, 0.1f, 0.1f);
            shaderLightingPass.setVec3("spotlights[" + std::to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
            shaderLightingPass.setVec3("spotlights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);

            shaderLightingPass.setFloat("spotlights[" + std::to_string(i) + "].cutOff", glm::cos(glm::radians(12.5f)));
            shaderLightingPass.setFloat("spotlights[" + std::to_string(i) + "].outerCutOff", glm::cos(glm::radians(17.5f)));
            // update attenuation parameters and calculate radius
            const float constant = 1.0f;
            const float linear = 0.7f;
            const float quadratic = 0.8f;
            shaderLightingPass.setFloat("spotlights[" + std::to_string(i) + "].linear", linear);
            shaderLightingPass.setFloat("spotlights[" + std::to_string(i) + "].quadratic", quadratic);
            const float maxBrightness = std::fmaxf(std::fmaxf(programState->spotLights[i].color.r,
                programState->spotLights[i].color.g), programState->spotLights[i].color.b);
            float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
            shaderLightingPass.setFloat("spotlights[" + std::to_string(i) + "].Radius", radius);
        }

        shaderLightingPass.setVec3("viewPos", programState->camera.Position);
        // finally render quad
        renderQuad();

        // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);

        // 3. render lights on top of scene
        // --------------------------------
        shaderLightBox.use();
        shaderLightBox.setMat4("projection", projection);
        shaderLightBox.setMat4("view", view);
        for (unsigned int i = 0; i < programState->pointLights.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, programState->pointLights[i].position);
            model = glm::scale(model, glm::vec3(0.1f));
            shaderLightBox.setMat4("model", model);
            shaderLightBox.setVec3("lightColor", programState->pointLights[i].color);
            renderCube();
        }

        for (unsigned int i = 0; i < programState->spotLights.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, programState->spotLights[i].position);
            model = glm::scale(model, glm::vec3(0.05f));
            shaderLightBox.setMat4("model", model);
            shaderLightBox.setVec3("lightColor", programState->spotLights[i].color);
            renderCube();
        }

        glDisable(GL_CULL_FACE);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // skybox cube

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);


        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS); // set depth function back to default
        glDepthMask(GL_TRUE);


        glm::vec3 gForce = glm::vec3(0.0f, -9.8f, 0.0f);
        genParticles(&particles, glm::vec3(0.0f, 14.5f, -10.0f));
        updateParticles(&particles, deltaTime, gForce, &currentAmount);



        int i = 0;
        for (auto it = particles.begin(); it != particles.end(); ++it)
        {
            positionBuffer[i * 3] = it->position.x;
            positionBuffer[i * 3 + 1] = it->position.y;
            positionBuffer[i * 3 + 2] = it->position.z;
            colorBuffer[i * 3] = it->color.x;
            colorBuffer[i * 3 + 1] = it->color.y;
            colorBuffer[i * 3 + 2] = it->color.z;
            i++;

        }
        glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
        glBufferData(GL_ARRAY_BUFFER, maxAmount * 3 * sizeof(float), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
        glBufferSubData(GL_ARRAY_BUFFER, 0, currentAmount * sizeof(float) * 3, positionBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, maxAmount * 3 * sizeof(float), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
        glBufferSubData(GL_ARRAY_BUFFER, 0, currentAmount * sizeof(float) * 3, colorBuffer);


        particleShader.use();
        view = glm::mat4(programState->camera.GetViewMatrix());
        particleShader.setMat4("projection", projection);
        particleShader.setMat4("view", view);

        glBindVertexArray(instanceVAO);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glVertexAttribDivisor(0, 0);
        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);

        if(konfeti)
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, currentAmount);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        shaderTransparent.use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        view = programState->camera.GetViewMatrix();
        shaderTransparent.setMat4("projection", projection);
        shaderTransparent.setMat4("view", view);


        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < stars.size(); i++)
        {
            float distance = glm::length(programState->camera.Position - stars[i]);
            sorted[distance] = stars[i];
        }

        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            model = glm::scale(model, glm::vec3(10.0f));
            shaderTransparent.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glDisable(GL_BLEND);

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        shaderBlur.use();
        glActiveTexture(GL_TEXTURE0);
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();

        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

        
        shaderBloomFinal.setInt("bloom", bloom);
        shaderBloomFinal.setFloat("exposure", exposure);
        shaderBloomFinal.setBool("hdr", hdr);
        renderQuad();



        //glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );



        if (programState->ImGuiEnabled)
            DrawImGui(programState);
        
        //clearParticles(&particles);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    programState->UpdatedSaveToFile("resources/program_state1.txt", programState->models.size(),
        programState->pointLights.size(), programState->spotLights.size());
    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------

    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteVertexArrays(1, &instanceVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteFramebuffers(1, &gBuffer);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(1, &gAlbedoSpec);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &transparentTexture);
    glDeleteTextures(2, colorBuffers);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        programState->camera.Position.y += deltaTime * programState->camera.MovementSpeed;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        programState->camera.Position.y -= deltaTime * programState->camera.MovementSpeed;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState* programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


   

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Begin("PointLight Info");
    ImGui::DragInt("Index", &programState->indexPointLight, 1, 0, programState->pointLights.size());
    ImGui::DragFloat3("Light position", (float*)&programState->pointLights[programState->indexPointLight].position, 0.01, -20.0, 20.0);
    ImGui::DragFloat3("Light color", (float*)&programState->pointLights[programState->indexPointLight].color, 0.05, 0.00, 1.0);
    ImGui::End();

    ImGui::Begin("SpotLight Info");
    ImGui::DragInt("Index", &programState->indexSpotLight, 1, 0, programState->spotLights.size());
    ImGui::DragFloat3("Light position", (float*)&programState->spotLights[programState->indexSpotLight].position, 0.01, -20.0, 20.0);
    ImGui::DragFloat3("Light color", (float*)&programState->spotLights[programState->indexSpotLight].color, 0.05, 0.00, 1.0);


    ImGui::Begin("Model Info");
    ImGui::DragInt("Index", &programState->index, 0, 1, programState->models.size());
    ImGui::DragFloat3("Model position", (float*)&programState->models[programState->index].position, 0.01, -20.0, 20.0);
    ImGui::DragFloat("Model scale", &programState->models[programState->index].scale, 0.02, 0.02, 128.0);
    ImGui::DragFloat("Model rotation", &programState->models[programState->index].angle, 0.5, 0.0, 360.0);
    ImGui::End();



    static const char* items[] = { "Models", "PointLight", "SpotLight" };
    static int current_item = 0;


    ImGui::Begin("Choose");


    if (ImGui::BeginCombo("Select Item", items[current_item]))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (current_item == n);

            if (ImGui::Selectable(items[n], is_selected))
                current_item = n;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }


    if (ImGui::Button("Place"))
    {
        const Camera& c = programState->camera;
        if (current_item == 0)
            programState->models[programState->index].position = glm::vec3(c.Position.x, c.Position.y, c.Position.z);
        if (current_item == 1)
            programState->pointLights[programState->indexPointLight].position = glm::vec3(c.Position.x, c.Position.y, c.Position.z);
        if (current_item == 2)
            programState->spotLights[programState->indexSpotLight].position = glm::vec3(c.Position.x, c.Position.y, c.Position.z);
    }

    ImGui::End();



    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
        bloom = !bloom;
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
        hdr = !hdr;
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        exposure += 0.05f;
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        exposure -= 0.05f;
    if (key == GLFW_KEY_K && action == GLFW_PRESS)
        konfeti = !konfeti;
    

}


unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
             // bottom face
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             // top face
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void genParticles(list<Particle>* particles, glm::vec3 position)
{
    Particle particle;
    int particlesGenerated = 1;
    float angleUp = 80.0f;
    for (unsigned int i = 0; i < particlesGenerated; i++)
    {
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5);
        particle.color = glm::vec3(rColor, gColor, bColor);
        particle.position = position;
        float x = glm::sin(glm::radians(angleUp)) * glm::cos(glm::radians(rand() % 360 + 0.5));
        float z = glm::sin(glm::radians(angleUp)) * glm::sin(glm::radians(rand() % 360 + 0.5));
        float y = glm::cos(glm::radians(angleUp));
        glm::vec3 direction = glm::vec3(x, y, z);
        float speed = 2.0f;

        particle.speed = direction * speed;
        particle.time_left = 1.5f;

        (*particles).push_back(particle);
    }
}

void updateParticles(list<Particle>* particles, float deltaTime, glm::vec3 gForce, unsigned int* amount)
{
    unsigned int s = 0;
    auto it = particles->begin();
    while (it != particles->end())
    {
        it->position += it->speed * deltaTime;  
        it->speed += gForce * deltaTime;        
        it->time_left -= deltaTime;             

        if (it->time_left <= 0.0f) {
            it = particles->erase(it); 
        }
        else {
            ++it;
            s++;
        }
    }

    *amount = s; 
}



