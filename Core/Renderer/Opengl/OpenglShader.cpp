#include <Renderer/Opengl/OpenglShader.hpp>
#include <Renderer/Opengl/OpenglTexture.hpp>
#include <Renderer/Resource/ITexture.hpp>
#include <Logger/Logger.hpp>
#include <fstream>
#include <sstream>

namespace InputProcessor::Renderer::Resource::Opengl {
	using namespace InputProcessor::Logger;

	OpenglShader::OpenglShader(const std::string& vertexSource, const std::string& fragmentSource)
		: mProgramID(0), mTextureUnitCounter(0)
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

	ShaderID OpenglShader::GetShaderId() const {
		return static_cast<ShaderID>(mProgramID);
	}

	void OpenglShader::SetUniformBuffer(const std::string& name, unsigned int bindingPointIdx) {
		if (mProgramID == 0) {
			IP_ENGINE_WARN("SetUniformBuffer called on invalid shader program");
			return;
		}

		GLuint blockIndex = glGetUniformBlockIndex(mProgramID, name.c_str());
		if (blockIndex == GL_INVALID_INDEX) {
			IP_ENGINE_WARN("Uniform block '{}' not found in shader program {}", name, mProgramID);
			return;
		}

		glUniformBlockBinding(mProgramID, blockIndex, static_cast<GLuint>(bindingPointIdx));
	}

	void OpenglShader::SetUniformTexture(const std::string& name, ITexture* texture) {
		if (mProgramID == 0) {
			IP_ENGINE_WARN("SetUniformTexture called on invalid shader program");
			return;
		}

		if (texture == nullptr) {
			IP_ENGINE_WARN("SetUniformTexture called with nullptr texture for '{}'", name);
			return;
		}

		int location = glGetUniformLocation(mProgramID, name.c_str());
		if (location == -1) {
			IP_ENGINE_WARN("Uniform '{}' not found in shader program {}", name, mProgramID);
			return;
		}

		// Determine max available texture units
		GLint maxUnits = 0;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
		if (maxUnits <= 0) maxUnits = 1;

		GLuint unit = mTextureUnitCounter % static_cast<GLuint>(maxUnits);
		// Only support OpenglTexture here; skip if texture is another implementation
		OpenglTexture* oglTex = dynamic_cast<OpenglTexture*>(texture);
		if (oglTex == nullptr) {
			IP_ENGINE_WARN("SetUniformTexture only supports OpenglTexture instances for '{}'", name);
			return;
		}

		GLuint nativeTexId = oglTex->GetOpenglId();
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, nativeTexId);
		glUniform1i(location, static_cast<GLint>(unit));

		// advance counter so next call binds to next unit
		mTextureUnitCounter = (mTextureUnitCounter + 1) % static_cast<GLuint>(maxUnits);
	}

	OpenglShader* OpenglShader::FromSource(const std::string& vertexSource, const std::string& fragmentSource) {
		IP_ENGINE_TRACE("Creating shader from raw sources. Vertex source length: {}, Fragment source length: {}", vertexSource.size(), fragmentSource.size());
		IP_ENGINE_TRACE("Vertex Source:\n{}", vertexSource);
		IP_ENGINE_TRACE("Fragment Source:\n{}", fragmentSource);

		OpenglShader* shader = new OpenglShader(vertexSource, fragmentSource);
		if (shader->GetShaderId() == 0) {
			delete shader;
			return nullptr;
		}

		return shader;
	}

	OpenglShader* OpenglShader::FromFiles(const std::string& vertexFile, const std::string& fragmentFile) {
		auto readFile = [](const std::string& path) -> std::string {
			std::ifstream ifs(path);
			if (!ifs.is_open()) return std::string();
			std::stringstream ss;
			ss << ifs.rdbuf();
			return ss.str();
		};

		std::string vertexSrc = readFile(vertexFile);
		if (vertexSrc.empty()) {
			IP_ENGINE_ERROR("Failed to read vertex shader file: {}", vertexFile);
			return nullptr;
		}

		std::string fragmentSrc = readFile(fragmentFile);
		if (fragmentSrc.empty()) {
			IP_ENGINE_ERROR("Failed to read fragment shader file: {}", fragmentFile);
			return nullptr;
		}

		IP_ENGINE_TRACE("Loaded shader files. Vertex file: {} ({} bytes), Fragment file: {} ({} bytes)", vertexFile, vertexSrc.size(), fragmentFile, fragmentSrc.size());
		IP_ENGINE_TRACE("Vertex Source:\n{}", vertexSrc);
		IP_ENGINE_TRACE("Fragment Source:\n{}", fragmentSrc);

		return FromSource(vertexSrc, fragmentSrc);
	}

}