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
#define DEFAULT_SHININESS_STRENGTH 0.5f

//When using the uniform buffer object
//the VIEWUBO binging point set to 1
#define VIEWUBO_BINDING_POINT 1
//the PROJUBO binging point set to 2
#define PROJUBO_BINDING_POINT 2

#define MAX_VIEWPORT_SIZE 32768

#define BOUNDING_BOX_PADDING 0.01
#define BOUNDINT_BOX_POINT_NUM 8

#define TEXT_CHAR_NUM 128


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
