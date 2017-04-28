#pragma once
#include <iostream>

#define SP_CERR( expr ) std::cerr<< expr << ". " << __FUNCTION__ \
		<< ", " << __FILE__<< ", " << __LINE__ << std::endl

#define SP_LOG( expr ) std::cout<< expr <<std::endl

#define FAILED_RETURN 0

#define SHADER_KINDS 3
