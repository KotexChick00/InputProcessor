#pragma once
#include <iostream>
#include <Window/IWindowVisitor.hpp>

namespace CoreEngine
{

    class ImguiWindowContextInitVisitor : public CoreEngine::IWindowVisitor
    {
    public:
        void Visit(CoreEngine::IWindow* window) override;
        // ImGui không hỗ trợ wxWidgets context — no-op để tránh lỗi compile
        void VisitWxWindow(CoreEngine::WxWindow* window) override { }
    };

    class ImguiWindowRenderVisitor : public CoreEngine::IWindowVisitor
    {
    public:
        void Visit(CoreEngine::IWindow* window) override;
        // ImGui không hỗ trợ wxWidgets render — no-op
        void VisitWxWindow(CoreEngine::WxWindow* window) override { }
    };

    class ImguiWindowShutdownVisitor : public CoreEngine::IWindowVisitor
    {
    public:
        void Visit(CoreEngine::IWindow* window) override;
        // ImGui không hỗ trợ wxWidgets shutdown — no-op
        void VisitWxWindow(CoreEngine::WxWindow* window) override { }
    };

}
