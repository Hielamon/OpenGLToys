#pragma once

#include "Utils.h"

namespace SP
{
	class ShaderUtil;

	enum ShaderType
	{
		VERTEX, FRAGMENT, GEOMETRY
	};

	/**The warper class containing max all three shader codes*/
	class ShaderCodes
	{
	public:
		ShaderCodes() = delete;

		ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath)
		{
			_readFromFile(vertFilePath, ShaderType::VERTEX);
			_readFromFile(fragFilePath, ShaderType::FRAGMENT);
		}

		ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath, const std::string &geomFilePath)
		{
			_readFromFile(vertFilePath, ShaderType::VERTEX);
			_readFromFile(fragFilePath, ShaderType::FRAGMENT);
			_readFromFile(geomFilePath, ShaderType::GEOMETRY);
		}

		void addMacros(const std::string &macros)
		{
			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				std::string &code = mcodes[i];
				if (!code.empty())
				{
					int start = code.find("#version");
					int insertPos = code.find("\n", start);
					code.insert(insertPos + 2, macros);
				}
			}
		}

		~ShaderCodes(){}

		friend class ShaderUtil;

	protected:
		std::string mcodes[SHADER_KINDS];

	private:
		bool _readFromFile(const std::string &filename, ShaderType type)
		{
			std::string &shaderCode = mcodes[type], line = "";
			std::ifstream shaderStream(filename, std::ios::in);
			if (!shaderStream.is_open())
			{
				SP_CERR("Failed to Open File: " + filename);
				exit(-1);
			}
			shaderCode = "";

			while (std::getline(shaderStream, line))
				shaderCode += line + "\n";

			shaderStream.close();
			return true;
		}
	};

	/**Create , Compile Shaders and Give interface to access the programID*/
	class ShaderUtil
	{
	public:
		ShaderUtil() = delete;

		ShaderUtil(std::shared_ptr<ShaderCodes> &pShaderCodes) 
			: mpShaderCodes(pShaderCodes)
		{
			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				if (!pShaderCodes->mcodes[i].empty())
				{
					mvshaderID[i] = _createAndCompile(pShaderCodes->mcodes[i], (ShaderType)i);
				}
				else
				{
					mvshaderID[i] = FAILED_RETURN;
				}
			}

			mprogramID = _linkShaders(mvshaderID);
			mopened = mprogramID == FAILED_RETURN ? false : true;
		}

		~ShaderUtil()
		{
			if (mopened)
			{
				glDeleteProgram(mprogramID);
			}

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				if (mvshaderID[i] != FAILED_RETURN)
				{
					glDeleteShader(mvshaderID[i]);
				}
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

	protected:
		std::shared_ptr<ShaderCodes> mpShaderCodes;

		GLuint mvshaderID[SHADER_KINDS];
		GLuint mprogramID;
		bool mopened;

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

		GLuint _linkShaders(GLuint (&vshaderID)[SHADER_KINDS] )
		{
			GLuint shaderProgram = glCreateProgram();

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				if (vshaderID[i] != FAILED_RETURN)
				{
					glAttachShader(shaderProgram, vshaderID[i]);
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
		
	};
}

