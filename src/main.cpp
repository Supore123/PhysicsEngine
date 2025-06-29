#include <GLFW/glfw3.h>
#include <iostream>

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(800, 600, "Simple Physics Engine", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
