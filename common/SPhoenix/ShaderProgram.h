#pragma once

#include "Utils.h"

namespace SP
{
	enum ShaderType
	{
		VERTEX = 0, FRAGMENT, GEOMETRY
	};

	//The warper class containing max all three shader codes and shader program
	class ShaderProgram
	{
	public:
		//ShaderProgram() = delete;
		ShaderProgram()
			: mbValidProgram(false), mvbValidShader{ false, false, false },
			mbCreated(false)
		{

		}
		
		ShaderProgram(const ShaderProgram &shader)
		{
			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				const std::string code = shader.getShaderCode(ShaderType(i));
				setShaderCode(code, ShaderType(i));
				const std::string path = shader.getShaderPath(ShaderType(i));
				setShaderPath(path, ShaderType(i));
			}
		}

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

		std::string getShaderCode(ShaderType type)
		{
			return mvCode[type];
		}

		std::string getShaderCode(ShaderType type) const
		{
			return mvCode[type];
		}


		std::string getShaderPath(ShaderType type)
		{
			return mvShaderPath[type];
		}

		std::string getShaderPath(ShaderType type) const
		{
			return mvShaderPath[type];
		}

		void setShaderCode(const std::string &code, ShaderType type)
		{
			mvCode[type] = code;
		}

		void setShaderPath(const std::string &shaderPath, ShaderType type)
		{
			mvShaderPath[type] = shaderPath;
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

		virtual void createProgram()
		{
			//_printShaderPath();

			if (mbCreated) return;

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				if (!mvCode[i].empty())
				{
					//ShaderType type = i == 0 ? VERTEX : i == 1 ? FRAGMENT : GE
					mvbValidShader[i] = _createAndCompile(mvCode[i], (ShaderType)i,
														  mvShaderID[i]);
				}
			}
			mbValidProgram = _linkShaders(mvShaderID, mProgramID);
			mbCreated = true;
		}

		virtual std::shared_ptr<ShaderProgram> createCopy()
		{
			std::shared_ptr<ShaderProgram> pShader = std::make_shared<ShaderProgram>();
			
			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				pShader->setShaderCode(mvCode[i], ShaderType(i));
				pShader->setShaderPath(mvShaderPath[i], ShaderType(i));
			}

			return pShader;
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
		std::string mvShaderPath[SHADER_KINDS];

		GLuint mvShaderID[SHADER_KINDS];
		bool mvbValidShader[SHADER_KINDS];

		GLuint mProgramID;
		bool mbValidProgram;
		bool mbCreated;

	private:
		bool _readFromFile(const std::string &filename, ShaderType type)
		{
			mvShaderPath[type] = filename;
			std::string &shaderCode = mvCode[type], line = "";
			std::ifstream shaderStream(filename, std::ios::in);
			if (!shaderStream.is_open())
			{
				SP_CERR("Failed to read File: " + filename);
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

			switch (type)
			{
			case SP::VERTEX:
				shaderID = glCreateShader(GL_VERTEX_SHADER);
				break;
			case SP::FRAGMENT:
				shaderID = glCreateShader(GL_FRAGMENT_SHADER);
				break;
			case SP::GEOMETRY:
				shaderID = glCreateShader(GL_GEOMETRY_SHADER);
				break;
			default:
				break;
			}

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
				_printShaderPath();
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
				_printShaderPath();
				glDeleteProgram(shaderProgram);

				result = false;
			}

			//_printShaderPath();

			return result;
		}

		void _printShaderPath()
		{
			std::cout << "Vertex Shader Path: " << mvShaderPath[0] << std::endl;
			std::cout << "Fragment Shader Path: " << mvShaderPath[1] << std::endl;
			std::cout << "Geometry Shader Path: " << mvShaderPath[2] << std::endl;
		}
	};
}

