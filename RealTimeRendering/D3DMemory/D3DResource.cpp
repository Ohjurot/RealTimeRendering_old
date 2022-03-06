#include "D3DResource.h"

bool RTR::D3DResource::EnsureResourceState(D3DCommandList& cmdList, D3D12_RESOURCE_STATES state)
{
    bool needsTransition = m_resourceState != state;

    if (needsTransition)
    {
        // Get and populate barrier
        auto* ptrBarrier = cmdList.ResourceBarrierPeekAndPush();
        ptrBarrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        ptrBarrier->Transition.pResource = m_ptrResource;
        ptrBarrier->Transition.StateBefore = m_resourceState;
        ptrBarrier->Transition.StateAfter = state;
        ptrBarrier->Transition.Subresource = 0;
        ptrBarrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        // Reflect state change
        m_resourceState = state;
    }

    return needsTransition;
}
