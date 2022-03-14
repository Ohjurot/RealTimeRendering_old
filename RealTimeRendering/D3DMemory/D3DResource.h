#pragma once

#include <WinInclude.h>

#include <Util/ComPointer.h>

#include <D3DCommon/D3DCmdList.h>

namespace RTR
{
    class D3DResource
    {
        public: 
            // Ensures the correct resource state (does a barrier if required!) Will return true when a transition occured
            bool EnsureResourceState(D3DCommandList& cmdList, D3D12_RESOURCE_STATES state);

            // Get buffer address
            inline D3D12_GPU_VIRTUAL_ADDRESS GetAddress()
            {
                return m_ptrResource->GetGPUVirtualAddress();
            }

            // Retrieve the currently stored resource state
            inline D3D12_RESOURCE_STATES GetResourceState()
            {
                return m_resourceState;
            }
            // Set the current stored resource state
            inline void SetResourceState(D3D12_RESOURCE_STATES state)
            {
                m_resourceState = state;
            }

            // Resource pointer conversion
            inline operator ID3D12Resource* ()
            {
                return m_ptrResource;
            }

        protected:
            // Resource Pointer
            ComPointer<ID3D12Resource> m_ptrResource;
            // State of the resource
            D3D12_RESOURCE_STATES m_resourceState = D3D12_RESOURCE_STATE_COMMON;
    };
}
