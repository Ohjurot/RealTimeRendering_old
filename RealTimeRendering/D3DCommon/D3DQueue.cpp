#include "D3DQueue.h"

RTR::D3DQueue::D3DQueue(D3D12_COMMAND_LIST_TYPE type) :
    m_type(type)
{
    // Create fence
    RTR_CHECK_HRESULT(
        "Creating signaling fence for command queue",
        GetD3D12DevicePtr()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_ptrFence))
    );

    // Describe queu
    D3D12_COMMAND_QUEUE_DESC qd = {};
    qd.Type = type;
    qd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
    qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    qd.NodeMask = 0;

    // Create event
    m_hWaitEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);

    // create queue
    RTR_CHECK_HRESULT(
        "Creating command queue object",
        GetD3D12DevicePtr()->CreateCommandQueue(&qd, IID_PPV_ARGS(&m_ptrQueue))
    );
}

UINT64 RTR::D3DQueue::Execute(ID3D12CommandList* const* cmdLists, unsigned int listCout)
{
    // Execute command list and signal fence to next value
    m_ptrQueue->ExecuteCommandLists(listCout, cmdLists);
    m_ptrQueue->Signal(m_ptrFence, ++m_lastSignaledValue);
    return m_lastSignaledValue;
}

UINT64 RTR::D3DQueue::Execute(ID3D12CommandList* cmdList)
{
    ID3D12CommandList* lists[] = { cmdList };
    return Execute(lists, 1);
}

void RTR::D3DQueue::Wait(UINT64 mark)
{
    // Check if wait if required
    if (m_ptrFence->GetCompletedValue() < mark)
    {
        // Try event substription and wait
        if (
            !m_hWaitEvent || 
            !ResetEvent(m_hWaitEvent) || 
            FAILED(m_ptrFence->SetEventOnCompletion(mark, m_hWaitEvent)) ||
            WaitForSingleObject(m_hWaitEvent, INFINITE) != WAIT_OBJECT_0
        )
        {
            // Manual wait loop
            while (m_ptrFence->GetCompletedValue() < mark)
            {
                SwitchToThread();
            }
        }
    }
}

void RTR::D3DQueue::Flush(UINT count)
{
    for (unsigned int i = 0; i < count; i++)
    {
        // Signal and wait count times to flush pending data
        m_ptrQueue->Signal(m_ptrFence, ++m_lastSignaledValue);
        Wait(m_lastSignaledValue);
    }
}

bool RTR::D3DQueue::IsFinished(UINT64 mark)
{
    return m_ptrFence->GetCompletedValue() >= mark;
}
