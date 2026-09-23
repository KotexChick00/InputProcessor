#pragma once
#include <pch.h>

namespace CoreEngine::Renderer {
#pragma region Buffers
	enum class CORE_API ClearBufferMasks {
		Color = 1,
		Depth = 1 << 1
	};

	inline ClearBufferMasks operator|(ClearBufferMasks a, ClearBufferMasks b) {
		return static_cast<ClearBufferMasks>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline bool HasFlag(ClearBufferMasks a, ClearBufferMasks b) {
		return static_cast<bool>(static_cast<int>(a) & static_cast<int>(b)) != 0;
	}

	enum class CORE_API RenderMode {
		Triangles = 0
	};

	struct CORE_API ColorRGB {
		float Red;
		float Green;
		float Blue;
		ColorRGB() : Red(0.f), Green(0.f), Blue(0.f) {}
		ColorRGB(float red, float green, float blue) : Red(red), Green(green), Blue(blue) {}
	};

	struct CORE_API ColorRGBA {
		float Red;
		float Green;
		float Blue;
		float Alpha;
		ColorRGBA() : Red(0.f), Green(0.f), Blue(0.f), Alpha(0.f) {}
		ColorRGBA(float red, float green, float blue, float alpha = 1.f) : Red(red), Green(green), Blue(blue), Alpha(alpha) {}
	};

#pragma endregion
#pragma region CullFace
	enum class CORE_API CullFaceMode {
		Front = 0,
		Back = 1,
		FrontAndBack = 2
	};

	enum class CORE_API CullFaceDirection {
		ClockWise,
		CounterClockWise
	};

	struct CORE_API CullFaceOptions {
		bool Enabled = false;
		CullFaceMode CullFaceMode = CullFaceMode::Back;
		CullFaceDirection CullFaceDirection = CullFaceDirection::CounterClockWise;
	};
#pragma endregion
#pragma region Depth
	enum class CORE_API DepthPassFunc {
		Always,
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual
	};

	enum class CORE_API DepthOperation {
		ReadOnly, // Although fragment passed but not write the value to the buffer
		ReadAndWrite,
	};

	struct CORE_API DepthOptions {
		bool Enabled = false;
		DepthPassFunc PassFunc = DepthPassFunc::Less;
		DepthOperation Operation = DepthOperation::ReadOnly;
	};
#pragma endregion
#pragma region Stencil
	enum class CORE_API StencilPassFunc {
		Always,
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual
	};

	enum class CORE_API StencilAction {
		Keep,
		SetZero,
		Replace,
		Increment,			// 255 -> 255
		IncrementWrap,		// 255 -> 1
		Decrement,			// 0 -> 0
		DecrementWrap,		// 0 -> 255
		InvertBitWise		// Invert bit in the buffer
	};

	struct CORE_API StencilOptions {
		bool Enabled = false;
		StencilPassFunc PassFunc = StencilPassFunc::Less;
		int ReferenceValue = 0x00;
		int MaskValue = 0xFF;

		StencilAction OnStencilPassFail = StencilAction::Keep;
		StencilAction OnStencilPassDepthFail = StencilAction::Keep;
		StencilAction OnStencilPassDepthPass = StencilAction::Keep;
	};
#pragma endregion
#pragma region Blend

	// Note: If use are trying to use Dual Source Blending, then you should export out when writing Fragment shader
	enum class CORE_API BlendEquation {
		Add,
		Substract,
		ReverseSubstract,
		Min,
		Max
	};

	enum class CORE_API BlendRGBFactorFunc {
		Zero,
		One,
		SourceColor,
		OneMinusSourceColor,
		DestinationColor,
		OneMinusDestinationColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationAlpha,
		OneMinusDestinationAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha
	};

	enum class CORE_API BlendAlphaFactorFunc {
		Zero,
		One,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationAlpha,
		OneMinusDestinationAlpha,
		ConstantAlpha,
		OneMinusConstantAlpha
	};

	struct CORE_API BlendOptions {
		bool Enabled = false;
		BlendEquation BlendEquation = BlendEquation::Add;

		BlendRGBFactorFunc SourceFactorRGBFunc = BlendRGBFactorFunc::One;
		BlendRGBFactorFunc DestinationFactorRGBFunc = BlendRGBFactorFunc::One;
		BlendAlphaFactorFunc SourceAlphaFactorFunc = BlendAlphaFactorFunc::Zero;
		BlendAlphaFactorFunc DestinationAlphaFactorFunc = BlendAlphaFactorFunc::Zero;
	};

#pragma endregion
#pragma region Viewport
	struct CORE_API ViewPortOptions {
		unsigned int X = 0;
		unsigned int Y = 0;
		unsigned int Width;
		unsigned int Height;
	};
#pragma endregion
	struct CORE_API RendererConfiguration {
		ColorRGBA ClearBufferColor;
		ViewPortOptions ViewPortOptions;
		DepthOptions DepthOptions;
		CullFaceOptions CullFaceOptions;
		StencilOptions StencilOptions;
		BlendOptions BlendOptions;
	};
}