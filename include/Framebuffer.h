#ifndef FRAMEBUFFER_H__
	#define FRAMEBUFFER_H__
	
  #include <PlatformSpecification.h>
  #include <string>

	class Framebuffer
	{
	public:
		Framebuffer()
      : m_update(true)
      , m_trans_x(0.f)
      , m_trans_y(0.f)
      , m_scale(1.f)
    {;}

    ~Framebuffer();
    void init();
    void bind();
    void draw();
    bool close();
    void title(const std::string &_title);

	private:
    GLFWwindow* m_window;
    GLuint m_texture;
    GLuint m_shader_program;
    GLuint m_vertex_shader;
    GLuint m_fragment_shader;
    GLuint m_vbo;
    GLuint m_vao;
    GLint m_pos_attrib;
    GLint m_tex_attrib;
    GLint m_trans_uniform;
    GLint m_scale_uniform;

    void createContext();
    void createSurface();
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  public:
    bool m_update;
    float m_trans_x, m_trans_y;
    float m_scale;
	};
	
#endif