#pragma once

#include <WinInclude.h>
#include <Util/ComPointer.h>
#include <Util/HrException.h>
#include <D3DCommon/D3DInstance.h>

namespace RTR
{
    // DirectX command queue
    class D3DQueue
    {
        public:
            // Construct
            D3DQueue() = delete;
            D3DQueue(const D3DQueue&) = delete;
            D3DQueue(D3D12_COMMAND_LIST_TYPE type);

            // Assign
            D3DQueue& operator=(const D3DQueue&) = delete;

            // Execute
            UINT64 Execute(ID3D12CommandList*const* cmdLists, unsigned int listCout);
            UINT64 Execute(ID3D12CommandList* cmdList);
            void Wait(UINT64 mark);
            void Flush(UINT count = 1);
            bool IsFinished(UINT64 mark);

            // Inline get type
            D3D12_COMMAND_LIST_TYPE GetQueueType() const noexcept
            {
                return m_type;
            }

            // Castable
            inline operator ID3D12CommandQueue*()
            {
                return m_ptrQueue;
            }

        private:
            // Type of cmd list
            D3D12_COMMAND_LIST_TYPE m_type;

            // Fence for waiting and signaling
            ComPointer<ID3D12Fence> m_ptrFence;
            // Queue pointer
            ComPointer<ID3D12CommandQueue> m_ptrQueue;

            // Waiting event
            HANDLE m_hWaitEvent = NULL;

            // Last signaled value
            UINT64 m_lastSignaledValue = 0;
    };
}
