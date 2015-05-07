#include <Framebuffer.h>
#include <iostream>

Framebuffer::~Framebuffer()
{
  glDeleteTextures(1, &m_texture);
  glDeleteProgram(m_shader_program);
  glDeleteShader(m_vertex_shader);
  glDeleteShader(m_fragment_shader);
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);

  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void Framebuffer::init()
{
  createContext();
  createSurface();

  glfwSetKeyCallback(m_window, keyCallback);
}

void Framebuffer::bind()
{
  glfwMakeContextCurrent(m_window);
  glUseProgram(m_shader_program);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Framebuffer::draw()
{
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  
  glfwSwapBuffers(m_window);
  glfwPollEvents();
}

void Framebuffer::title(const std::string &_title)
{
  glfwSetWindowTitle(window, _title.c_str());
}

void Framebuffer::createSurface()
{
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  float vertices[] = {
    -0.5f,  0.5f, 0.f, 1.f,
     0.5f,  0.5f, 1.f, 1.f,
     0.5f, -0.5f, 1.f, 0.f,

     0.5f, -0.5f, 1.f, 0.f,
    -0.5f, -0.5f, 0.f, 0.f,
    -0.5f,  0.5f, 0.f, 1.f
  };

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
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

  m_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(m_vertex_shader, 1, &vertex_source, NULL);
  glCompileShader(m_vertex_shader);

  m_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(m_fragment_shader, 1, &fragment_source, NULL);
  glCompileShader(m_fragment_shader);

  m_shader_program = glCreateProgram();
  glAttachShader(m_shader_program, m_vertex_shader);
  glAttachShader(m_shader_program, m_fragment_shader);
  glLinkProgram(m_shader_program);
  glUseProgram(m_shader_program);

  m_pos_attrib = glGetAttribLocation(m_shader_program, "position");
  glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(pos_attrib);

  m_tex_attrib = glGetAttribLocation(m_shader_program, "coordinate");
  glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
  glEnableVertexAttribArray(tex_attrib);

  m_trans_uniform = glGetUniformLocation(m_shader_program, "translation");
  glUniform2f(trans_uniform, m_trans_x, m_trans_y);

  m_scale_uniform = glGetUniformLocation(m_shader_program, "scale");
  glUniform1f(scale_uniform, m_scale);

  float *pixels = new float[256*256*3];
  for(int i = 0; i < 196608; ++i)
  {
    pixels[i] = static_cast<float>(i) / 196608;
  }

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_FLOAT, pixels);
  delete[] pixels;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
}

void Framebuffer::createContext()
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
  
  m_window = glfwCreateWindow(WIDTH, HEIGHT, "Graphics Environment", NULL, NULL);

  glfwMakeContextCurrent(m_window);
  glfwSwapInterval(false);
  
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
}

void Framebuffer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(action == GLFW_PRESS)
  {
    m_update = true;
    switch(key)
    {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(m_window, GL_TRUE);
        break;
      case GLFW_KEY_RIGHT:
        m_trans_x += 0.1f;
        break;
      case GLFW_KEY_LEFT:
        m_trans_x -= 0.1f;
        break;
      case GLFW_KEY_UP:
        m_trans_y += 0.1f;
        break;
      case GLFW_KEY_DOWN:
        m_trans_y -= 0.1f;
        break;
      case GLFW_KEY_X:
        m_scale += 0.1f;
        break;
      case GLFW_KEY_Z:
        m_scale -= 0.1f;
        break;
    }
  }
}