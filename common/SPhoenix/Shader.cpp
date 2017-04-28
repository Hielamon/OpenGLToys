#include <SPhoenix/Shader.h>
#include <assert.h>

namespace SP
{
	ShaderCodes::ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath)
	{
		std::string fileName[2] = { vertFilePath, fragFilePath };

		for (size_t i = 0; i < 2; i++)
		{
			std::string &shaderCode = mcodes[i], line = "";
			std::ifstream shaderStream(fileName[i], std::ios::in);
			if (!shaderStream.is_open())
			{
				SP_CERR("Failed to Open File: " + fileName[i]);
				exit(-1);
			}
			while (std::getline(shaderStream, line))
				shaderCode += "\n" + line;
			shaderStream.close();
		}
	}

	ShaderCodes::ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath, const std::string &geomFilePath)
	{
		std::string fileName[3] = { vertFilePath, fragFilePath, geomFilePath};

		for (size_t i = 0; i < 3; i++)
		{
			std::string &shaderCode = mcodes[i], line = "";
			std::ifstream shaderStream(fileName[i], std::ios::in);
			if (!shaderStream.is_open())
			{
				SP_CERR("Failed to Open File: " + fileName[i]);
				exit(-1);
			}
			while (std::getline(shaderStream, line))
				shaderCode += "\n" + line;
			shaderStream.close();
		}
	}

	std::string & ShaderCodes::operator[](size_t idx)
	{
		return mcodes[idx];
	}

	ShaderUtil::ShaderUtil(ShaderCodes &shaderCodes)
	{
		std::vector<GLuint> shaderIDs;

		for (size_t i = 0; i < SHADER_KINDS; i++)
		{
			std::string &code = shaderCodes[i];
			if (!code.empty())
			{
				GLuint shaderID = _createAndCompile(code, (ShaderType)i);
				shaderIDs.push_back(shaderID);
			}
			else
			{
				shaderIDs.push_back(FAILED_RETURN);
			}
		}

		mprogramID = _linkShaders(shaderIDs);
		mopened = mprogramID == FAILED_RETURN ? false : true;
	}

	ShaderUtil::~ShaderUtil()
	{
		if (mopened)
		{
			glDeleteProgram(mprogramID);
		}
	}

	GLuint ShaderUtil::_createAndCompile(const std::string &shaderCode, ShaderType type)
	{
		const char * shaderCodePtr = shaderCode.c_str();

		GLuint shaderID = glCreateShader(type == VERTEX ? GL_VERTEX_SHADER : 
			(type == FRAGMENT ? GL_FRAGMENT_SHADER : GL_GEOMETRY_SHADER));
		
		glShaderSource(shaderID, 1, &shaderCodePtr, NULL);
		glCompileShader(shaderID);

		GLint success;
		GLchar infoLog[512];
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
			SP_CERR("Compile Error log : " + std::string(infoLog));
			glDeleteShader(shaderID);
			exit(-1);
		}
		return shaderID;
	}

	GLuint ShaderUtil::_linkShaders(const std::vector<GLuint> &shaderIDs)
	{
		assert(shaderIDs.size() == SHADER_KINDS);

		GLuint shaderProgram = glCreateProgram();

		for (size_t i = 0; i < SHADER_KINDS; i++)
		{
			if (shaderIDs[i] != 0)
			{
				glAttachShader(shaderProgram, shaderIDs[i]);
			}
		}

		glLinkProgram(shaderProgram);

		GLint success;
		GLchar infoLog[512];
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			SP_CERR("Link Error log :" + std::string(infoLog));
			glDeleteProgram(shaderProgram);
			exit(-1);
		}

		return shaderProgram;
	}

	bool ShaderUtil::isOpen()
	{
		return mopened;
	}

	void ShaderUtil::useProgram()
	{
		glUseProgram(mprogramID);
	}

	GLuint ShaderUtil::getProgramID()
	{
		return mprogramID;
	}
}
