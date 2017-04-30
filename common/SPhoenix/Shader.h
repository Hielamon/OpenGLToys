#pragma once

#include <SPhoenix/utils.h>

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
		ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath);
		ShaderCodes(const std::string &vertFilePath, const std::string &fragFilePath, const std::string &geomFilePath);

		~ShaderCodes(){}

	protected:
		ShaderCodes();

		std::vector<std::shared_ptr<std::string>> mcodes;

	private:
		void _initCommon(std::vector<std::string> &filenames);
	};

	/**Create , Compile Shaders and Give interface to access the programID*/
	class ShaderUtil : public ShaderCodes
	{
	public:
		ShaderUtil(ShaderCodes &shaderCodes);
		ShaderUtil() {}

		~ShaderUtil();
		bool isOpen();
		void useProgram();
		GLuint getProgramID();

	private:

		GLuint _createAndCompile(const std::string &shaderCode, ShaderType type);

		GLuint _linkShaders(const std::vector<GLuint> &shaderIDs);

		GLuint mprogramID;
		bool mopened;
	};
}

