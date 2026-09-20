#pragma once
#include <iostream>
#include <vector>
#include <functional>

namespace CoreEngine::UI {
	enum class SizeMode {
		Absolute,
		ParentRelative
	};

	struct SizeInfo {
		SizeMode SizeMode;
		
		// If size mode is ParentRelative
		float WidthFraction = 1.f;
		float HeightFraction = 1.f;

		// Else
		float Width = 0.f;
		float Height = 0.f;

		// Generals
		float MinWidth = 0.f;
		float MaxWidth = FLT_MAX;
		float MinHeight = 0.f;
		float MaxHeight = FLT_MAX;
	};

	class IResizeableUIElement {
	public:
		virtual void Resize(SizeInfo sizeInfo) = 0;
		virtual SizeInfo GetSize() const = 0;
	};

	enum class AnchorPresetPosition {
		TopLeft,
		TopMid,
		TopRight,
		CenterLeft,
		CenterMid,
		CenterRight,
		BotLeft,
		BotMid,
		BotRight
	};

	struct AnchorPositionInfo {
		AnchorPresetPosition PresetPosition;
		unsigned int OffsetX = 0;
		unsigned int OffsetY = 0;
	};

	class IUIElement {
	public:
		virtual void Render() = 0;
		virtual void SetAnchor(AnchorPositionInfo anchorPositionInfo) = 0;
		virtual AnchorPositionInfo GetAnchorInfo(AnchorPositionInfo anchorPositionInfo) const = 0;
	};

	struct PositionInfo {
		unsigned int OffsetX;
		unsigned int OffsetY;
	};

	class IPositionSetableUIElement {
	public:
		virtual void SetPosition(PositionInfo position) = 0;
		virtual PositionInfo GetPosition() = 0;
	};

	struct ContainerOffsetInfo {
		unsigned int Top = 0;
		unsigned int Bot = 0;
		unsigned int Left = 0;
		unsigned int Right = 0;
	};

	class IContainer : public IUIElement, IResizeableUIElement {
	public:
		virtual void AddChildren(IUIElement* element) = 0;
		virtual void OffsetContainer(ContainerOffsetInfo offsetInfo) = 0;
		virtual std::vector<IUIElement*> GetChildrens() const = 0;
	};

	enum class VboxAlignment {
		Begin,
		Center,
		End
	};

	struct VboxConfiguration {
		float ElementSpace = 0.f;
		VboxAlignment Alignment;
	};

	class IVboxContainer : public IContainer {
	public:
		virtual void SetConfig(VboxConfiguration config) { }
		virtual VboxConfiguration GetConfig() const = 0;
	};

	enum class HboxAlignment {
		Begin,
		Center,
		End
	};

	struct HboxConfiguration {
		float ElementSpace = 0.f;
		VboxAlignment Alignment;
	};

	class IHboxContainer : public IContainer {
	public:
		virtual void SetConfig(HboxConfiguration config) { }
		virtual HboxConfiguration GetConfig() const = 0;
	};	

	class IButton : public IUIElement, IPositionSetableUIElement, IResizeableUIElement {
	public:
		virtual void SetCallback(std::function<void()> commandFunction) { }
		virtual void SetText(const std::string& text) = 0;
	};
};