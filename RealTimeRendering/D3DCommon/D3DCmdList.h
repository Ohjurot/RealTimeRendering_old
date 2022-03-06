#pragma once

#include <WinInclude.h>
#include <Util/ComPointer.h>
#include <Util/HrException.h>
#include <D3DCommon/D3DInstance.h>
#include <D3DCommon/D3DQueue.h>
#include <D3DCommon/D3DRootConfiguration.h>
#include <D3DCommon/D3DPipelineState.h>

namespace RTR
{
    // Command list to record GPU commands
    class D3DCommandList
    {
        public:
            // Construct
            D3DCommandList() = delete;
            D3DCommandList(const D3DCommandList&) = delete;
            D3DCommandList(D3DQueue& refQueue);

            // Assign
            D3DCommandList& operator=(const D3DCommandList&) = delete;

            // Peek the next possible resource barrier
            D3D12_RESOURCE_BARRIER* ResourceBarrierPeek();
            // Peek the next possible resource barrier and mark it as used
            D3D12_RESOURCE_BARRIER* ResourceBarrierPeekAndPush();
            // Mark next resource barrier as used
            void ResourceBarrierPush();
            // Flush / Execute resource barriers
            void ResourceBarrierFlush();

            // Begin rendering on ONE Render Target
            void BeginRender(ID3D12Resource* ptrRtvResource, D3D12_RESOURCE_STATES oldState, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
            // Begin redner with DSV
            void BeginRender(D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, ID3D12Resource* ptrRtvResource, D3D12_RESOURCE_STATES oldState, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
            // Begin rendering on multiple render targets (format: count, [ID3D12Resource*, D3D12_RESOURCE_STATES, D3D12_CPU_DESCRIPTOR_HANDLE], ...
            void BeginRender(unsigned int rtvCount, ...);
            // Begin rendering on multiple render targets (format: count, [ID3D12Resource*, D3D12_RESOURCE_STATES, D3D12_CPU_DESCRIPTOR_HANDLE], ...
            void BeginRender(D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, unsigned int rtvCount, ...);
            // End rendering (resource barrier back to old state)
            void EndRender();

            // Prepare input assembler (without index buffer)
            void IAPrepare(const D3D12_VERTEX_BUFFER_VIEW& refVertexBufferView, D3D12_PRIMITIVE_TOPOLOGY bufferTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // Prepare input assembler (without index buffer) & multiple vertex buffer
            void IAPrepare(unsigned int vertexBufferCount, const D3D12_VERTEX_BUFFER_VIEW* arrayOfVertexBuffers, D3D12_PRIMITIVE_TOPOLOGY bufferTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // Prepare input assembler
            void IAPrepare(const D3D12_VERTEX_BUFFER_VIEW& refVertexBufferView, const D3D12_INDEX_BUFFER_VIEW& refIndexBufferView, D3D12_PRIMITIVE_TOPOLOGY bufferTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            // Prepare input assembler for multiple very buffers
            void IAPrepare(unsigned int vertexBufferCount, const D3D12_VERTEX_BUFFER_VIEW* arrayOfVertexBuffers, const D3D12_INDEX_BUFFER_VIEW& refIndexBufferView, D3D12_PRIMITIVE_TOPOLOGY bufferTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            
            // Prepare stream output
            void SOPrepare(const D3D12_STREAM_OUTPUT_BUFFER_VIEW& refStreamOutputView);
            // Prepare multiple stream output
            void SOPrepare(unsigned int streamOutputViewCount, const D3D12_STREAM_OUTPUT_BUFFER_VIEW* arrStreamOutputViews);

            // Only set the viewport (gennerate rect automaticals)
            void RSPrepare(const D3D12_VIEWPORT& refViewPort);
            // Set one viewport and Scissor rect
            void RSPrepare(const D3D12_VIEWPORT& refViewPort, const D3D12_RECT& refScissorRect);
            // Set multiple viewports and rects
            void RSPrepare(unsigned int vpRsPairCount, const D3D12_VIEWPORT* arrViewPorts, const D3D12_RECT* arrScissorRects);

            // Set OM blend factors
            void OMSetBlendFactor(float blendFactors[4]);
            // Set OM blend factors
            void OMSetBlendFactor(float blendR, float blendG, float blendB, float blendA);
            // Set OM blend factors
            void OMSetDepthBounds(float min, float max);
            // Set OM stencil reference value
            void OMSetStencilRef(uint32_t stencilValue);

            // Bind pipeline state
            bool BindPipelineState(D3DPipelineState& refState);
            // Bind root configuration
            void BindRootConfiguration(const RootConfiguration& refRootConfig);

            // Draws instanced (1 by default) with or without index buffer (determained by last call to IAPrepare)
            void Draw(unsigned int vertexOrIndexCount, unsigned int instanceCount = 1);

            // Execute command list
            void ExecutSync();

        private:
            // Internal helpers for less code duplication
            void prepareSingelRt(ID3D12Resource* ptrRtvResource, D3D12_RESOURCE_STATES oldState, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
            void prepareMultipleRt(unsigned int rtvCount, D3D12_CPU_DESCRIPTOR_HANDLE* handlesOut, va_list vaArgs);

        private:
            // Pointer to responisble queue
            D3DQueue* m_ptrQueue = nullptr;

            // Index buffer state
            bool m_hasIndexBuffer = false;

            // Resource barrier state
            unsigned int m_usedBarriers = 0;
            D3D12_RESOURCE_BARRIER m_barrieres[32];

            // RTV State
            unsigned int m_rtvCount = 0;
            ID3D12Resource* m_rtvResources[8];
            D3D12_RESOURCE_STATES m_rtvRestoreStates[8];

            // DirectX list & allocator pointer
            ComPointer<ID3D12GraphicsCommandList6> m_ptrList;
            ComPointer<ID3D12CommandAllocator> m_ptrAllocator;
    };
}
