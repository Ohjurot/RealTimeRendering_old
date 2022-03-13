#pragma once

#include <WinInclude.h>
#include <Util/ComPointer.h>
#include <Util/HrException.h>
#include <D3DCommon/D3DInstance.h>

#include <exception>

namespace RTR
{
    // Descriptor handle
    class D3DDescriptorHandle
    {
        public:
            // Constructors
            D3DDescriptorHandle() = default;
            D3DDescriptorHandle(const D3DDescriptorHandle&) = default;
            inline D3DDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpu, D3D12_CPU_DESCRIPTOR_HANDLE cpu) :
                m_GPU(gpu),
                m_CPU(cpu)
            {}

            // Assign
            D3DDescriptorHandle& operator=(const D3DDescriptorHandle&) = default;

            // Get handles
            inline D3D12_CPU_DESCRIPTOR_HANDLE cpu()
            {
                return m_CPU;
            }
            inline D3D12_GPU_DESCRIPTOR_HANDLE gpu()
            {
                return m_GPU;
            }

            // Conversion operators
            inline operator D3D12_GPU_DESCRIPTOR_HANDLE()
            {
                return m_GPU;
            }
            inline operator const D3D12_GPU_DESCRIPTOR_HANDLE* ()
            {
                return &m_GPU;
            }
            inline operator D3D12_CPU_DESCRIPTOR_HANDLE()
            {
                return m_CPU;
            }
            inline operator const D3D12_CPU_DESCRIPTOR_HANDLE* ()
            {
                return &m_CPU;
            }

        private:
            // Guarded handles
            D3D12_CPU_DESCRIPTOR_HANDLE m_CPU;
            D3D12_GPU_DESCRIPTOR_HANDLE m_GPU;
    };

    // Range of descriptors
    class D3DDescriptorRange
    {
        public:
            // Constructor
            D3DDescriptorRange() = default;
            D3DDescriptorRange(const D3DDescriptorRange&) = default;
            D3DDescriptorRange(D3DDescriptorHandle first, UINT count, UINT increment);

            // Assign
            D3DDescriptorRange& operator=(const D3DDescriptorRange&) = default;

            // Access handle
            D3DDescriptorHandle At(UINT idx);

            // Access operator
            inline D3DDescriptorHandle operator[](UINT idx)
            {
                return At(idx);
            }

            // Conversion operator for first element
            inline operator D3DDescriptorHandle()
            {
                return At(0);
            }
            inline operator D3D12_CPU_DESCRIPTOR_HANDLE()
            {
                return At(0).cpu();
            }
            inline operator D3D12_CPU_DESCRIPTOR_HANDLE*()
            {
                return &At(0).cpu();
            }
            inline operator D3D12_GPU_DESCRIPTOR_HANDLE()
            {
                return At(0).gpu();
            }
            inline operator D3D12_GPU_DESCRIPTOR_HANDLE* ()
            {
                return &At(0).gpu();
            }

        private:
            // First handle in this range
            D3DDescriptorHandle m_first;
            // Maximum number of handles in the range
            UINT m_count = 0;
            // Handle increment count
            UINT m_increment = 0;
    };

    // DirectX command queue
    class D3DDescriptorHeap
    {
        public:
            // Construct
            D3DDescriptorHeap() = default;
            D3DDescriptorHeap(const D3DDescriptorHeap&) = delete;
            D3DDescriptorHeap(D3DDescriptorHeap&&) = default;
            D3DDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count);

            // Assign
            D3DDescriptorHeap& operator=(const D3DDescriptorHeap&) = delete;
            D3DDescriptorHeap& operator=(D3DDescriptorHeap&&) noexcept;

            // Get count 
            inline size_t GetSize()
            {
                return m_handleCount;
            }

            // Get first handle
            D3DDescriptorHandle First();
            // Get handle at position
            D3DDescriptorHandle At(size_t idx);
            // Get a range
            D3DDescriptorRange Range(size_t idx, size_t count);

            // At operator
            inline D3DDescriptorHandle operator[](size_t idx)
            {
                return At(idx);
            }

            // Convert to heap / access heap
            inline operator ID3D12DescriptorHeap* ()
            {
                return m_ptrDescriptorHeap;
            }
            inline ID3D12DescriptorHeap* operator->()
            {
                return m_ptrDescriptorHeap;
            }

        private:
            // Heap itself
            ComPointer<ID3D12DescriptorHeap> m_ptrDescriptorHeap;

            // Start handles
            D3D12_CPU_DESCRIPTOR_HANDLE m_firstCpuHandle = { 0 };
            D3D12_GPU_DESCRIPTOR_HANDLE m_firstGpuHandle = { 0 };

            // Increment and count of handle
            UINT m_handleCount = 0;
            UINT m_handleIncrment = 0;

    };
}
