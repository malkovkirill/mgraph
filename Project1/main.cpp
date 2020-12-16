#include <glad/glad.h>
#include <glfw3.h>
#include "shader_class.h"
#include <iostream>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 LightPos = glm::vec3(0.0, 5.0f, 0.0f);
glm::vec3 Camerapos = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 Cameradir = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));
glm::vec3 Cameraright = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0), Cameradir));
glm::vec3 Cameraup = glm::normalize(glm::cross(Cameradir, Cameraright));

glm::vec3 LightColor = glm::vec3(2.3f, 2.3f, 2.3f);

void coord_system(glm::vec3* pos, glm::vec2* tpos, glm::vec3 norm, glm::vec3* tangent, glm::vec3* bitangent);
void create_mirror_scene(Shader SceneShader, Shader LightShader, unsigned int FloorTexture, unsigned int FloorNormal, unsigned int FloorParallax, unsigned int CubeTexture, unsigned int CubeNormal, unsigned int CubeParallax, int i);
void createFrameBuffer(unsigned int* FB, unsigned int* Tex, unsigned int* RBO, unsigned int WIDTH, unsigned int HEIGHT);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void create_scene(Shader SceneShader, unsigned int FloorTexture, unsigned int FloorNormal, unsigned int FloorParallax, unsigned int CubeTexture, unsigned int CubeNormal, unsigned int CubeParallax, int i);
void create_cube(Shader Shadr, glm::mat4 model);
void create_floor();
unsigned int texLoad(char const* Path);
// Константы
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTime = 0.0f;
float lastTime = 0.0f;
float lastX = SCR_WIDTH/2, lastY = SCR_HEIGHT/2;
float yaw = -90.0f, pitch = 0;
bool firstMouse = 1;


