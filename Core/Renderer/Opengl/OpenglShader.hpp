#pragma once
#include <Renderer/Resource/IShader.hpp>
#include <glad/glad.h>

namespace InputProcessor::Renderer::Resource::Opengl {
	class OpenglShader : public IShader {
	public:
		virtual ~OpenglShader() override;
		virtual void SetUniformMatrix4fv(const std::string& name, const float* value) override;
		virtual void SetUniformMatrix3fv(const std::string& name, const float* value) override;
		virtual void SetUniform1i(const std::string& name, int value) override;
		virtual void SetUniform1f(const std::string& name, float value) override;
		virtual void SetUniform2f(const std::string& name, float v0, float v1) override;
		virtual void SetUniform3f(const std::string& name, float v0, float v1, float v2) override;
		virtual void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) override;
		virtual void SetUniformBuffer(const std::string& name, unsigned int bindingPointIdx) override;
		virtual void SetUniformTexture(const std::string& name, ITexture* texture) override;

		virtual void Use() override;
		virtual void Unuse() override;

		ShaderID GetShaderId() const override;
	private:
		unsigned int mProgramID;
		unsigned int mTextureUnitCounter = 0;

	private:
		OpenglShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		static unsigned int CompileShader(unsigned int type, const std::string& source);

	public:
		// Convenience overloads for raw string sources/paths
		static OpenglShader* FromSource(const std::string& vertexSource, const std::string& fragmentSource);
		static OpenglShader* FromFiles(const std::string& vertexFile, const std::string& fragmentFile);
	};
}
