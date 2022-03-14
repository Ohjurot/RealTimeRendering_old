#include <WinInclude.h>
#include <D3DCommon/D3DInstance.h>
#include <D3DCommon/D3DWindow.h>
#include <D3DCommon/D3DQueue.h>
#include <D3DCommon/D3DCmdList.h>
#include <D3DCommon/D3DDescriptorHeap.h>
#include <D3DMemory/D3DUploadBuffer.h>

#include <Util/DirWatcher.h>
#include <imgui/ImGuiManager.h>

#include <exception>

using namespace RTR;

INT wWinMain_safe(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR cmdArgs, INT cmdShow)
{
    // Init dir watcher
    DirWatchInit();

    // Init D3D12 and run application
    if (InitD3D12())
    {
        // Common
        D3DQueue queue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        D3DCommandList list(queue);
        D3DUploadBuffer uploadBuffer(MemMiB(128));

        // Window
        Window wnd(L"RTR Window", queue);
        ImGuiManager::Init(&wnd);

        // App loop
        while (wnd.ProcessWindowEvents())
        {
            // Resize window if required
            if (wnd.NeedsResize())
            {
                queue.Flush(2);
                wnd.Resize();
            }

            // === BEGIN DRAW ===
            list.BeginRender(wnd.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, wnd.GetCurrentCPUHandle());
            ImGuiManager::NewFrame();
            
            // TODO: Rendering calls
            ImGui::ShowDemoWindow();
                        

            // === END DRAW ===
            ImGuiManager::Render(list);
            list.EndRender();

            // Present frame
            list.ExecutSync();
            wnd.Present(true);

            // Check for file change events
            DirWatchRefresh();
        }

        // Destroy imgui
        ImGuiManager::Shutdown();

        queue.Flush(2);
        list.~D3DCommandList();

        uploadBuffer.~D3DUploadBuffer();
        wnd.~Window();
        queue.~D3DQueue();

        // Shutdown D3D12
        ShutdownD3D12();
    }

    // Shutdown dir watcher
    DirWatchShutdown();

    return 0;
}

INT WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, PWSTR _In_ cmdArgs, INT _In_ cmdShow)
{
    INT returnValue = -1;

    try
    {
        returnValue = wWinMain_safe(hInstance, hPrevInstance, cmdArgs, cmdShow);
    }
    catch (std::exception& ex)
    {
        MessageBoxA(NULL, ex.what(), "Fatal application error", MB_OK | MB_ICONERROR);
    }

    return returnValue;
}
