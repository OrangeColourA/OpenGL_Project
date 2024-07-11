#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


//macros

#define ASSERT(x) if (!(x)) __debugbreak();
#define GL_Call(x) GL_ClearError();\
    x;\
    ASSERT(GL_LogCall(#x, __FILE__, __LINE__))

// openGL error handling functions using glGetError

static void GL_ClearError()
{
    while (glGetError());
}

static bool GL_LogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL error] : " << error <<  std::endl;
        std::cout << "file : " << file << std::endl;
        std::cout << "function : " << function << std::endl;
        std::cout << "line : " << line << std::endl;
        return false;
    }

    return true;

}

struct ShaderSourceCode
{
    std::string vertexShader;
    std::string fragmentShader;
};

//parsing a file with shaders source code into different shader types
static ShaderSourceCode ShaderSourceParsing(std::string filename)
{
    enum class ShaderType {
        NONE_SHADER = -1, VERTEX_SHADER = 0, FRAGMENT_SHADER = 1
    };

    std::ifstream source(filename);
    std::string line;

    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE_SHADER;

    while( std::getline(source,line) )
    { 
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX_SHADER;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT_SHADER;
            }
        }
        else
        {
            ss[static_cast<int>(type)] << line << '\n';
        }
    
    }


    return { ss[0].str(), ss[1].str()};
}


//Creating a shader
//it needs 2 source code files or programs
//these are vertexShader (creating object?) and fragmentShader (deciding which pixel to draw and what color)
//and now we will create them as a strings
//we make function static because?
//fucntion is reteurning id, same as with vertexBuffer


static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);

    //in next variable we will store our code
    //BE SURE that everything is OK with source, because we are
    //passing it by a reference
    const char* src = source.c_str();

    //next we are setting this source to our shader
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //handling compile error
    int error_type;
    glGetShaderiv(id, GL_COMPILE_STATUS, &error_type);

    if (error_type == GL_FALSE)
    {
        //ERROR HANDLING
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        // alloca is a function that allocates mamory on a stack dynamicaly

        char* msg = (char*)(alloca(length * sizeof(char)));
        glGetShaderInfoLog(id, length, &length, msg);


        std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex " : "fragment") 
                                         << "shader" << std::endl;
        std::cout << msg << std::endl;
        glDeleteShader(id);

        return 0;
    
    }

    return id;

}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    // we are creating a program that contains our code
    // that will be stored as id in unsigned int variable
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}



int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);
    

    if (glewInit() != GLEW_OK)
    {
        std::cout << "ERROR" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;


    //initialise floats buffer

    float positions[] = {
        -0.5f, -0.5f, // 0
         0.5f, -0.5f, // 1
         0.5f,  0.5f, // 2
        -0.5f,  0.5f  // 3
    };

    unsigned int indicies[] = {
        0, 1, 2,
        2, 3, 0
    };

    // initialise ID of vertex buffer
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    unsigned int index_buffer;
    GL_Call(glGenBuffers(1, &index_buffer));
    GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer));
    GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indicies, GL_STATIC_DRAW));

    ShaderSourceCode source = ShaderSourceParsing("res/shaders/Basic.shader");

    /*std::cout << source.vertexShader << std::endl;
    std::cout << source.fragmentShader << std::endl;*/

    unsigned int shader = CreateShader(source.vertexShader, source.fragmentShader);
    GL_Call(glUseProgram(shader));

    //setting up uniform for different colors for each fragment

    GL_Call(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);
    GL_Call(glUniform4f(location, 0.5f, 1.0f, 0.8f, 1.0f));


    float red = 0.5f;
    float incr = 0.05f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GL_Call(glClear(GL_COLOR_BUFFER_BIT));

        GL_Call(glUniform4f(location, red, .3f, 0.8f, 1.0f));
        GL_Call(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        

        if (red > 1.0f)
        {
            incr = -0.05f;
        }
        else if (red < 0.0f)
        {
            incr = 0.05f;
        }

        red += incr;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}