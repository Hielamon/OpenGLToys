#pragma once
#include <iostream>

#define SP_CERR( expr ) std::cerr<< expr << ". " << __FUNCTION__ \
		<< ", " << __FILE__<< ", " << __LINE__ << std::endl

#define SP_LOG( expr ) std::cout<< expr <<std::endl

#define FAILED_RETURN 0

#define SHADER_KINDS 3

#define KEY_COUNT 349
#define MOUSE_COUNT 8

#define DEFAULT_SHININESS 32.0f

//When using the uniform buffer object
//the UMatrices binging point set to 1
#define VIEWUBO_BINDING_POINT 1

#define MAX_VIEWPORT_SIZE 32768



#define GL_DEBUG_INVALID_ENUM \
if (glGetError() == GL_INVALID_ENUM)\
{\
	std::cout << " ERROR : GL_INVALID_ENUM " << std::endl;\
}

#define GL_DEBUG_INVALID_OPERATION \
if (glGetError() == GL_INVALID_OPERATION)\
{\
	std::cout << " ERROR : GL_INVALID_OPERATION " << std::endl;\
}

#define GL_DEBUG_NO_ERROR \
if (glGetError() == GL_NO_ERROR)\
{\
	std::cout << " ERROR : GL_NO_ERROR " << std::endl;\
}

#define GL_DEBUG_INVALID_ENUM \
if (glGetError() == GL_INVALID_ENUM)\
{\
	std::cout << " ERROR : GL_INVALID_ENUM " << std::endl;\
}

#define GL_DEBUG_ALL \
GL_DEBUG_INVALID_ENUM \
GL_DEBUG_INVALID_OPERATION \
GL_DEBUG_NO_ERROR \
GL_DEBUG_INVALID_ENUM
/*GLenum error = glGetError();
if (error == GL_NO_ERROR)
{
	std::cout << " ERROR :  GL_NO_ERROR" << std::endl;
}*/