int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MyWindow", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // отсюда начинаем

    glEnable(GL_DEPTH_TEST);
    Shader SceneShader("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Parket.vs", "C:/Users/Малков Кирилл/source/repos/Project1/Project1/Parket.fs");
    Shader Depth("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Depth.vs", "C:/Users/Малков Кирилл/source/repos/Project1/Project1/Depth.fs");
    Shader LightShader("C:/Users/Малков Кирилл/source/repos/Project1/Project1/V_LightShader.txt", "C:/Users/Малков Кирилл/source/repos/Project1/Project1/F_LightShader.txt");
    Shader DepthMap("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Depth_Map.vs", "C:/Users/Малков Кирилл/source/repos/Project1/Project1/Depth_Map.fs");
    Shader Mirror("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Mirror.vs", "C:/Users/Малков Кирилл/source/repos/Project1/Project1/Mirror.fs");
    
    unsigned int CubeTexture = texLoad("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Textures/wooden_container.jpg");
    unsigned int StoneTexture = texLoad("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Textures/bricks.jpg");
    unsigned int StoneNormal = texLoad("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Textures/bricks_normal.jpg");
    unsigned int StoneParallax = texLoad("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Textures/Stone_Parallax.jpg");
    unsigned int DesertTexture = texLoad("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Textures/stone.jpg");
    unsigned int DesertNormal = texLoad("C:/Users/Малков Кирилл/source/repos/Project1/Project1/Textures/Stone_Normal_Map.png");

    unsigned int MirrorFBO[7] = { 0 }, MirrorTex[7] = { 0 }, MirrorRBO[7] = { 0 };
    for (int i = 0; i < 7; i++)
    {
        createFrameBuffer(MirrorFBO + i, MirrorTex + i, MirrorRBO + i, 800, 600);
    }

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    SceneShader.use(); 

    glUniform1i(glGetUniformLocation(SceneShader.ID, "shadowMap"), 0);
    SceneShader.setInt("texture", 1);
    SceneShader.setInt("texture_norm", 2);
    SceneShader.setInt("parallaxMap", 3);

    DepthMap.use();
    DepthMap.setInt("depthMap", 0);

    Mirror.use();
    Mirror.setInt("shadowMap", 0);
    Mirror.setInt("MirrorTex", 1);
    unsigned int MirCubeVAO[7] = { 0 };
    unsigned int MirCubeVBO[7] = { 0 };
    float vertices[] = {
                    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
                     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
                     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
                     1.0f,  1.0f, -1.0f, 0.0f,  0.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f, 0.0f,  0.0f, -1.0f,
                    -1.0f,  1.0f, -1.0f, 0.0f,  0.0f, -1.0f,
                    
                    -1.0f, -1.0f, -1.0f, 0.0f,  0.0f,  -1.0f,
                     1.0f,  1.0f, -1.0f, 0.0f,  0.0f,  -1.0f,
                     1.0f, -1.0f, -1.0f, 0.0f,  0.0f,  -1.0f,
                     1.0f,  1.0f, -1.0f, 0.0f,  0.0f,  -1.0f,
                    -1.0f, -1.0f, -1.0f, 0.0f,  0.0f,  -1.0f,
                    -1.0f,  1.0f, -1.0f, 0.0f,  0.0f, -1.0f,

                    -1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
                     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
                     1.0f,  1.0f,  1.0f, 1.0f,  0.0f,  0.0f,
                     1.0f,  1.0f,  1.0f, 1.0f,  0.0f,  0.0f,
                    -1.0f,  1.0f,  1.0f, 1.0f,  0.0f,  0.0f,
                    -1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,

                    -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,
                    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
                    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
                    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
                    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
                    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,

                     1.0f,  1.0f,  1.0f, 1.0f,  0.0f,  0.0f,
                     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
                     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
                     1.0f, -1.0f, -1.0f, 1.0f,  0.0f,  0.0f,
                     1.0f,  1.0f,  1.0f, 1.0f,  0.0f,  0.0f,
                     1.0f, -1.0f,  1.0f, 1.0f,  0.0f,  0.0f,

                    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f,  0.0f,
                     1.0f, -1.0f, -1.0f, 0.0f, -1.0f,  0.0f,
                     1.0f, -1.0f,  1.0f, 0.0f, -1.0f,  0.0f,
                     1.0f, -1.0f,  1.0f, 0.0f, -1.0f,  0.0f,
                    -1.0f, -1.0f,  1.0f, 0.0f, -1.0f,  0.0f,
                    -1.0f, -1.0f, -1.0f, 0.0f, -1.0f,  0.0f,

                    -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,  0.0f,
                     1.0f,  1.0f , 1.0f, 0.0f, 1.0f,  0.0f,
                     1.0f,  1.0f, -1.0f, 0.0f, 1.0f,  0.0f,
                     1.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f,
                    -1.0f,  1.0f, -1.0f, 0.0f, 1.0f,  0.0f,
                    -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f
    };
    for (int i = 0; i < 7; i++)
    {
        glGenVertexArrays(1, MirCubeVAO + i);
        glGenBuffers(1, MirCubeVBO + i);
        glBindVertexArray(MirCubeVAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, MirCubeVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, 6 * 6 * sizeof(float), vertices + 6 * 6 * i, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    float Depth_fl[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f
    };
    unsigned int depthVAO, depthVBO;
    glGenVertexArrays(1, &depthVAO);
    glGenBuffers(1, &depthVBO);
    glBindBuffer(GL_ARRAY_BUFFER, depthVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Depth_fl), Depth_fl, GL_STATIC_DRAW);

    glBindVertexArray(depthVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window))
    {
        float a = 2 * sin(glfwGetTime());
        float b = 2 * cos(glfwGetTime());
        LightPos = glm::vec3(b + 0.0 + a,4.0f, a+b+3.0f);
        float currentTime = (float)glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float near_plane = 0.005f, far_plane = 25.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        glm::mat4 lightView = glm::lookAt(LightPos,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        Depth.use();
        Depth.setMat4("lightSpaceMatr", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        create_scene(Depth, DesertTexture, DesertNormal, StoneParallax, StoneTexture, StoneNormal, StoneParallax, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        DepthMap.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glBindVertexArray(depthVAO);
       // glDrawArrays(GL_TRIANGLES, 0, 6);
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 model = glm::mat4(1.0);

//теперь создаем сцену
        //сначала делаем грани зеркала
        glm::vec3 Mirpos[7] = { glm::vec3(0.0) };

        glm::vec3 Mirdir[7] = { glm::vec3(0.0) };
        for (int i = 0; i < 7; i++)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, MirrorFBO[i]);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (i == 4)
            {
                Mirdir[i] = Cameradir -  glm::vec3(2 * Cameradir.x, 0, 0);
                Mirpos[i] = Camerapos - (glm::vec3(2 * Camerapos.x, 0, 0) - glm::vec3(2 * 1.0, 0, 0));
            }
            if (i == 6)
            {
                Mirdir[i] = Cameradir -  glm::vec3(0, 2 * Cameradir.y, 0);
                Mirpos[i] = Camerapos - (glm::vec3(0, 2 * Camerapos.y, 0) - glm::vec3(0, 1.0 * 2, 0));
            }
            if (i == 2)
            {
                Mirpos[i] = Camerapos - (glm::vec3(0, 0, 2 * Camerapos.z) - glm::vec3(0, 0, 1.0 * 2));
                Mirdir[i] = Cameradir - glm::vec3(0, 0, 2 * Cameradir.z);
            }
            if (i == 3)
            {
                Mirdir[i] = Cameradir - glm::vec3(2 * Cameradir.x, 0, 0);
                Mirpos[i] = Camerapos - (glm::vec3(2 * Camerapos.x, 0, 0) - glm::vec3(-2 * 1.0, 0, 0));
            }
            if (i == 5)
            {
                Mirdir[i] = Cameradir - glm::vec3(0, 2 * Cameradir.y, 0);
                Mirpos[i] = Camerapos - (glm::vec3(0, 2 * Camerapos.y, 0) - glm::vec3(0, -1.0 * 2, 0));
            }
            if (i == 1)
            {
                Mirdir[i] = Cameradir - glm::vec3(0, 0, 2 * Cameradir.z);
                Mirpos[i] = Camerapos - glm::vec3(0, 0, 2 * Camerapos.z) + glm::vec3(0, 0, -1.0 * 2);
            }
            if (i == 0)
            {
                Mirdir[i] = Cameradir - glm::vec3(0, 0, 2 * Cameradir.z);
                Mirpos[i] = Camerapos - glm::vec3(0, 0, 2 * Camerapos.z) + glm::vec3(0, 0, 1.0 * 2);
            }

            view = glm::lookAt(Mirpos[i], Mirpos[i] + Mirdir[i], Cameraup);
            projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
            LightShader.setMat4("projection", projection);
            model = glm::translate(model, LightPos);
            LightShader.setMat4("view", view);
            LightShader.setMat4("model", model);

            SceneShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            SceneShader.setVec3("LightColor", LightColor);
            SceneShader.setVec3("viewPos", Mirpos[i]);
            SceneShader.setVec3("LightPos", LightPos);
            SceneShader.setMat4("projection", projection);
            SceneShader.setMat4("LightSpaceMatr", lightSpaceMatrix);
            SceneShader.setMat4("view", view);
            create_mirror_scene(SceneShader, LightShader, DesertTexture, DesertNormal, StoneParallax, StoneTexture, StoneNormal, StoneParallax, i);


            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
       
        // делаем источник освещения
        LightShader.use();

        view = glm::lookAt(Camerapos, Camerapos + Cameradir, Cameraup);

        LightShader.setMat4("projection", projection);
        model = glm::mat4(1.0);

        model = glm::translate(model, LightPos);
        LightShader.setMat4("view", view);
        LightShader.setMat4("model", model);

        create_cube(LightShader, model);

        //делаем остальную сцену

        SceneShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        SceneShader.setVec3("LightColor", LightColor);
        SceneShader.setVec3("viewPos", Camerapos);
        SceneShader.setVec3("LightPos", LightPos);
        SceneShader.setMat4("projection", projection);
        SceneShader.setMat4("LightSpaceMatr", lightSpaceMatrix);
        SceneShader.setMat4("view", view);
        create_scene(SceneShader, DesertTexture, DesertNormal, StoneParallax, StoneTexture, StoneNormal, StoneParallax, 1);
        glBindVertexArray(0);


        Mirror.use();
        Mirror.setMat4("projection", projection);
        glm::mat4 MirProjection = projection;
        model = glm::mat4(1.0f);
        Mirror.setMat4("model", model);
        Mirror.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        Mirror.setVec3("LightColor", LightColor);
        Mirror.setVec3("viewPos", Camerapos);
        Mirror.setVec3("LightPos", LightPos);
        Mirror.setMat4("projection", projection);
        Mirror.setMat4("LightSpaceMatr", lightSpaceMatrix);
        Mirror.setMat4("view", view);

        for (int i = 1; i < 7; ++i)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, MirrorTex[i]);
            
            glBindVertexArray(MirCubeVAO[i]);
            glm::mat4 MirView = glm::lookAt(Mirpos[i],
                Mirpos[i]+ Mirdir[i],
                Cameraup);
            glm::mat4 MirSpaceMatrix = MirProjection * MirView;

            Mirror.setMat4("MirSpaceMatr", MirSpaceMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
     /*   DepthMap.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, MirrorTex[i+1]);
        glBindVertexArray(depthVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);*/
      

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


void create_scene(Shader SceneShader, unsigned int FloorTexture, unsigned int FloorNormal, unsigned int FloorParallax, unsigned int CubeTexture, unsigned int CubeNormal, unsigned int CubeParallax, int i)
{
    glm::mat4 model = glm::mat4(1.0f);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, FloorTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, FloorNormal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, FloorParallax);
    SceneShader.setFloat("height_scale", 0.1);
    SceneShader.setMat4("model",model);
    create_floor();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 0.0f, 2.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, CubeTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, CubeNormal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, CubeParallax);
    SceneShader.setFloat("height_scale", 0.1);
    SceneShader.setMat4("model", model);
    create_cube(SceneShader, model);
    if (i == 0)
    {
        model = glm::mat4(1.0f);
        SceneShader.setMat4("model", model);
        create_cube(SceneShader, model);
    }
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, CubeTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, CubeNormal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, CubeParallax);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 3.0f));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(7.0, 0.0, -2.0)));
    model = glm::scale(model, glm::vec3(0.25));
    SceneShader.setFloat("height_scale", 0.1);
    SceneShader.setMat4("model", model);
    create_cube(SceneShader, model);


}

