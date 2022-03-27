#include <WinInclude.h>
#include <D3DCommon/D3DInstance.h>
#include <D3DCommon/D3DWindow.h>
#include <D3DCommon/D3DQueue.h>
#include <D3DCommon/D3DCmdList.h>
#include <D3DCommon/D3DDescriptorHeap.h>
#include <D3DMemory/D3DUploadBuffer.h>
#include <RTR/3DModells/ModelContext.h>
#include <RTR/3DModells/MatrixBuffer.h>
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
        BasicRendering(MatrixBuffer& matBuffer, D3DDescriptorHandle handle) :
            // Shaders
            m_vs(L"shaders/BasicVS.hlsl", RTR_SHADER_VS_6_0, L"BasicVS"),
            m_ps(L"shaders/BasicPS.hlsl", RTR_SHADER_PS_6_0, L"BasicPS"),
            D3DPipelineState(PipelineStateType::Graffics),

            // Matrices
            m_matProj(matBuffer.GetMatrix()),
            m_matView(matBuffer.GetMatrix()),
            m_matModel(matBuffer.GetMatrix()),

            // Configuration for the root signature
            m_rc(PipelineStateType::Graffics, 1,
                RootConfigurationEntry::MakeDescriptorTable(handle)
            )
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
            cbv.BufferLocation = matBuffer.GetAddress() + (m_matProj.offset * sizeof(DirectX::XMMATRIX));
            cbv.SizeInBytes = sizeof(DirectX::XMMATRIX) * 4;
            GetD3D12DevicePtr()->CreateConstantBufferView(&cbv, handle);
        } 

        // Vertex creation callback
        static void CbVertexCreate(Vertex* ptrVtx, size_t vtxIdx, const aiMesh* ptrMesh)
        {
            memcpy(ptrVtx, &ptrMesh->mVertices[vtxIdx], sizeof(aiVector3D));
            ptrVtx->pw = 1.0f;
        }

        // Easy bind
        bool Bind(D3DCommandList& cmdList)
        {
            bool bound = cmdList.BindPipelineState(*this);
            if (bound)
            {
                cmdList.BindRootConfiguration(m_rc);
            }
            return bound;
        }

        // Internally updates all transforms
        void UpdateMatrices(MatrixBuffer& matBuffer, float aspectRatio)
        {
            // Projection
            m_matProj.M = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(120.f), aspectRatio, 0.1f, 10.0f);
            matBuffer.UpdateCPU(m_matProj);

            // View
            m_matView.M = DirectX::XMMatrixTranslation(m_camPosition[0], m_camPosition[1], m_camPosition[2]);
            m_matView.M = DirectX::XMMatrixInverse(nullptr, m_matView.M);
            matBuffer.UpdateCPU(m_matView);

            // Model
            m_matModel.M = DirectX::XMMatrixRotationRollPitchYaw(
                DirectX::XMConvertToRadians(m_modelRotation[0]),
                DirectX::XMConvertToRadians(m_modelRotation[1]),
                DirectX::XMConvertToRadians(m_modelRotation[2])
            );
            matBuffer.UpdateCPU(m_matModel);
        }

        // Makes ImGui calls
        void UpdateImgui()
        {
            ImGui::Begin("Basic Rendering");

            // Camera position
            ImGui::DragFloat3("Camera Position", m_camPosition, 0.1f);
            // Model rotation
            ImGui::DragFloat3("Model Rotation", m_modelRotation, 5.0f);

            ImGui::End();
        }

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
        RootConfiguration m_rc;

        // Projection
        Matrix m_matProj;

        // View
        Matrix m_matView;
        float m_camPosition[3] = {0.0f, 0.0f, -10.f};

        // Model
        Matrix m_matModel;
        float m_modelRotation[3] = { 0.0f, 0.0f, 0.0f };
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

        // Matrix buffer
        MatrixBuffer matBuffer(32);
        D3DDescriptorHeap cbvSrvUavHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 32);

        // Window
        Window wnd(L"RTR Window", queue);
        ImGuiManager::Init(&wnd);

        // Assert copyable state
        mdlCtx.GetGeometryBufferResource()->EnsureResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST);
        list.ResourceBarrierFlush();
        list.ExecutSync();

        // Custom rendering instance
        BasicRendering renderingPso(matBuffer, cbvSrvUavHeap[0]);
        ModelInfo suzanne = mdlCtx.LoadModel("models/Suzanne.fbx", uploadBuffer, sizeof(BasicRendering::Vertex), (FModelVertexCallback)&BasicRendering::CbVertexCreate);
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

            // === UPDATE DATA ===
            // Update suzanne
            renderingPso.UpdateMatrices(matBuffer, (float)wnd.GetWidth() / wnd.GetHeight());

            // Matrix copy
            matBuffer.EnsureResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST);
            list.ExecutSync();
            matBuffer.UpdateGPU(uploadBuffer);
            uploadBuffer.ExecuteSync();
            matBuffer.EnsureResourceState(list, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

            // === BEGIN DRAW ===
            list.BeginRender(wnd.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, wnd.GetCurrentCPUHandle());
            ImGuiManager::NewFrame();
            
            // Keeping the imgui demo
            renderingPso.UpdateImgui();

            // Render suzanne
            list.BindDescriptorHeaps(cbvSrvUavHeap);
            if (renderingPso.Bind(list))
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
        cbvSrvUavHeap.~D3DDescriptorHeap();
        matBuffer.~MatrixBuffer();
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
