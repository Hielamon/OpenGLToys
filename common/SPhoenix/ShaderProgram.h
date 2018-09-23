#pragma once

#include "Utils.h"

namespace SP
{
	enum ShaderType
	{
		VERTEX, FRAGMENT, GEOMETRY
	};

	//The warper class containing max all three shader codes and shader program
	class ShaderProgram
	{
	public:
		ShaderProgram() = delete;

		ShaderProgram(const std::string &vertFilePath, const std::string &fragFilePath)
			: mbValidProgram(false), mvbValidShader { false, false, false },
			mbCreated(false)
		{
			_readFromFile(vertFilePath, ShaderType::VERTEX);
			_readFromFile(fragFilePath, ShaderType::FRAGMENT);
		}

		ShaderProgram(const std::string &vertFilePath, const std::string &fragFilePath, const std::string &geomFilePath)
			: mbValidProgram(false), mvbValidShader { false, false, false },
			mbCreated(false)
		{
			_readFromFile(vertFilePath, ShaderType::VERTEX);
			_readFromFile(fragFilePath, ShaderType::FRAGMENT);
			_readFromFile(geomFilePath, ShaderType::GEOMETRY);
		}

		~ShaderProgram()
		{
			clearProgram();
		}

		void addMacros(const std::string &macros)
		{
			clearProgram();

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				std::string &code = mvCode[i];
				if (!code.empty())
				{
					int start = code.find("#version");
					int insertPos = code.find("\n", start);
					code.insert(insertPos + 1, macros);
				}
			}
		}

		void createProgram()
		{
			if (mbCreated) return;

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				if (!mvCode[i].empty())
				{
					mvbValidShader[i] = _createAndCompile(mvCode[i], (ShaderType)i,
														  mvShaderID[i]);
				}
			}
			mbValidProgram = _linkShaders(mvShaderID, mProgramID);
			mbCreated = true;
		}

		void clearProgram()
		{
			if (mbCreated)
			{
				if (mbValidProgram)
				{
					glDeleteProgram(mProgramID);
					mbValidProgram = false;
				}

				for (size_t i = 0; i < SHADER_KINDS; i++)
				{
					if (mvbValidShader[i])
					{
						glDeleteShader(mvShaderID[i]);
						mvbValidShader[i] = false;
					}
				}

				mbCreated = false;
			}
		}

		bool isValidProgram()
		{
			return mbValidProgram;
		}

		void useProgram()
		{
			glUseProgram(mProgramID);
		}

		GLuint getProgramID()
		{
			return mProgramID;
		}

	protected:
		std::string mvCode[SHADER_KINDS];

		GLuint mvShaderID[SHADER_KINDS];
		bool mvbValidShader[SHADER_KINDS];

		GLuint mProgramID;
		bool mbValidProgram;
		bool mbCreated;

	private:
		bool _readFromFile(const std::string &filename, ShaderType type)
		{
			std::string &shaderCode = mvCode[type], line = "";
			std::ifstream shaderStream(filename, std::ios::in);
			if (!shaderStream.is_open())
			{
				SP_CERR("Failed to valid File: " + filename);
				exit(-1);
			}
			shaderCode = "";

			while (std::getline(shaderStream, line))
				shaderCode += line + "\n";

			shaderStream.close();
			return true;
		}

		bool _createAndCompile(const std::string &shaderCode, ShaderType type,
							   GLuint &shaderID)
		{
			const char * shaderCodePtr = shaderCode.c_str();

			shaderID = glCreateShader(type == VERTEX ? GL_VERTEX_SHADER :
				(type == FRAGMENT ? GL_FRAGMENT_SHADER : GL_GEOMETRY_SHADER));

			glShaderSource(shaderID, 1, &shaderCodePtr, NULL);
			glCompileShader(shaderID);

			bool result = true;
			GLint success;
			GLchar infoLog[512];
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
				SP_CERR("Compile Error log : " + std::string(infoLog));
				glDeleteShader(shaderID);
				result = false;
			}

			return result;
		}

		bool _linkShaders(GLuint(&vshaderID)[SHADER_KINDS], GLuint &shaderProgram)
		{
			shaderProgram = glCreateProgram();

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				if (vshaderID[i] != FAILED_RETURN)
				{
					glAttachShader(shaderProgram, vshaderID[i]);
				}
			}

			glLinkProgram(shaderProgram);

			bool result = true;
			GLint success;
			GLchar infoLog[512];
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
				SP_CERR("Link Error log :" + std::string(infoLog));
				glDeleteProgram(shaderProgram);

				result = false;
			}

			return result;
		}
	};
}

