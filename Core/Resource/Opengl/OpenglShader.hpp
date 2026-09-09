#pragma once
#include <Resource/IShader.hpp>
#include <glad/glad.h>

namespace InputProcessor::Resource::Opengl {
	class OpenglShader : public IShader {
	public:
		OpenglShader(const std::string& vertexSource, const std::string& fragmentSource);
		virtual ~OpenglShader() override;
		virtual void SetUniformMatrix4fv(const std::string& name, const float* value) override;
		virtual void SetUniformMatrix3fv(const std::string& name, const float* value) override;
		virtual void SetUniform1i(const std::string& name, int value) override;
		virtual void SetUniform1f(const std::string& name, float value) override;
		virtual void SetUniform2f(const std::string& name, float v0, float v1) override;
		virtual void SetUniform3f(const std::string& name, float v0, float v1, float v2) override;
		virtual void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) override;

		virtual void Use() override;
		virtual void Unuse() override;
	private:
		unsigned int mProgramID;

	private:
		unsigned int CompileShader(unsigned int type, const std::string& source);
	};
}