unsigned int mirLightCubeVAO = 0;
unsigned int mirLightCubeVBO = 0;

void create_mirror_scene(Shader SceneShader, Shader LightShader, unsigned int FloorTexture, unsigned int FloorNormal, unsigned int FloorParallax, unsigned int CubeTexture, unsigned int CubeNormal, unsigned int CubeParallax, int i)
{
    glm::mat4 model = glm::mat4(1.0f);
    if (i == 1 || i == 2 || i == 3 || i == 4)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, FloorTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, FloorNormal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, FloorParallax);
        SceneShader.setMat4("model", model);
        SceneShader.setFloat("height_scale", 0.1);
        create_floor();
    }
    if (i == 2)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 7.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, CubeTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, CubeNormal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, CubeParallax);
        SceneShader.setMat4("model", model);
        SceneShader.setFloat("height_scale", 0.1);
        create_cube(SceneShader, model);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, CubeTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, CubeNormal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, CubeParallax);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 3.0f));
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(7.0, 0.0, -2.0)));
        model = glm::scale(model, glm::vec3(0.25));
        SceneShader.setMat4("model", model);
        create_cube(SceneShader, model);
    }
    if (i == 6)
    {
        LightShader.use();
        if (mirLightCubeVAO == 0)
        {
            float vertices[] = {
                    -1.0f, -1.0f, -1.0f,
                     1.0f,  1.0f, -1.0f,
                     1.0f, -1.0f, -1.0f,
                     1.0f,  1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f,  1.0f, -1.0f,

                    -1.0f, -1.0f, -1.0f,
                     1.0f,  1.0f, -1.0f,
                     1.0f, -1.0f, -1.0f,
                     1.0f,  1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f,  1.0f, -1.0f,

                    -1.0f, -1.0f,  1.0f,
                     1.0f, -1.0f,  1.0f,
                     1.0f,  1.0f,  1.0f,
                     1.0f,  1.0f,  1.0f,
                    -1.0f,  1.0f,  1.0f,
                    -1.0f, -1.0f,  1.0f,

                    -1.0f,  1.0f,  1.0f,
                    -1.0f,  1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f,  1.0f,
                    -1.0f,  1.0f,  1.0f,

                     1.0f,  1.0f,  1.0f,
                     1.0f, -1.0f, -1.0f,
                     1.0f,  1.0f, -1.0f,
                     1.0f, -1.0f, -1.0f,
                     1.0f,  1.0f,  1.0f,
                     1.0f, -1.0f,  1.0f,

                    -1.0f, -1.0f, -1.0f,
                     1.0f, -1.0f, -1.0f,
                     1.0f, -1.0f,  1.0f,
                     1.0f, -1.0f,  1.0f,
                    -1.0f, -1.0f,  1.0f,
                    -1.0f, -1.0f, -1.0f,

                    -1.0f,  1.0f, -1.0f,
                     1.0f,  1.0f , 1.0f,
                     1.0f,  1.0f, -1.0f,
                     1.0f,  1.0f,  1.0f,
                    -1.0f,  1.0f, -1.0f,
                    -1.0f,  1.0f,  1.0f
            };
            glGenVertexArrays(1, &mirLightCubeVAO);
            glGenBuffers(1, &mirLightCubeVBO);
            glBindVertexArray(mirLightCubeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, mirLightCubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        glBindVertexArray(mirLightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //куб света в зеркалькой грани
    }
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void create_cube(Shader Shadr, glm::mat4 model)
{
    if (cubeVAO == 0)
    {
        glm::vec3 pos[4];
        pos[0] = glm::vec3(-1.0f, -1.0f, -1.0f);
        pos[2] = glm::vec3(1.0f, 1.0f, -1.0f);
        pos[1] = glm::vec3(1.0f, -1.0f, -1.0f);
        pos[3] = glm::vec3(-1.0f, 1.0f, -1.0f);

        glm::vec3 norm = glm::vec3(0.0f, 0.0f, -1.0f);

        glm::vec2 tpos[4];
        tpos[0] = glm::vec2(0.0f, 0.0f);
        tpos[2] = glm::vec2(1.0f, 1.0f);
        tpos[1] = glm::vec2(1.0f, 0.0f);
        tpos[3] = glm::vec2(0.0f, 1.0f);

        glm::vec3 tangent[2], bitangent[2];

        glm::vec3 vert1 = pos[1] - pos[0];
        glm::vec3 vert2 = pos[2] - pos[0];

        glm::vec2 tvert1 = tpos[1] - tpos[0];
        glm::vec2 tvert2 = tpos[2] - tpos[0];

        float descr = 1 / (tvert1.x * tvert2.y - tvert1.y * tvert2.x);

        tangent[0].x = descr * (tvert2.y * vert1.x - tvert1.y * vert2.x);
        tangent[0].y = descr * (tvert2.y * vert1.y - tvert1.y * vert2.y);
        tangent[0].z = descr * (tvert2.y * vert1.z - tvert1.y * vert2.z);

        bitangent[0] = glm::cross(norm, tangent[0]);

        vert1 = pos[2] - pos[0];
        vert2 = pos[3] - pos[0];
        tvert1 = tpos[2] - tpos[0];
        tvert2 = tpos[3] - tpos[0];

        descr = 1 / (tvert1.x * tvert2.y - tvert1.y * tvert2.x);

        tangent[1].x = descr * (tvert2.y * vert1.x - tvert1.y * vert2.x);
        tangent[1].y = descr * (tvert2.y * vert1.y - tvert1.y * vert2.y);
        tangent[1].z = descr * (tvert2.y * vert1.z - tvert1.y * vert2.z);


        bitangent[1] = glm::cross(norm, tangent[1]);
        float vertices[] = {
            pos[0].x, pos[0].y, pos[0].z, norm.x, norm.y, norm.z, tpos[0].x, tpos[0].y, tangent[0].x, tangent[0].y, tangent[0].z, bitangent[0].x, bitangent[0].y, bitangent[0].z,
            pos[1].x, pos[1].y, pos[1].z, norm.x, norm.y, norm.z, tpos[1].x, tpos[1].y, tangent[0].x, tangent[0].y, tangent[0].z, bitangent[0].x, bitangent[0].y, bitangent[0].z,
            pos[2].x, pos[2].y, pos[2].z, norm.x, norm.y, norm.z, tpos[2].x, tpos[2].y, tangent[0].x, tangent[0].y, tangent[0].z, bitangent[0].x, bitangent[0].y, bitangent[0].z,

            pos[0].x, pos[0].y, pos[0].z, norm.x, norm.y, norm.z, tpos[0].x, tpos[0].y, tangent[1].x, tangent[1].y, tangent[1].z, bitangent[1].x, bitangent[1].y, bitangent[1].z,
            pos[2].x, pos[2].y, pos[2].z, norm.x, norm.y, norm.z, tpos[2].x, tpos[2].y, tangent[1].x, tangent[1].y, tangent[1].z, bitangent[1].x, bitangent[1].y, bitangent[1].z,
            pos[3].x, pos[3].y, pos[3].z, norm.x, norm.y, norm.z,  tpos[3].x, tpos[3].y, tangent[1].x, tangent[1].y, tangent[1].z, bitangent[1].x, bitangent[1].y, bitangent[1].z
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(cubeVAO);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glm::mat4 model1 = model;
    model1 = glm::rotate(model1, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    Shadr.setMat4("model", model1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model1 = model;
    model1 = glm::rotate(model1, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
    model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 0.0f));
    Shadr.setMat4("model", model1);
    //glDrawArrays(GL_TRIANGLES, 0, 6);

    model1 = model;
    model1 = glm::rotate(model1, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    Shadr.setMat4("model", model1);
    //glDrawArrays(GL_TRIANGLES, 0, 6);

    model1 = model;
    model1 = glm::rotate(model1, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
    Shadr.setMat4("model", model1);
    //glDrawArrays(GL_TRIANGLES, 0, 6);

    model1 = model;
    model1 = glm::rotate(model1, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
    Shadr.setMat4("model", model1);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

}

void coord_system(glm::vec3* pos, glm::vec2* tpos, glm::vec3 norm, glm::vec3* tangent, glm::vec3* bitangent)
{
    glm::vec3 vert1 = pos[1] - pos[0];
    glm::vec3 vert2 = pos[2] - pos[0];

    glm::vec2 tvert1 = tpos[1] - tpos[0];
    glm::vec2 tvert2 = tpos[2] - tpos[0];

    float descr = 1 / (tvert1.x * tvert2.y - tvert1.y * tvert2.x);

    tangent[0].x = descr * (tvert2.y * vert1.x - tvert1.y * vert2.x);
    tangent[0].y = descr * (tvert2.y * vert1.y - tvert1.y * vert2.y);
    tangent[0].z = descr * (tvert2.y * vert1.z - tvert1.y * vert2.z);

    bitangent[0] = glm::cross(norm, tangent[0]);

    vert1 = pos[2] - pos[0];
    vert2 = pos[3] - pos[0];
    tvert1 = tpos[2] - tpos[0];
    tvert2 = tpos[3] - tpos[0];

    descr = 1 / (tvert1.x * tvert2.y - tvert1.y * tvert2.x);

    tangent[1].x = descr * (tvert2.y * vert1.x - tvert1.y * vert2.x);
    tangent[1].y = descr * (tvert2.y * vert1.y - tvert1.y * vert2.y);
    tangent[1].z = descr * (tvert2.y * vert1.z - tvert1.y * vert2.z);

    bitangent[1] = glm::cross(norm, tangent[1]);
};

unsigned int floorVAO = 0;
unsigned int floorVBO = 0;
void create_floor()
{
    if (floorVAO == 0)
    {
        glm::vec3 pos[4];
        pos[0] = glm::vec3(25.0f, -1.0f, 25.0f);
        pos[1] = glm::vec3(-25.0f, -1.0f, 25.0f);
        pos[2] = glm::vec3(-25.0f, -1.0f, -25.0f);
        pos[3] = glm::vec3(25.0f, -1.0f, -25.0f);

        glm::vec3 norm(0.0f, 1.0f, 0.0f);

        glm::vec2 tpos[4];
        tpos[0] = glm::vec2(25.0f, 0.0f);
        tpos[1] = glm::vec2(0.0f, 0.0f);
        tpos[2] = glm::vec2(0.0f, 25.0f);
        tpos[3] = glm::vec2(25.0f, 25.0f);

        glm::vec3 tangent[2], bitangent[2];
        coord_system(pos, tpos, norm, tangent, bitangent);
        glm::vec3 vert1 = pos[1] - pos[0];
        glm::vec3 vert2 = pos[2] - pos[0];

        glm::vec2 tvert1 = tpos[1] - tpos[0];
        glm::vec2 tvert2 = tpos[2] - tpos[0];

        float descr = 1 / (tvert1.x * tvert2.y - tvert1.y * tvert2.x);

        tangent[0].x = descr * (tvert2.y * vert1.x - tvert1.y * vert2.x);
        tangent[0].y = descr * (tvert2.y * vert1.y - tvert1.y * vert2.y);
        tangent[0].z = descr * (tvert2.y * vert1.z - tvert1.y * vert2.z);

        bitangent[0] = glm::cross(norm, tangent[0]);

        vert1 = pos[2] - pos[0];
        vert2 = pos[3] - pos[0];
        tvert1 = tpos[2] - tpos[0];
        tvert2 = tpos[3] - tpos[0];

        descr = 1 / (tvert1.x * tvert2.y - tvert1.y * tvert2.x);

        tangent[1].x = descr * (tvert2.y * vert1.x - tvert1.y * vert2.x);
        tangent[1].y = descr * (tvert2.y * vert1.y - tvert1.y * vert2.y);
        tangent[1].z = descr * (tvert2.y * vert1.z - tvert1.y * vert2.z);

        bitangent[1] = glm::cross(norm, tangent[1]);
        float floor[] = {
            pos[0].x, pos[0].y, pos[0].z, norm.x, norm.y, norm.z, tpos[0].x, tpos[0].y, tangent[0].x, tangent[0].y, tangent[0].z, bitangent[0].x, bitangent[0].y, bitangent[0].z,
            pos[1].x, pos[1].y, pos[1].z, norm.x, norm.y, norm.z, tpos[1].x, tpos[1].y, tangent[0].x, tangent[0].y, tangent[0].z, bitangent[0].x, bitangent[0].y, bitangent[0].z,
            pos[2].x, pos[2].y, pos[2].z, norm.x, norm.y, norm.z, tpos[2].x, tpos[2].y, tangent[0].x, tangent[0].y, tangent[0].z, bitangent[0].x, bitangent[0].y, bitangent[0].z,

            pos[0].x, pos[0].y, pos[0].z, norm.x, norm.y, norm.z, tpos[0].x, tpos[0].y, tangent[1].x, tangent[1].y, tangent[1].z, bitangent[1].x, bitangent[1].y, bitangent[1].z,
            pos[2].x, pos[2].y, pos[2].z, norm.x, norm.y, norm.z, tpos[2].x, tpos[2].y, tangent[1].x, tangent[1].y, tangent[1].z, bitangent[1].x, bitangent[1].y, bitangent[1].z,
            pos[3].x, pos[3].y, pos[3].z, norm.x, norm.y, norm.z,  tpos[3].x, tpos[3].y, tangent[1].x, tangent[1].y, tangent[1].z, bitangent[1].x, bitangent[1].y, bitangent[1].z
        };
        glGenVertexArrays(1, &floorVAO);
        glGenBuffers(1, &floorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(floor), floor, GL_STATIC_DRAW);

        glBindVertexArray(floorVAO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(floorVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
};

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 3 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Camerapos += cameraSpeed * Cameradir;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Camerapos -= cameraSpeed * Cameradir;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Camerapos -= glm::normalize(glm::cross(Cameradir, Cameraup)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Camerapos += glm::normalize(glm::cross(Cameradir, Cameraup)) * cameraSpeed;
    }
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;


    Cameradir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    Cameradir.y = sin(glm::radians(pitch));
    Cameradir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    Cameradir = glm::normalize(Cameradir);
}

unsigned int texLoad(const char* Path)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(Path, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

void createFrameBuffer(unsigned int* FBO, unsigned int* Tex, unsigned int* RBO, unsigned int WIDTH, unsigned int HEIGHT)
{
    glGenFramebuffers(1, FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, *FBO);

    glGenTextures(1, Tex);
    glBindTexture(GL_TEXTURE_2D, *Tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *Tex, 0);

    glGenRenderbuffers(1, RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, *RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *RBO);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}