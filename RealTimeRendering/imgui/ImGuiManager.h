#pragma once

#include <imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>

#include <D3DCommon/D3DWindow.h>
#include <D3DCommon/D3DCmdList.h>
#include <D3DCommon/D3DDescriptorHeap.h>

#include <utility>

namespace RTR
{
    class ImGuiManager : public WindowListener
    {
        public:
            // Init the imgui ctx
            static void Init(Window* ptrWindow);
            // Shutdown the context
            static void Shutdown();
            // Start a new imgui frame
            static void NewFrame();
            // Render the draw imgui data
            static void Render(D3DCommandList& list);

        public:
            // Passes on an event that ImGui can handle
            bool HandleWindowEvent(Window* wnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* ptrResult) override;

        private:
            // I'm a singleton
            ImGuiManager() = default;
            ImGuiManager(const ImGuiManager&) = delete;
            static ImGuiManager s_mInstance;

        private:
            // Window pointer
            Window* m_ptrWindow = nullptr;

            // Heap for imgui
            D3DDescriptorHeap m_descHeap;
    };
}
