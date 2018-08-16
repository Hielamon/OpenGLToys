#pragma once
#include <iostream>

#define SP_CERR( expr ) std::cerr<< expr << ". " << __FUNCTION__ \
		<< ", " << __FILE__<< ", " << __LINE__ << std::endl

#define SP_LOG( expr ) std::cout<< expr <<std::endl

#define FAILED_RETURN 0

#define SHADER_KINDS 3

#define KEY_COUNT 349
#define MOUSE_COUNT 8

//When using the uniform buffer object
//the UMatrices binging point set to 1
#define VIEWUBO_BINDING_POINT 1
