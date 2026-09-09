#include <Resource/Opengl/OpenglShader.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Resource::Opengl {
	using namespace InputProcessor::Logger;

	OpenglShader::OpenglShader(const std::string& vertexSource, const std::string& fragmentSource)
		: mProgramID(0)
	{
		unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexSource);
		unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

		if (vs == 0 || fs == 0) {
			IP_ENGINE_ERROR("Aborting shader program linking due to compilation failure.");
			if (vs != 0) glDeleteShader(vs);
			if (fs != 0) glDeleteShader(fs);
			return;
		}

		mProgramID = glCreateProgram();
		glAttachShader(mProgramID, vs);
		glAttachShader(mProgramID, fs);
		glLinkProgram(mProgramID);

		int isLinked = 0;
		glGetProgramiv(mProgramID, GL_LINK_STATUS, &isLinked);
		if (!isLinked) {
			constexpr int LOG_BUFFER_SIZE = 1024;
			char infoLog[LOG_BUFFER_SIZE];
			glGetProgramInfoLog(mProgramID, LOG_BUFFER_SIZE, nullptr, infoLog);

			IP_ENGINE_ERROR("Shader program linking failed:\n{}", infoLog);

			glDeleteProgram(mProgramID);
			mProgramID = 0;
		}

		// Flag intermediate shaders for deletion once linked into the binary
		glDetachShader(mProgramID, vs);
		glDetachShader(mProgramID, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	OpenglShader::~OpenglShader() {
		if (mProgramID != 0) {
			IP_ENGINE_TRACE("Shader program destroyed id: {}", mProgramID);
			glDeleteProgram(mProgramID);
		}
	}

	void OpenglShader::Use() {
		glUseProgram(mProgramID);
	}

	void OpenglShader::Unuse() {
		glUseProgram(0);
	}

	unsigned int OpenglShader::CompileShader(unsigned int type, const std::string& source) {
		unsigned int shader = glCreateShader(type);
		const char* src = source.c_str();

		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			constexpr int LOG_BUFFER_SIZE = 1024;
			char infoLog[LOG_BUFFER_SIZE];
			glGetShaderInfoLog(shader, LOG_BUFFER_SIZE, nullptr, infoLog);

			const char* shaderTypeStr = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
			IP_ENGINE_ERROR("{} shader compilation failed:\n{}", shaderTypeStr, infoLog);

			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	// --- Uniform Setters ---

	void OpenglShader::SetUniformMatrix4fv(const std::string& name, const float* value) {
		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location != -1) {
			glUniformMatrix4fv(location, 1, GL_FALSE, value);
		}
		else {
			IP_ENGINE_WARN("Uniform Matrix4fv '{}' not found", name);
		}
	}

	void OpenglShader::SetUniformMatrix3fv(const std::string& name, const float* value) {
		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location != -1) {
			glUniformMatrix3fv(location, 1, GL_FALSE, value);
		}
		else {
			IP_ENGINE_WARN("Uniform Matrix3fv '{}' not found", name);
		}
	}

	void OpenglShader::SetUniform1i(const std::string& name, int value) {
		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location != -1) {
			glUniform1i(location, value);
		}
		else {
			IP_ENGINE_WARN("Uniform 1i '{}' not found", name);
		}
	}

	void OpenglShader::SetUniform1f(const std::string& name, float value) {
		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location != -1) {
			glUniform1f(location, value);
		}
		else {
			IP_ENGINE_WARN("Uniform 1f '{}' not found", name);
		}
	}

	void OpenglShader::SetUniform2f(const std::string& name, float v0, float v1) {
		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location != -1) {
			glUniform2f(location, v0, v1);
		}
		else {
			IP_ENGINE_WARN("Uniform 2f '{}' not found", name);
		}
	}

	void OpenglShader::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location != -1) {
			glUniform3f(location, v0, v1, v2);
		}
		else {
			IP_ENGINE_WARN("Uniform 3f '{}' not found", name);
		}
	}

	void OpenglShader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location != -1) {
			glUniform4f(location, v0, v1, v2, v3);
		}
		else {
			IP_ENGINE_WARN("Uniform 4f '{}' not found", name);
		}
	}
}