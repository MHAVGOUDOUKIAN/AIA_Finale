#include "OpenGL_App.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

OpenGL_App::OpenGL_App() : opCV() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (m_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Failed to initialize GLAD" << std::endl; }

    m_shaderProgram = new OpenGL_Shader("src/shader.vert", "src/shader.frag");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
}

OpenGL_App::~OpenGL_App() {}
        
void OpenGL_App::run() {
    float crate_vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    };

    float camera_sprite_vertices[] = {
        -1.f, -1.f, 0.85f,  0.0f, 0.0f,
         1.f, -1.f, 0.85f,  1.0f, 0.0f,
         1.f,  1.f, 0.85f,  1.0f, 1.0f,
         1.f,  1.f, 0.85f,  1.0f, 1.0f,
        -1.f,  1.f, 0.85f,  0.0f, 1.0f,
        -1.f, -1.f, 0.85f,  0.0f, 0.0f,
    };

    unsigned int VBO_crate, VAO_crate;
    glGenVertexArrays(1, &VAO_crate);
    glGenBuffers(1, &VBO_crate);

    glBindVertexArray(VAO_crate);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_crate);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crate_vertices), crate_vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int VBO_cam, VAO_cam;
    glGenVertexArrays(1, &VAO_cam);
    glGenBuffers(1, &VBO_cam);

    glBindVertexArray(VAO_cam);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_cam);
    glBufferData(GL_ARRAY_BUFFER, sizeof(camera_sprite_vertices), camera_sprite_vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    OpenGL_Texture* tex(new OpenGL_Texture("assets/container.jpg"));
    GLuint textureCam;
    glGenTextures(1, &textureCam);
    
    // Tant que l'on ne ferme pas la fenetre ou que la capture video 
    // est tjs en cours on ne quitte pas
    while (!glfwWindowShouldClose(m_window) && opCV.is_Capture_Open())
    {
        processInput();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shaderProgram->use();

        // la fonction opCV.update() permet d'obtenir deux choses:
        // 1 - Visualiser les carrés détectés par la camera lors de la phase d'init.
        // 2 - Les points de tracking lors de la phase de jeu
        cv::Mat captureWebCam = opCV.update();
        cv::cvtColor(captureWebCam, captureWebCam, cv::COLOR_RGB2BGR);
        flip(captureWebCam, captureWebCam, -1);
        glBindTexture(GL_TEXTURE_2D, textureCam);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, captureWebCam.cols, captureWebCam.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, captureWebCam.ptr());
        glGenerateMipmap(GL_TEXTURE_2D);

        // render cam
        glBindVertexArray(VAO_cam);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // render boxes
        glBindTexture(GL_TEXTURE_2D, tex->getID());
        glBindVertexArray(VAO_crate);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO_crate);
    glDeleteBuffers(1, &VBO_crate);
    glDeleteVertexArrays(1, &VAO_cam);
    glDeleteBuffers(1, &VBO_cam);

    glfwTerminate();
}

void OpenGL_App::processInput() { 
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(m_window, true);
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) opCV.end_init_phase=true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }