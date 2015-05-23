#include <PlatformSpecification.h>
#include <Framebuffer.h>
#include <Perlin.h>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/random.hpp>
#include <fstream>
#include <string>

#define WIDTH 700
#define HEIGHT 500
#define SIZE WIDTH * HEIGHT

struct InputData
{
  float Da;
  float Db;
  float f;
  float k;
  float delta;
};

struct SimData
{
  float a_current[SIZE];
  float b_current[SIZE];
  float a_buffer[SIZE];
  float b_buffer[SIZE];
};

std::string read_file(const char _filepath[])
{
  std::string output;
  std::ifstream file(_filepath);

  if(file.is_open())
  {
    std::string line;
    while(!file.eof())
    {
      std::getline(file, line);
      output.append(line + "\n");
    }
  }
  else
  {
    std::cout << "File could not be opened." << std::endl;
    exit(EXIT_FAILURE);
  }

  file.close();
  return output;
}

void opencl_error_check(const cl_int _error)
{
  if(_error != CL_SUCCESS)
  {
    std::cout << "OpenCL error: " << _error << std::endl;
    exit(EXIT_FAILURE);
  }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  InputData *input = static_cast<InputData*>(glfwGetWindowUserPointer(window));

  if(action == GLFW_PRESS)
  {
    switch(key)
    {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
      case GLFW_KEY_Q:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
      case GLFW_KEY_A:
        input->Da *= 1.25f;
        std::cout << input->Da << std::endl;
        break;
      case GLFW_KEY_Z:
        input->Da *= 0.8f;
        std::cout << input->Da << std::endl;
        break;
      case GLFW_KEY_S:
        input->Db *= 1.25f;
        std::cout << input->Db << std::endl;
        break;
      case GLFW_KEY_X:
        input->Db *= 0.8f;
        std::cout << input->Db << std::endl;
        break;
      case GLFW_KEY_D:
        input->f *= 1.25f;
        std::cout << "f = " << input->f << std::endl;
        break;
      case GLFW_KEY_C:
        input->f *= 0.8f;
        std::cout << "f = " << input->f << std::endl;
        break;
      case GLFW_KEY_F:
        input->k *= 1.25f;
        std::cout << "k = " << input->k << std::endl;
        break;
      case GLFW_KEY_V:
        input->k *= 0.8f;
        std::cout << "k = " << input->k << std::endl;
        break;
    }
  }
}

