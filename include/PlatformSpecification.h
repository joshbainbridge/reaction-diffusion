#ifndef PLATFORM_SPECIFICATION_H__
	#define PLATFORM_SPECIFICATION_H__
	
	#ifdef __APPLE__
    #include "OpenCL/opencl.h"
		#define GLFW_INCLUDE_GLCOREARB
		#include <GLFW/glfw3.h>
	#elif __linux__
    #include "CL/cl.h"
		#define USING_GLEW
    #define GLEW_STATIC
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
	#elif _WIN32
    #include "CL/cl.h"
		#define USING_GLEW
		#define GLEW_STATIC
		#include <GL/glew.h>
		#include <GLFW/glfw3.h>
	#endif
	
#endif