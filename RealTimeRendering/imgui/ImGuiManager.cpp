#include "ImGuiManager.h"

RTR::ImGuiManager RTR::ImGuiManager::s_mInstance;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void RTR::ImGuiManager::Init(Window* ptrWindow)
{
    // Local init
    s_mInstance.m_descHeap = std::move(D3DDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 16));
    s_mInstance.m_ptrWindow = ptrWindow;
    ptrWindow->AddCustomEventListener(&s_mInstance);

    // Imgui init
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(ptrWindow->GetWindowHandle());
    ImGui_ImplDX12_Init(GetD3D12DevicePtr(), 2, DXGI_FORMAT_R8G8B8A8_UNORM, s_mInstance.m_descHeap, s_mInstance.m_descHeap.First().cpu(), s_mInstance.m_descHeap.First().gpu());
}

void RTR::ImGuiManager::Shutdown()
{
    // Destroy imgui
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Destroy self
    s_mInstance.m_descHeap.~D3DDescriptorHeap();
}

void RTR::ImGuiManager::NewFrame()
{
    // Start frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void RTR::ImGuiManager::Render(D3DCommandList& list)
{
    // Render imgui
    ImGui::Render();

    // Render directx12
    list.BindDescriptorHeaps(s_mInstance.m_descHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), (ID3D12GraphicsCommandList*)list);
}

bool RTR::ImGuiManager::HandleWindowEvent(Window* wnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* ptrResult)
{
    return ImGui_ImplWin32_WndProcHandler(wnd->GetWindowHandle(), msg, wParam, lParam);
}
