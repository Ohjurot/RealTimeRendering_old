#include "D3DCmdList.h"


RTR::D3DCommandList::D3DCommandList(D3DQueue& refQueue) :
    m_ptrQueue(&refQueue)
{
    // Create allocator
    RTR_CHECK_HRESULT(
        "Creating command allocator for command list", 
        GetD3D12DevicePtr()->CreateCommandAllocator(refQueue.GetQueueType(), IID_PPV_ARGS(&m_ptrAllocator))
    );

    // Create list
    RTR_CHECK_HRESULT(
        "Creating command list object",
        GetD3D12DevicePtr()->CreateCommandList1(0, refQueue.GetQueueType(), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_ptrList))
    );

    // Reset list for inital state
    RTR_CHECK_HRESULT("Inital command list reset", m_ptrList->Reset(m_ptrAllocator, nullptr));
}

D3D12_RESOURCE_BARRIER* RTR::D3DCommandList::ResourceBarrierPeek()
{
    // Check if barrier is left
    if (m_usedBarriers < _countof(m_barrieres))
    {
        // Flush resource barriers
        ResourceBarrierFlush();
    }
    
    return &m_barrieres[m_usedBarriers];
}

D3D12_RESOURCE_BARRIER* RTR::D3DCommandList::ResourceBarrierPeekAndPush()
{
    auto* current = ResourceBarrierPeek();
    ResourceBarrierPush();
    return current;
}

void RTR::D3DCommandList::ResourceBarrierPush()
{
    m_usedBarriers++;
}

void RTR::D3DCommandList::ResourceBarrierFlush()
{
    if (m_usedBarriers)
    {
        m_ptrList->ResourceBarrier(m_usedBarriers, m_barrieres);
        m_usedBarriers = 0;
    }
}

