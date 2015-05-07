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
    void title(const std::string &_title);

	private:
		bool m_update;
		float m_trans_x, m_trans_y;
		float m_scale;

    GLFWwindow* m_window;
    GLunit m_texture;
    GLunit m_shader_program;
    GLunit m_vertex_shader;
    GLunit m_fragment_shader;
    GLunit m_vbo;
    GLunit m_vao;
    GLint m_pos_attrib;
    Glint m_tex_attrib;
    Glint m_trans_uniform;
    Glint m_scale_uniform;

    void createContext();
    void createSurface();
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	};
	
#endif