// MIT License
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com

#include "mpvideocube.h"

int main(int argc, char const *argv[])
{
    if(argc<2) {
        std::cout << "Usage: " << argv[0] << " videofilename" << std::endl;
        return -1;
    }
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if ((window = glfwCreateWindow(window_width, window_height, "MPVideoCube", NULL, NULL)) == NULL)
    {
        std::cout << "ERROR::GLFW::Failed to create window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR::GLAD::Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH);
    glEnable(GL_MULTISAMPLE);

    // MPV initialization and configuration
    mpv = mpv_create();
    if (mpv_initialize(mpv) < MPV_ERROR_SUCCESS)
    {
        std::cout << "ERROR::MPV::Failed to initialize mpv" << std::endl;
        return -1;
    }

    mpv_opengl_init_params opengl_init_params{
        get_proc_address,
        nullptr,
        nullptr};

    int adv{1};

    mpv_render_param render_param[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &opengl_init_params},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &adv},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    if (mpv_render_context_create(&mpv_ctx, mpv, render_param) < MPV_ERROR_SUCCESS)
    {
        std::cout << "ERROR::MPV::Failed to create MPV render context" << std::endl;
        return -1;
    }

    mpv_set_wakeup_callback(mpv, on_mpv_events, NULL);
    mpv_render_context_set_update_callback(mpv_ctx, on_mpv_render_update, NULL);

    const char *cmd[] = {"loadfile", argv[1], NULL};
    mpv_command(mpv, cmd);
    mpv_set_option_string(mpv, "loop", "");
    mpv_set_option_string(mpv, "gpu-api", "opengl");
    mpv_set_option_string(mpv, "hwdec", "auto");
    mpv_set_option_string(mpv, "terminal", "yes");

    // SHADER creation

    Shader cubeShader("shaders/cube_vs.glsl", "shaders/cube_fs.glsl");
    Shader *screenShader = new Shader("shaders/screen_vs.glsl", "shaders/screen_fs.glsl");

    // CUBE Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0); // coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float))); //texture

    // SCREEN Quad VAO and VBO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0); // coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float))); //texture

    //Framebuffer for Video Target - Video Texture
    unsigned int video_rbo;
    glGenFramebuffers(1, &video_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, video_framebuffer);
    // create a color attachment texture
    glGenTextures(1, &video_textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbo_width, fbo_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, video_textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &video_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, video_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbo_width, fbo_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, video_rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: VIDEO Framebuffer #" << video_framebuffer << "is not complete!" << std::endl;

    //Framebuffer for Screen Target - Main Screen
    unsigned int screen_rbo;
    glGenFramebuffers(1, &screen_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, screen_framebuffer);
    // create a color attachment texture
    glGenTextures(1, &screen_textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, screen_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &screen_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, screen_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, screen_rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: SCREEN Framebuffer #" << screen_framebuffer << "is not complete!" << std::endl;

    mpv_opengl_fbo mpv_fbo{
        static_cast<int>(video_framebuffer),
        fbo_width,
        fbo_height,
        0};
    int flip_y{1};

    mpv_render_param params_fbo[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}};

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processGLFWInput(window);
        // -----

        mpv_render_context_render(mpv_ctx, params_fbo); // this "renders" to the video_framebuffer "linked by ID" in the params_fbo - BLOCKING

        // **************** RENDER TO THE SCREEN FBO
        glBindFramebuffer(GL_FRAMEBUFFER, screen_framebuffer); // <-- BIND THE SCREEN FBO
        glEnable(GL_DEPTH_TEST);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);                   // RED BACKGROUND
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cubeShader.use(); // <-- The CUBE SHADER
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window_width / (float)window_height, 0.1f, 100.0f);
        cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
        model = glm::rotate(model, currentFrame, glm::vec3(sin(currentFrame / 10.), cos(currentFrame / 10.), sin(currentFrame / 10.) * cos(currentFrame / 10.)));
        glBindVertexArray(cubeVAO); // <-- The CUBE
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer); // <-- VIDEO Colorbuffer IS THE TEXTURE

        cubeShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36); // CUBE 1

        model = glm::rotate(model, currentFrame, glm::vec3(sin(currentFrame / 3.), cos(currentFrame / 7.), sin(currentFrame / 11.) * cos(currentFrame / 2.)));
        model = glm::translate(model, glm::vec3(1 * sin(currentFrame / 2.0), sin(currentFrame / 13.0), 1 * sin(currentFrame / 53.0)));
        cubeShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36); // CUBE 2

        // **************** RENDER TO THE MAIN FBO.
        // We use the QUAD with the SCREEN FBO as texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // <-- BIND THE DEFAULT FBO
        glDisable(GL_DEPTH_TEST);             // NO DEPTH TEST!
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        screenShader->use();
        // screenShader.use(); // <-- The SCREEN SHADER
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(sin(currentFrame / 10.0), sin(currentFrame / 10.0), 0.0f));

        // screenShader.setMat4("model", model);
        screenShader->setMat4("model", model);
        glBindVertexArray(quadVBO); // <-- The SCREEN QUAD
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screen_textureColorbuffer); // <-- SCREEN Colorbuffer IS THE TEXTURE
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // -----
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processGLFWInput(GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_FALSE);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Returns the address of the specified function (name) for the given context (ctx)
static void *get_proc_address(void *ctx, const char *name)
{
    glfwGetCurrentContext();
    return reinterpret_cast<void *>(glfwGetProcAddress(name));
}

static void on_mpv_render_update(void *ctx)
{
    // std::cout << "INFO::" << __func__ << std::endl;
}

static void on_mpv_events(void *ctx)
{
    // std::cout << "INFO::" << __func__ << std::endl;
}
