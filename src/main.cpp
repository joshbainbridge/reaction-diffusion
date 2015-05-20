#include <PlatformSpecification.h>
#include <Framebuffer.h>
#include <Perlin.h>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/random.hpp>
#include <string>
#include <math.h>

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

int mod(const int _a, const int _b)
{
  int value = _a % _b;
  if (value < 0)
    value += _b;
  return value;
}

float laplacian(const float* _array, const int _point, const int _width, const int _height)
{
  int xpos = _point % _width;
  int ypos = _point / _width;
  int positive_x = mod(xpos + 1, _width);
  int negative_x = mod(xpos - 1, _width);
  int positive_y = mod(ypos + 1, _height) * _width;
  int negative_y = mod(ypos - 1, _height) * _width;

  int index[] = {
    negative_x + positive_y, xpos + positive_y, positive_x + positive_y,
    negative_x + ypos * _width, xpos + ypos * _width, positive_x + ypos * _width,
    negative_x + negative_y, xpos + negative_y, positive_x + negative_y
  };

  float weight[] = {
    0.05f, 0.2f, 0.05f,
    0.2f, -1.f, 0.2f,
    0.05f, 0.2f, 0.05f
  };

  float output = _array[index[0]] * weight[0] + _array[index[1]] * weight[1] + _array[index[2]] * weight[2]
  + _array[index[3]] * weight[3] + _array[index[4]] * weight[4] + _array[index[5]] * weight[5]
  + _array[index[6]] * weight[6] + _array[index[7]] * weight[7] + _array[index[8]] * weight[8];

  return output;
}

// float laplacian(const float* _array, const int _point, const int _width, const int _height)
// {
//   int xpos = _point % _width;
//   int ypos = _point / _width;
//   int positive_x = mod(xpos + 1, _width) + (ypos * _width);
//   int negative_x = mod(xpos - 1, _width) + (ypos * _width);
//   int positive_y = xpos + mod(ypos + 1, _height) * _width;
//   int negative_y = xpos + mod(ypos - 1, _height) * _width;
//   return _array[positive_x] + _array[negative_x] + _array[positive_y] + _array[negative_y] - 4 * _array[_point];
// }

int main(void)
{
  Perlin perlin;

  InputData input;
  input.Da = 1.f;
  input.Db = 0.5f;
  input.f = 0.055f;
  input.k = 0.062f;
  input.delta = 1.f;

  Framebuffer *framebuffer = new Framebuffer();

  GLFWwindow* window = framebuffer->init(WIDTH, HEIGHT, &input);
  glfwSetKeyCallback(window, keyCallback);

  boost::mt19937 generator;
  boost::uniform_real<float> uniform_dist;

  SimData *data = new SimData;
  for(int i = 0; i < SIZE; ++i)
  {
    float xpos = i % WIDTH;
    float ypos = i / WIDTH;
    if(perlin.noise(xpos / 100, ypos / 100, 0) > 0.3f)
    {
      data->a_current[i] = 1.f;
      data->b_current[i] = 1.f;
    }
    else
    {
      data->a_current[i] = 1.f;
      data->b_current[i] = 0.f;
    }
  }

  float *image = new float[SIZE*3];
  for(int i = 0; i < SIZE; ++i)
  {
    image[i * 3 + 0] = data->a_current[i];
    image[i * 3 + 1] = data->a_current[i];
    image[i * 3 + 2] = data->a_current[i];
  }

  framebuffer->image(image, WIDTH, HEIGHT);
  framebuffer->bind();

  unsigned int iteration = 0;
  boost::chrono::milliseconds iteration_second( 1000 / 60 );

  while( !framebuffer->close() )
  {
    boost::chrono::high_resolution_clock::time_point timer_start = boost::chrono::high_resolution_clock::now();

    for(int i = 0; i < SIZE; ++i)
    {
      data->a_buffer[i] = data->a_current[i];
      data->b_buffer[i] = data->b_current[i];

      float reaction = data->a_buffer[i] * (data->b_buffer[i] * data->b_buffer[i]);
      data->a_current[i] = data->a_buffer[i] + (input.Da * laplacian(data->a_buffer, i, WIDTH, HEIGHT) - reaction + input.f * (1.f - data->a_buffer[i])) * input.delta;
      data->b_current[i] = data->b_buffer[i] + (input.Db * laplacian(data->b_buffer, i, WIDTH, HEIGHT) + reaction - (input.k + input.f) * data->b_buffer[i]) * input.delta;
    }

    for(int i = 0; i < SIZE; ++i)
    {
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
    if(iteration_time < iteration_second)
    {
      boost::this_thread::sleep_for(iteration_second - iteration_time);
    }    
  }

  delete data;
  delete framebuffer;
  delete[] image;

  exit(EXIT_SUCCESS);
}