int main(int argc, char const *argv[])
{
  InputData input;
  input.Da = 1.f;
  input.Db = 0.5f;
  input.f = 0.018f;
  input.k = 0.051f;
  input.delta = 0.8f;

  Framebuffer *framebuffer = new Framebuffer();

  GLFWwindow* window = framebuffer->init(WIDTH, HEIGHT, &input);
  glfwSetKeyCallback(window, keyCallback);

  SimData *data = new SimData;
  float *image = new float[SIZE*3];

  Perlin perlin;
  for(int i = 0; i < SIZE; ++i)
  {
    float xpos = i % WIDTH;
    float ypos = i / WIDTH;

    if(perlin.noise(xpos / 100, ypos / 100, 0) > 0.4f)
    {
      data->a_current[i] = 1.f;
      data->b_current[i] = 1.f;
    }
    else
    {
      data->a_current[i] = 1.f;
      data->b_current[i] = 0.f;
    }

    data->a_buffer[i] = 0.f;
    data->b_buffer[i] = 0.f;
  }

  // OpenCL setup starts here

  // Setup
  cl_platform_id platform_id;
  clGetPlatformIDs(1, &platform_id, NULL);
  cl_uint device_count;
  clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &device_count);
  cl_device_id *device_ids = new cl_device_id[device_count];
  clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, device_count, device_ids, NULL);

  // Error code
  cl_int error = CL_SUCCESS;

  // Context creation
  const cl_context_properties context_properties[] = {
    CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platform_id),
    0
  };
  cl_context context = clCreateContext(context_properties, device_count, device_ids, NULL, NULL, &error);
  opencl_error_check(error);

  // GLuint m_texture;
  // glGenTextures(1, &m_texture);
  // glBindTexture(GL_TEXTURE_2D, m_texture);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

  // Memory allocation
  cl_mem cMem_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * SIZE, data->a_current, &error);
  opencl_error_check(error);
  cl_mem cMem_b = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * SIZE, data->b_current, &error);
  opencl_error_check(error);
  cl_mem bMem_a = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * SIZE, data->a_buffer, &error);
  opencl_error_check(error);
  cl_mem bMem_b = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * SIZE, data->b_buffer, &error);
  opencl_error_check(error);
  // cl_mem mem = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, m_texture, &error);
  // opencl_error_check(error);

  // Load source file
  std::string file = read_file("kernels/image.cl");
  const char* source[] = {file.c_str()};

  // Build program
  cl_program program = clCreateProgramWithSource(context, 1, source, NULL, &error);
  opencl_error_check(error);
  error = clBuildProgram(program, device_count, device_ids, NULL, NULL, NULL);
  if(error != CL_SUCCESS)
  {
    char buffer[1024];
    clGetProgramBuildInfo(program, device_ids[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
    std::cout << buffer << std::endl;
    exit(EXIT_FAILURE);
  }

  // Resolution for kernel
  const float width = WIDTH;
  const float height = HEIGHT;

  // Create kernel one
  cl_kernel kernel_one = clCreateKernel(program, "square", &error);
  opencl_error_check(error);

  // Set argurments for kernel one
  clSetKernelArg(kernel_one, 0, sizeof(cl_mem), &cMem_a);
  clSetKernelArg(kernel_one, 1, sizeof(cl_mem), &cMem_b);
  clSetKernelArg(kernel_one, 2, sizeof(cl_mem), &bMem_a);
  clSetKernelArg(kernel_one, 3, sizeof(cl_mem), &bMem_b);
  clSetKernelArg(kernel_one, 4, sizeof(InputData), &input);
  clSetKernelArg(kernel_one, 5, sizeof(float), &width);
  clSetKernelArg(kernel_one, 6, sizeof(float), &height);

  // Create kernel two
  cl_kernel kernel_two = clCreateKernel(program, "square", &error);
  opencl_error_check(error);

  // Set argurments for kernel two
  clSetKernelArg(kernel_two, 0, sizeof(cl_mem), &bMem_a);
  clSetKernelArg(kernel_two, 1, sizeof(cl_mem), &bMem_b);
  clSetKernelArg(kernel_two, 2, sizeof(cl_mem), &cMem_a);
  clSetKernelArg(kernel_two, 3, sizeof(cl_mem), &cMem_b);
  clSetKernelArg(kernel_two, 4, sizeof(InputData), &input);
  clSetKernelArg(kernel_two, 5, sizeof(float), &width);
  clSetKernelArg(kernel_two, 6, sizeof(float), &height);

  // Create queue
  cl_command_queue queue = clCreateCommandQueue(context, device_ids[0], 0, &error);
  opencl_error_check(error);

  // OpenCL setup ends here

  framebuffer->bind();
  unsigned int iteration = 0;
  boost::chrono::milliseconds iteration_delta(static_cast<int>((1000.f / 60.f) * input.delta));

  while( !framebuffer->close() )
  {
    boost::chrono::high_resolution_clock::time_point timer_start = boost::chrono::high_resolution_clock::now();

    // Run queue
    std::size_t size[] = {SIZE};
    error = clEnqueueNDRangeKernel(queue, (iteration % 2) ? kernel_one : kernel_two, 1, 0, size, NULL, 0, NULL, NULL);
    opencl_error_check(error);

    // Get data from GPU
    error = clEnqueueReadBuffer(queue, cMem_a, CL_TRUE, 0, sizeof(float) * SIZE, data->a_current, 0, NULL, NULL);
    opencl_error_check(error);

    for(int i = 0; i < SIZE; ++i)
    {
      float current = image[i * 3 + 0];
      image[i * 3 + 0] = data->a_current[i];
      image[i * 3 + 1] = data->a_current[i];
      image[i * 3 + 2] = data->a_current[i];
    }

    framebuffer->image(image, WIDTH, HEIGHT);
    framebuffer->draw();

    std::string title = "Graphics Environment Iteration: " + std::to_string(++iteration);
    framebuffer->title(title);

    boost::chrono::high_resolution_clock::time_point timer_end = boost::chrono::high_resolution_clock::now();
    boost::chrono::milliseconds iteration_time(boost::chrono::duration_cast<boost::chrono::milliseconds>(timer_end - timer_start).count());
    if(iteration_time < iteration_delta)
    {
      boost::this_thread::sleep_for(iteration_delta - iteration_time);
    }    
  }

  // Cleanup
  clReleaseCommandQueue(queue);
  clReleaseMemObject(bMem_b);
  clReleaseMemObject(bMem_a);
  clReleaseMemObject(cMem_b);
  clReleaseMemObject(cMem_a);
  clReleaseKernel(kernel_two);
  clReleaseKernel(kernel_one);
  clReleaseProgram(program);
  clReleaseContext(context);
  delete[] device_ids;

  delete data;
  delete framebuffer;
  delete[] image;

  exit(EXIT_SUCCESS);
}