void RTR::D3DCommandList::BeginRender(ID3D12Resource* ptrRtvResource, D3D12_RESOURCE_STATES oldState, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
    // Prepare for RT
    prepareSingelRt(ptrRtvResource, oldState, rtvHandle);

    // Set RTV
    m_ptrList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void RTR::D3DCommandList::BeginRender(D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, ID3D12Resource* ptrRtvResource, D3D12_RESOURCE_STATES oldState, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
    // Prepare for RT
    prepareSingelRt(ptrRtvResource, oldState, rtvHandle);

    // Set RTV
    m_ptrList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
}

void RTR::D3DCommandList::BeginRender(unsigned int rtvCount, ...)
{
    // Buffer for va hanldes
    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[8] = {};

    // Begin va processing
    va_list vaArgs;
    va_start(vaArgs, rtvCount);
    
    // Prepare rts
    prepareMultipleRt(rtvCount, rtvs, vaArgs);

    // End va processing
    va_end(vaArgs);

    // Set rtvs
    m_ptrList->OMSetRenderTargets(rtvCount, rtvs, FALSE, nullptr);
}

void RTR::D3DCommandList::BeginRender(D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, unsigned int rtvCount, ...)
{
    // Buffer for va hanldes
    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[8] = {};

    // Begin va processing
    va_list vaArgs;
    va_start(vaArgs, rtvCount);

    // Prepare rts
    prepareMultipleRt(rtvCount, rtvs, vaArgs);

    // End va processing
    va_end(vaArgs);

    // Set rtvs
    m_ptrList->OMSetRenderTargets(rtvCount, rtvs, FALSE, &dsvHandle);
}

void RTR::D3DCommandList::EndRender()
{
    // Restore all old states
    for (unsigned int i = 0; i < m_rtvCount; i++)
    {
        // Execute resource barriers
        auto* barr = ResourceBarrierPeekAndPush();
        barr->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barr->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barr->Transition.pResource = m_rtvResources[i];
        barr->Transition.Subresource = 0;
        barr->Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barr->Transition.StateAfter = m_rtvRestoreStates[i];
    }

    m_rtvCount = 0;
}

void RTR::D3DCommandList::IAPrepare(const D3D12_VERTEX_BUFFER_VIEW& refVertexBufferView, D3D12_PRIMITIVE_TOPOLOGY bufferTopology)
{
    m_ptrList->IASetVertexBuffers(0, 1, &refVertexBufferView);
    m_ptrList->IASetPrimitiveTopology(bufferTopology);

    m_hasIndexBuffer = false;
}

void RTR::D3DCommandList::IAPrepare(unsigned int vertexBufferCount, const D3D12_VERTEX_BUFFER_VIEW* arrayOfVertexBuffers, D3D12_PRIMITIVE_TOPOLOGY bufferTopology)
{
    m_ptrList->IASetVertexBuffers(0, vertexBufferCount, arrayOfVertexBuffers);
    m_ptrList->IASetPrimitiveTopology(bufferTopology);

    m_hasIndexBuffer = false;
}

void RTR::D3DCommandList::IAPrepare(const D3D12_VERTEX_BUFFER_VIEW& refVertexBufferView, const D3D12_INDEX_BUFFER_VIEW& refIndexBufferView, D3D12_PRIMITIVE_TOPOLOGY bufferTopology)
{
    m_ptrList->IASetVertexBuffers(0, 1, &refVertexBufferView);
    m_ptrList->IASetIndexBuffer(&refIndexBufferView);
    m_ptrList->IASetPrimitiveTopology(bufferTopology);

    m_hasIndexBuffer = true;
}

void RTR::D3DCommandList::IAPrepare(unsigned int vertexBufferCount, const D3D12_VERTEX_BUFFER_VIEW* arrayOfVertexBuffers, const D3D12_INDEX_BUFFER_VIEW& refIndexBufferView, D3D12_PRIMITIVE_TOPOLOGY bufferTopology)
{
    m_ptrList->IASetVertexBuffers(0, vertexBufferCount, arrayOfVertexBuffers);
    m_ptrList->IASetIndexBuffer(&refIndexBufferView);
    m_ptrList->IASetPrimitiveTopology(bufferTopology);

    m_hasIndexBuffer = true;
}

void RTR::D3DCommandList::SOPrepare(const D3D12_STREAM_OUTPUT_BUFFER_VIEW& refStreamOutputView)
{
    m_ptrList->SOSetTargets(0, 1, &refStreamOutputView);
}

void RTR::D3DCommandList::SOPrepare(unsigned int streamOutputViewCount, const D3D12_STREAM_OUTPUT_BUFFER_VIEW* arrStreamOutputViews)
{
    m_ptrList->SOSetTargets(0, streamOutputViewCount, arrStreamOutputViews);
}

void RTR::D3DCommandList::RSPrepare(const D3D12_VIEWPORT& refViewPort)
{
    D3D12_RECT scissorRect;
    scissorRect.left = (LONG)refViewPort.TopLeftX;
    scissorRect.right = (LONG)(refViewPort.TopLeftX + refViewPort.Width);
    scissorRect.top = (LONG)(refViewPort.TopLeftY);
    scissorRect.bottom = (LONG)(refViewPort.TopLeftY + refViewPort.Height);
    RSPrepare(1, &refViewPort, &scissorRect);
}

void RTR::D3DCommandList::RSPrepare(const D3D12_VIEWPORT& refViewPort, const D3D12_RECT& refScissorRect)
{
    RSPrepare(1, &refViewPort, &refScissorRect);
}

void RTR::D3DCommandList::RSPrepare(unsigned int vpRsPairCount, const D3D12_VIEWPORT* arrViewPorts, const D3D12_RECT* arrScissorRects)
{
    m_ptrList->RSSetViewports(vpRsPairCount, arrViewPorts);
    m_ptrList->RSSetScissorRects(vpRsPairCount, arrScissorRects);
}

void RTR::D3DCommandList::OMSetBlendFactor(float blendFactors[4])
{
    m_ptrList->OMSetBlendFactor(blendFactors);
}

void RTR::D3DCommandList::OMSetBlendFactor(float blendR, float blendG, float blendB, float blendA)
{
    float arrFactors[] = { blendR, blendG, blendB, blendA };
    m_ptrList->OMSetBlendFactor(arrFactors);
}

void RTR::D3DCommandList::OMSetDepthBounds(float min, float max)
{
    m_ptrList->OMSetDepthBounds(min, max);
}

void RTR::D3DCommandList::OMSetStencilRef(uint32_t stencilValue)
{
    m_ptrList->OMSetStencilRef(stencilValue);
}

bool RTR::D3DCommandList::BindPipelineState(D3DPipelineState& refState)
{
    return refState.Bind(m_ptrList);
}

void RTR::D3DCommandList::BindRootConfiguration(const RootConfiguration& refRootConfig)
{
    if (!refRootConfig.Bind(m_ptrList))
    {
        #ifdef _DEBUG
        OutputDebugString(L"\n");
        #endif
    }
}

void RTR::D3DCommandList::Draw(unsigned int vertexOrIndexCount, unsigned int instanceCount)
{
    if (m_hasIndexBuffer)
    {
        // Draw indexed
        m_ptrList->DrawIndexedInstanced(vertexOrIndexCount, instanceCount, 0, 0, 0);
    }
    else
    {
        // Draw vertex
        m_ptrList->DrawInstanced(vertexOrIndexCount, instanceCount, 0, 0);
    }
}

void RTR::D3DCommandList::ExecutSync()
{
    // Flush pending barriers
    ResourceBarrierFlush();

    // Close cmd list
    RTR_CHECK_HRESULT("Closing command list", m_ptrList->Close());

    // Execute and wait on queue
    m_ptrQueue->Wait(m_ptrQueue->Execute(m_ptrList));

    // Reset list & allocator
    RTR_CHECK_HRESULT("Reseting command allocator", m_ptrAllocator->Reset());
    RTR_CHECK_HRESULT("Reseting command list", m_ptrList->Reset(m_ptrAllocator, nullptr));
}

void RTR::D3DCommandList::prepareSingelRt(ID3D12Resource* ptrRtvResource, D3D12_RESOURCE_STATES oldState, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
    // Store input parameters
    m_rtvCount = 1;
    m_rtvResources[0] = ptrRtvResource;
    m_rtvRestoreStates[0] = oldState;

    // Execute resource barries
    auto* barr = ResourceBarrierPeekAndPush();
    barr->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barr->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barr->Transition.pResource = ptrRtvResource;
    barr->Transition.Subresource = 0;
    barr->Transition.StateBefore = oldState;
    barr->Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Flushed barriers
    ResourceBarrierFlush();

    // Clear RTV
    static const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_ptrList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void RTR::D3DCommandList::prepareMultipleRt(unsigned int rtvCount, D3D12_CPU_DESCRIPTOR_HANDLE* handlesOut, va_list vaArgs)
{
    // Process all variadic arguments
    m_rtvCount = rtvCount;
    for (unsigned int i = 0; i < rtvCount; i++)
    {
        // Get arguments from vaList
        m_rtvResources[i] = va_arg(vaArgs, ID3D12Resource*);
        m_rtvRestoreStates[i] = va_arg(vaArgs, D3D12_RESOURCE_STATES);
        handlesOut[i] = va_arg(vaArgs, D3D12_CPU_DESCRIPTOR_HANDLE);

        // Resource barrier
        auto* barr = ResourceBarrierPeekAndPush();
        barr->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barr->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barr->Transition.pResource = m_rtvResources[i];
        barr->Transition.Subresource = 0;
        barr->Transition.StateBefore = m_rtvRestoreStates[i];
        barr->Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }
    va_end(vaArgs);

    // Flushed barriers
    ResourceBarrierFlush();

    // Clear rtvs
    for (unsigned int i = 0; i < rtvCount; i++)
    {
        static const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_ptrList->ClearRenderTargetView(handlesOut[i], clearColor, 0, nullptr);
    }
}

