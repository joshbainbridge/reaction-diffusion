#include <PlatformSpecification.h>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>

#define WIDTH 700
#define HEIGHT 500

static bool update;
static float trans_x, trans_y;
static float scale;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(action == GLFW_PRESS)
  {
    update = true;
    switch(key)
    {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
      case GLFW_KEY_RIGHT:
        trans_x += 0.1f;
        break;
      case GLFW_KEY_LEFT:
        trans_x -= 0.1f;
        break;
      case GLFW_KEY_UP:
        trans_y += 0.1f;
        break;
      case GLFW_KEY_DOWN:
        trans_y -= 0.1f;
        break;
      case GLFW_KEY_X:
        scale += 0.1f;
        break;
      case GLFW_KEY_Z:
        scale -= 0.1f;
        break;
    }
  }
}

GLFWwindow* openglInit()
{
	int glfw_error = glfwInit();
  if(glfw_error != GL_TRUE)
  {
    std::cout << "GLFW error: " << glfw_error << std::endl;
    exit(EXIT_FAILURE);
	}
	
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 2);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Graphics Environment", NULL, NULL);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(GL_FALSE);
	
  GLenum opengl_error = glGetError();
  if(opengl_error != GL_NO_ERROR)
  {
    std::cout << "OpenGL error: " << opengl_error << std::endl;
    exit(EXIT_FAILURE);
  }
  
  #ifdef USING_GLEW
  glewExperimental = GL_TRUE;
  GLenum glew_error = glewInit();
  if(glew_error != GLEW_OK)
  {
    std::cout << "Glew error: " << glew_error << std::endl;
    exit(EXIT_FAILURE);
  }
  #endif
	
	return window;
}

void openglDraw(GLFWwindow* window)
{
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  float vertices[] = {
    -0.5f,  0.5f, 0.f, 1.f,
     0.5f,  0.5f, 1.f, 1.f,
     0.5f, -0.5f, 1.f, 0.f,

     0.5f, -0.5f, 1.f, 0.f,
    -0.5f, -0.5f, 0.f, 0.f,
    -0.5f,  0.5f, 0.f, 1.f
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  const GLchar* vertex_source =
    "#version 330\n"
    "in vec2 position;"
    "in vec2 coordinate;"
    "uniform vec2 translation;"
    "uniform float scale;"
    "out vec3 Color;"
    "out vec2 Coordinate;"
    "void main()"
    "{"
    "   Color = vec3(coordinate, 1.0);"
    "   Coordinate = coordinate;"
    "   gl_Position = vec4(position * scale + translation, 0.0, 1.0);"
    "}";

  const GLchar* fragment_source =
    "#version 330\n"
    "in vec3 Color;"
    "in vec2 Coordinate;"
    "uniform sampler2D tex;"
    "out vec4 outColor;"
    "void main()"
    "{"
    "   outColor = texture(tex, Coordinate) * vec4(Color, 1.0);"
    "}";

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_source, NULL);
  glCompileShader(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_source, NULL);
  glCompileShader(fragment_shader);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  GLint pos_attrib = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(pos_attrib);

  GLint tex_attrib = glGetAttribLocation(shader_program, "coordinate");
  glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
  glEnableVertexAttribArray(tex_attrib);

  GLint trans_uniform = glGetUniformLocation(shader_program, "translation");
  glUniform2f(trans_uniform, trans_x, trans_y);

  GLint scale_uniform = glGetUniformLocation(shader_program, "scale");
  glUniform1f(scale_uniform, scale);

  float *pixels = new float[256*256*3];
  for(int i = 0; i < 196608; ++i)
  {
    pixels[i] = static_cast<float>(i) / 196608;
  }

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_FLOAT, pixels);
  delete[] pixels;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  unsigned int iteration = 0;
	boost::chrono::milliseconds iteration_second( 1000 / 60 );

  while( !glfwWindowShouldClose(window) )
  {
    boost::chrono::high_resolution_clock::time_point timer_start = boost::chrono::high_resolution_clock::now();
  
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glfwSwapBuffers(window);
    glfwPollEvents();

    if(update == true)
    {
      update = false;
      glUniform2f(trans_uniform, trans_x, trans_y);
      glUniform1f(scale_uniform, scale);
    }

    std::string title = "Graphics Environment Iteration: " + std::to_string(++iteration);
    glfwSetWindowTitle(window, title.c_str());

    boost::chrono::high_resolution_clock::time_point timer_end = boost::chrono::high_resolution_clock::now();
    boost::chrono::milliseconds iteration_time(boost::chrono::duration_cast<boost::chrono::milliseconds>(timer_end - timer_start).count());
    if(iteration_time < iteration_second)
    {
      boost::this_thread::sleep_for(iteration_second - iteration_time);
    }    
  }

  glDeleteTextures(1, &texture);
  glDeleteProgram(shader_program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}

int main(void)
{
	GLFWwindow* window = openglInit();

  update = false;
  trans_x = 0.f;
  trans_y = 0.f;
  scale = 1.f;
  
  glfwSetKeyCallback(window, keyCallback);
	
	openglDraw(window);

  glfwDestroyWindow(window);
  glfwTerminate();

  exit(EXIT_SUCCESS);
}