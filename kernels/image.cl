struct InputData
{
  float Da;
  float Db;
  float f;
  float k;
  float delta;
};

static int mod(const int _a, const int _b)
{
  int value = _a % _b;
  if (value < 0)
    value += _b;
  return value;
}

static float laplacian(__global float* _array, const int _point, const int _width, const int _height)
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

__kernel void square(
  __global float* a_current,
  __global float* b_current,
  __global float* a_buffer,
  __global float* b_buffer,
  struct InputData input,
  float width,
  float height)
{
  size_t i = get_global_id(0);

  float reaction = a_buffer[i] * (b_buffer[i] * b_buffer[i]);

  a_current[i] = a_buffer[i] + (input.Da * laplacian(a_buffer, i, width, height) - reaction + input.f * (1.f - a_buffer[i])) * input.delta;
  b_current[i] = b_buffer[i] + (input.Db * laplacian(b_buffer, i, width, height) + reaction - (input.k + input.f) * b_buffer[i]) * input.delta;
}