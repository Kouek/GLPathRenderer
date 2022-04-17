#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <GLPathRenderer.h>


#define GL_CHECK \
         {\
            GLenum gl_err;\
            if((gl_err=glGetError())!=GL_NO_ERROR){\
				printf("Line: %d, ErrCode: %x\n",static_cast<unsigned int>(gl_err), __LINE__);\
            }\
         }

constexpr glm::uvec2 windowSize{ 800,800 };
constexpr glm::vec2 clips{ .1f,10.f };
constexpr float fov = 60.f;
constexpr float dashboardZ = -1.f;

struct Shaders
{
    struct
    {
        GLuint program;
        GLint matPos, colPos;
    }colorShader;
};
static Shaders createShaders();

static void onFramebufferSizeChanged(GLFWwindow* window, int w, int h);
static void onMousePressed(GLFWwindow* window, int button, int action, int mods);
static void onMouseMoved(GLFWwindow* window, double x, double y);

static kouek::GLPathRenderer* pathRendererGlbPtr = nullptr;

int main()
{
    assert(windowSize.x == windowSize.y);

    // GLFW context, use OpenlGL 4.4
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        windowSize.x, windowSize.y, "LearnOpenGL", nullptr, nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onFramebufferSizeChanged);
    glfwSetMouseButtonCallback(window, onMousePressed);
    glfwSetCursorPosCallback(window, onMouseMoved);

    // Load compatible OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Init shaders
    Shaders shaders = createShaders();

    // Init transform
    glm::mat4 VP = glm::perspectiveFov(
        glm::radians(fov),
        (float)windowSize.x, (float)windowSize.y,
        clips.x, clips.y);

    // Render Loop
    kouek::GLPathRenderer pathRenderer;
    pathRendererGlbPtr = &pathRenderer;
    {
        GLuint pathID = pathRenderer.addPath(
            glm::vec3{ 1.f, 1.f, 1.f }, glm::vec3{ 0, 0, dashboardZ });
        pathRenderer.startPath(pathID);
        //GLuint id = pathRenderer.addSubPath();
        //pathRenderer.startSubPath(id);
        //id = pathRenderer.addVertex(glm::vec3{ .5f, .5f, dashboardZ });
        //pathRenderer.startVertex(id);
        //id = pathRenderer.addVertex(glm::vec3{ .7f, .3f, dashboardZ });
        //pathRenderer.startVertex(id);
        //pathRenderer.endSubPath();
    }
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0, 0, 0, .1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaders.colorShader.program);
        glUniformMatrix4fv(shaders.colorShader.matPos,
            1, GL_FALSE, (const float*)&VP);
        pathRenderer.draw(shaders.colorShader.colPos);

        glfwSwapBuffers(window);
    }

	return 0;
}

Shaders createShaders()
{
    Shaders shaders;
    // colorShader
    {
        const char* vertShaderCode =
            "#version 410 core\n"
            "uniform mat4 matrix;\n"
            "layout(location = 0) in vec3 position;\n"
            "layout(location = 1) in vec3 id;\n"
            "void main()\n"
            "{\n"
            "	gl_Position = matrix * vec4(position.xyz, 1.0);\n"
            "}\n";
        const char* fragShaderCode =
            "#version 410 core\n"
            "uniform vec3 color;\n"
            "out vec4 outputColor;\n"
            "void main()\n"
            "{\n"
            "    outputColor = vec4(color, 1.0);\n"
            "}\n";
        
        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vertShaderCode, nullptr);
        glCompileShader(vertShader);
        GLint success;
        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
        assert(success);

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fragShaderCode, nullptr);
        glCompileShader(fragShader);
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        assert(success);

        shaders.colorShader.program = glCreateProgram();
        glAttachShader(shaders.colorShader.program, vertShader);
        glAttachShader(shaders.colorShader.program, fragShader);
        glLinkProgram(shaders.colorShader.program);
        glGetProgramiv(shaders.colorShader.program, GL_LINK_STATUS, &success);
        assert(success);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        shaders.colorShader.matPos = glGetUniformLocation(
            shaders.colorShader.program, "matrix");
        shaders.colorShader.colPos = glGetUniformLocation(
            shaders.colorShader.program, "color");
        assert(shaders.colorShader.matPos != -1);
        assert(shaders.colorShader.colPos != -1);
    }
    return shaders;
}

void onFramebufferSizeChanged(GLFWwindow* window, int w, int h)
{
    
}

static bool pressed = false;
void onMousePressed(GLFWwindow* window, int button, int action, int mods)
{
    static auto lastAction = GLFW_RELEASE;
    if (lastAction == GLFW_PRESS && action == GLFW_RELEASE)
    {
        pressed = false;
        pathRendererGlbPtr->endSubPath();
    }
    else if (lastAction == GLFW_RELEASE && action == GLFW_PRESS)
    {
        pressed = true;
        GLuint id = pathRendererGlbPtr->addSubPath();
        pathRendererGlbPtr->startSubPath(id);
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        onMouseMoved(window, x, y);
    }
    lastAction = action;
}

void onMouseMoved(GLFWwindow* window, double x, double y)
{
    if (pressed)
    {
        static double screenToWorld = 2.0 * tanf(glm::radians(.5f * fov))
            / windowSize.y
            * (double)fabsf(dashboardZ);
        static glm::vec3 lastPos{ 0,0,dashboardZ };
        y = windowSize.y - y;
        glm::vec3 pos{
            (x - windowSize.x / 2) * screenToWorld,
            (y - windowSize.y / 2) * screenToWorld,
            dashboardZ };
        glm::vec3 dist = pos - lastPos;
        if (glm::dot(dist, dist) >= kouek::GLPathRenderer::minDistSqrBtwnVerts)
        {
            GLuint id = pathRendererGlbPtr->addVertex(pos);
            pathRendererGlbPtr->startVertex(id);
            lastPos = pos;
        }
    }
}
