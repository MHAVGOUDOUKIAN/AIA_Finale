#ifndef OPENGL_APP_HPP
#define OPENGL_APP_HPP

#include <glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include <OpenGL_Shader.hpp>
#include <OpenGL_Texture.hpp>
#include <OpenGL_Camera.hpp>
#include <OpenCV_Main.hpp>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

class OpenGL_App {
    public:
        OpenGL_App();
        ~OpenGL_App();
        void run();

    private:
        void processInput();
        GLFWwindow* m_window;
        OpenGL_Shader* m_shaderProgram;
        OpenCV_App opCV;
};

#endif
