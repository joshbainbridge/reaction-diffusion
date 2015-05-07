#include <Framebuffer.h>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <string>

int main(void)
{
  Framebuffer *framebuffer = new Framebuffer();

  framebuffer->init();
  framebuffer->bind();

  unsigned int iteration = 0;
  boost::chrono::milliseconds iteration_second( 1000 / 60 );

  while( !framebuffer->close() )
  {
    boost::chrono::high_resolution_clock::time_point timer_start = boost::chrono::high_resolution_clock::now();
    
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

  delete framebuffer;

  exit(EXIT_SUCCESS);
}