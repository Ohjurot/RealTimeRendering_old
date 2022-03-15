#include <WinInclude.h>
#include <D3DCommon/D3DInstance.h>
#include <D3DCommon/D3DWindow.h>
#include <D3DCommon/D3DQueue.h>
#include <D3DCommon/D3DCmdList.h>
#include <D3DCommon/D3DDescriptorHeap.h>
#include <D3DMemory/D3DUploadBuffer.h>
#include <RTR/3DModells/ModelContext.h>
#include <imgui/ImGuiManager.h>
#include <Util/DirWatcher.h>


#include <exception>

using namespace RTR;

class BasicRendering : public D3DPipelineState
{
    public:
        // Defines a vertex
        struct Vertex
        {
            float px, py, pz, pw;
        };

    public:
        // Constructor that load shaders
        BasicRendering() :
            m_vs(L"shaders/BasicVS.hlsl", RTR_SHADER_VS_6_0, L"BasicVS"),
            m_ps(L"shaders/BasicPS.hlsl", RTR_SHADER_PS_6_0, L"BasicPS"),
            D3DPipelineState(PipelineStateType::Graffics)
        {}

    protected:
        // Construct the pipeline state
        bool __internal_ConstructPso(IPsoManipulator* ptrManipulator) override
        {
            // Check the type
            if (ptrManipulator->GetType() != PipelineStateType::Graffics)
                throw std::exception("Unexpected pipeline state manipulator type");

            // Case to gfx manipulator
            GfxPsoManipulator* pso = (GfxPsoManipulator*)ptrManipulator;

            // Set shaders
            pso->BindShader(ShaderType::VS, &m_vs);
            pso->BindShader(ShaderType::PS, &m_ps);

            // Setup render target
            pso->OMSetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

            // Setup the input layout
            pso->IAAddElement("SV_POSITION", DXGI_FORMAT_R32G32B32A32_FLOAT);

            return true;
        }

    private:
        // My shaders
        Shader m_vs, m_ps;
};


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
        ModelContext mdlCtx(MemMiB(512));

        // Window
        Window wnd(L"RTR Window", queue);
        ImGuiManager::Init(&wnd);

        // Assert copyable state
        mdlCtx.GetGeometryBufferResource()->EnsureResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST);
        list.ResourceBarrierFlush();
        list.ExecutSync();

        // Custom rendering instance
        BasicRendering renderingPso;
        ModelInfo suzanne = mdlCtx.LoadModel("models/Suzanne.fbx", uploadBuffer);
        if (!suzanne)
            throw std::exception("Cannot load Suzanne!");

        // Upload data and restore buffer state 
        uploadBuffer.Execute();
        uploadBuffer.Wait();
        mdlCtx.GetGeometryBufferResource()->EnsureResourceState(list, 
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER);

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
            
            // Keeping the imgui demo
            ImGui::ShowDemoWindow();
            
            // Render suzanne
            if (list.BindPipelineState(renderingPso))
            {
                // Bind viewport
                D3D12_VIEWPORT vp;
                vp.TopLeftX = 0;
                vp.TopLeftY = 0;
                vp.Width = wnd.GetWidth();
                vp.Height = wnd.GetHeight();
                vp.MinDepth = 1.0f;
                vp.MaxDepth = 0.0f;
                list.RSPrepare(vp);

                // Bind mesh
                MeshInfo mesh = mdlCtx.GetMeshInfo(suzanne);
                list.IAPrepare(
                    mesh.vertexBuffer.CreateVertexBufferView(sizeof(BasicRendering::Vertex)),
                    mesh.indexBuffer.CreateIndexBufferView(sizeof(unsigned int))
                );

                // Draw indexed
                list.Draw(mesh.indexCount);
            }

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

        // Destroy custom instances
        mdlCtx.~ModelContext();
        renderingPso.~BasicRendering();

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
