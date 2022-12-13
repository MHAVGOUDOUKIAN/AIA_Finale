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
#include <Constant.hpp>


class OpenGL_App {
    public:
        OpenGL_App();
        ~OpenGL_App();
        void run();

        void build_laby();
        void set_tabl_value(int i, double value);
        double get_tabl_value(int i);

        int tabl_size;

    private:
        void processInput();
        GLFWwindow* m_window;
        OpenGL_Shader* m_shaderProgram;
        OpenCV_App opCV;

        unsigned int VBO_lab, VAO_lab;
        float lab_vertices[2000000];
        bool lab_created;
};

#endif
