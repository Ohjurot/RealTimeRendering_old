#pragma once

#include <WinInclude.h>

#include <Util/ComPointer.h>
#include <Util/HrException.h>
#include <Util/Memory.h>

#include <D3DCommon/D3DInstance.h>
#include <D3DCommon/D3DQueue.h>

namespace RTR
{
    // Self managed upload buffer
    class D3DUploadBuffer
    {
        public:
            // Construct and destruct
            D3DUploadBuffer() = delete;
            D3DUploadBuffer(UINT64 bufferSize);
            D3DUploadBuffer(const D3DUploadBuffer&) = delete;
            ~D3DUploadBuffer();

            // No copy
            D3DUploadBuffer& operator=(const D3DUploadBuffer&) = delete;
            D3DUploadBuffer& operator=(D3DUploadBuffer&&) = default;

            // Upload memory reservation function
            void* ReserverUploadMemory(UINT64 reservationSize);

            // Post memory copy executions
            bool CommitBufferCopy(void* ptrLocalMemory, UINT64 localMemorySize, ID3D12Resource* ptrTargetResource, UINT64 targetOffset = 0);
            bool CommitTextureCopy(void* ptrLocalMemory, DXGI_FORMAT format, UINT width, UINT height, UINT depth, UINT rowStride, ID3D12Resource* ptrTargetTexture, UINT destX = 0, UINT destY = 0, UINT destZ = 0, UINT subresourceIndex = 0);

            // Image row with requirements
            static UINT GetRequiredImageRowStride(UINT width, UINT bytesPerPixel);

            // Operate on existing memory
            bool CopyBufferData(void* ptrLocalMemory, UINT64 localMemorySize, ID3D12Resource* ptrTargetResource, UINT64 targetOffset = 0);
            bool CopyTextureData(void* ptrLocalMemory, DXGI_FORMAT format, UINT width, UINT height, UINT depth, UINT rowStride, ID3D12Resource* ptrTargetTexture, UINT destX = 0, UINT destY = 0, UINT destZ = 0, UINT subresourceIndex = 0);

            // Execution and wait
            void Execute();
            void Wait();
            bool CheckFinished();
            
            // Sync execution
            inline void ExecuteSync()
            {
                Execute();
                Wait();
            }

            // Get execution state
            inline bool GetExecutionState()
            {
                return m_isExecuting;
            }
            inline UINT64 GetOpenReservationsCount()
            {
                return m_openMemoryReservations;
            }

        private:
            // Execution state
            bool m_isExecuting = 0;
            UINT64 m_queueWaitValue = 0;

            // D3D Buffer resource
            ComPointer<ID3D12Resource> m_d3dResource;

            // Mapped pointer
            unsigned char* m_ptrMappedData = nullptr;

            // Command queue, list and allocator that will be used for upload commands
            D3DQueue m_uploadQueue;
            ComPointer<ID3D12CommandAllocator> m_uploadCommandAllocator;
            ComPointer<ID3D12GraphicsCommandList> m_uploadCommandList;

            // Buffer size and usage details
            UINT64 m_bufferSize = 0;
            UINT64 m_bufferUsage = 0;

            // Open memory reservations
            UINT64 m_openMemoryReservations = 0;
    };
}
