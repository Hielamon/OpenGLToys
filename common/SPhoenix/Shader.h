#pragma once

#include "utils.h"

namespace SP
{
	enum ShaderType
	{
		VERTEX, FRAGMENT, GEOMETRY
	};

	/**The warper class containing max all three shader codes*/
	class ShaderCodes
	{
	public:
		ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath)
		{
			std::vector<std::string> fileNames = { vertFilePath, fragFilePath };

			_initCommon(fileNames);
		}
		ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath, const std::string &geomFilePath)
		{
			std::vector<std::string> fileNames = { vertFilePath, fragFilePath, geomFilePath };

			_initCommon(fileNames);
		}
		~ShaderCodes(){}

	protected:
		ShaderCodes()
		{
			std::vector<std::string> fileNames;
			_initCommon(fileNames);
		}
		std::vector<std::shared_ptr<std::string>> mcodes;

	private:
		void _initCommon(std::vector<std::string> &filenames)
		{
			mcodes.resize(SHADER_KINDS);

			for (size_t i = 0; i < filenames.size(); i++)
			{
				mcodes[i] = std::make_shared<std::string>();
				std::string &shaderCode = *mcodes[i], line = "";
				std::ifstream shaderStream(filenames[i], std::ios::in);
				if (!shaderStream.is_open())
				{
					SP_CERR("Failed to Open File: " + filenames[i]);
					exit(-1);
				}
				while (std::getline(shaderStream, line))
					shaderCode += "\n" + line;
				shaderStream.close();
			}
		}
	};

	/**Create , Compile Shaders and Give interface to access the programID*/
	class ShaderUtil : public ShaderCodes
	{
	public:
		ShaderUtil(ShaderCodes &shaderCodes) : ShaderCodes(shaderCodes)
		{
			std::vector<GLuint> shaderIDs;

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				if (mcodes[i] != nullptr)
				{
					GLuint shaderID = _createAndCompile(*mcodes[i], (ShaderType)i);
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
		ShaderUtil() {}
		~ShaderUtil()
		{
			if (mopened)
			{
				glDeleteProgram(mprogramID);
			}
		}
		bool isOpen()
		{
			return mopened;
		}
		void useProgram()
		{
			glUseProgram(mprogramID);
		}
		GLuint getProgramID()
		{
			return mprogramID;
		}

	private:
		GLuint _createAndCompile(const std::string &shaderCode, ShaderType type)
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
		GLuint _linkShaders(const std::vector<GLuint> &shaderIDs)
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

		GLuint mprogramID;
		bool mopened;
	};
}

