#include <WinInclude.h>
#include <D3DCommon/D3DInstance.h>
#include <D3DCommon/D3DWindow.h>
#include <D3DCommon/D3DQueue.h>
#include <D3DCommon/D3DCmdList.h>
#include <D3DCommon/D3DPipelineState.h>

#include <D3DMemory/D3DResource.h>
#include <D3DMemory/D3DUploadBuffer.h>

#include <Util/Shader.h>
#include <Util/DirWatcher.h>

#include <dxc/dxcapi.h>

#include <exception>

using namespace RTR;

class MyPsoType : public D3DPipelineState
{
    public:
        MyPsoType() :
            m_vs(L"test.hlsl", RTR_SHADER_VS_6_0, L"VsMain"),
            m_ps(L"test.hlsl", RTR_SHADER_PS_6_0, L"PsMain"),
            D3DPipelineState(PipelineStateType::Graffics)
        {}

        bool __internal_ConstructPso(IPsoManipulator* ptrManipulator) override
        {
            // Assert type
            if (ptrManipulator->GetType() != PipelineStateType::Graffics)
                return false;

            // Get Typed manipulator
            GfxPsoManipulator* ptrGfxManipulator = (GfxPsoManipulator*)ptrManipulator;

            // Setup shader
            ptrGfxManipulator->BindShader(ShaderType::VS, &m_vs);
            ptrGfxManipulator->BindShader(ShaderType::PS, &m_ps);
            
            // Setup render target
            ptrGfxManipulator->OMSetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

            // Setup IA Elements
            ptrGfxManipulator->IAAddElement("SV_POSITION", DXGI_FORMAT_R32G32B32A32_FLOAT);

            return true;
        }

    private:
        // Shader
        Shader m_vs, m_ps;
};

struct Vertex
{
    float x, y, z, w;
};

class TempBuffer : public D3DResource
{
    public:
        TempBuffer(size_t count)
        {
            D3D12_RESOURCE_DESC rd;
            rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            rd.Width = sizeof(Vertex) * count;
            rd.Height = 1;
            rd.DepthOrArraySize = 1;
            rd.MipLevels = 1;
            rd.Format = DXGI_FORMAT_UNKNOWN;
            rd.SampleDesc.Count = 1;
            rd.SampleDesc.Quality = 0;
            rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            rd.Flags = D3D12_RESOURCE_FLAG_NONE;

            GetD3D12DevicePtr()->CreateCommittedResource(GetD3D12DefaultHeapProperites(), D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_ptrResource));
            m_resourceState = D3D12_RESOURCE_STATE_COMMON;
        }
};

INT wWinMain_safe(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR cmdArgs, INT cmdShow)
{
    MyPsoType pso;

    // Init dir watcher
    DirWatchInit();

    // Init D3D12 and run application
    if (InitD3D12())
    {
        D3DQueue queue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        D3DCommandList list(queue);
        D3DUploadBuffer uploadBuffer(MemMiB(128));

        TempBuffer vertexBuffer(512);

        Vertex cpuData[] = {
            {-0.5f, -0.5f, 0.0f, 1.0f},
            { 0.0f,  0.5f, 0.0f, 1.0f},
            { 0.5f, -0.5f, 0.0f, 1.0f},
        };

        // Vertex buffer resource state
        if(vertexBuffer.EnsureResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST))
            list.ExecutSync();

        uploadBuffer.CopyBufferData(cpuData, sizeof(cpuData), vertexBuffer);
        uploadBuffer.Execute();
        uploadBuffer.Wait();

        Window wnd(L"RTR Window", queue);
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
            
            if (list.BindPipelineState(pso))
            {
                // Vertex buffer resource state
                vertexBuffer.EnsureResourceState(list, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

                // Bind vertex buffer
                D3D12_VERTEX_BUFFER_VIEW vbv;
                vbv.BufferLocation = vertexBuffer.operator ID3D12Resource*()->GetGPUVirtualAddress();
                vbv.SizeInBytes = sizeof(cpuData);
                vbv.StrideInBytes = sizeof(Vertex);

                // Set vertex buffer
                list.IAPrepare(vbv);

                // Viewport
                D3D12_VIEWPORT vp;
                vp.TopLeftX = 0;
                vp.TopLeftY = 0;
                vp.Width = wnd.GetWidth();
                vp.Height = wnd.GetHeight();
                vp.MinDepth = 1.0f;
                vp.MaxDepth = 0.0f;
                list.RSPrepare(vp);

                // Draw 
                list.ResourceBarrierFlush();
                list.Draw(3);
            }
                        
            // === END DRAW ===
            list.EndRender();

            // Present frame
            list.ExecutSync();
            wnd.Present(true);

            // Check for file change events
            DirWatchRefresh();
        }
        queue.Flush(2);
        list.~D3DCommandList();

        vertexBuffer.~TempBuffer();

        uploadBuffer.~D3DUploadBuffer();
        pso.~MyPsoType();
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
