#pragma once
#include <string>

namespace InputProcessor::Renderer::Resource {
	typedef unsigned int ShaderID;

	// Forward declare ITexture to avoid circular include
	class ITexture;

	class IShader {
	public:
		virtual ~IShader() = default;
		virtual void SetUniformMatrix4fv(const std::string& name, const float* value) = 0;
		virtual void SetUniformMatrix3fv(const std::string& name, const float* value) = 0;
		virtual void SetUniform1i(const std::string& name, int value) = 0;
		virtual void SetUniform1f(const std::string& name, float value) = 0;
		virtual void SetUniform2f(const std::string& name, float v0, float v1) = 0;
		virtual void SetUniform3f(const std::string& name, float v0, float v1, float v2) = 0;
		virtual void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) = 0;
		virtual void SetUniformBuffer(const std::string& name, unsigned int bindingPointIdx) = 0;
		virtual void SetUniformTexture(const std::string& name, ITexture* texture) = 0;

		virtual void Use() = 0;
		virtual void Unuse() = 0;

		virtual ShaderID GetShaderId() const = 0;
	};
}
