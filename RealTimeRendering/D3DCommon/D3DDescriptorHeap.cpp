#include "D3DDescriptorHeap.h"

RTR::D3DDescriptorRange::D3DDescriptorRange(D3DDescriptorHandle first, UINT count, UINT increment) :
    m_first(first), m_count(count), m_increment(increment)
{ }

RTR::D3DDescriptorHandle RTR::D3DDescriptorRange::At(UINT idx)
{
    // Index was out of bounds
    if (idx >= m_count)
        throw std::exception("Index out of bounds!");

    return D3DDescriptorHandle({ m_first.cpu().ptr + m_increment * idx }, { m_first.gpu().ptr + m_increment * idx });
}

RTR::D3DDescriptorHeap::D3DDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
{
    // Describe heap
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.Type = type;
    desc.NumDescriptors = count;
    desc.NodeMask = NULL;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    
    // Create heap
    RTR_CHECK_HRESULT(
        "Creating descriptor heap",
        GetD3D12DevicePtr()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_ptrDescriptorHeap))
    );

    // Get begin handles
    m_firstCpuHandle =  m_ptrDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_firstGpuHandle =  m_ptrDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

    // Set count and increment
    m_handleCount = count;
    m_handleIncrment = GetD3D12HandleIncrement(type);
}

RTR::D3DDescriptorHeap& RTR::D3DDescriptorHeap::operator=(D3DDescriptorHeap&& other) noexcept
{
    this->~D3DDescriptorHeap();
    memcpy(this, &other, sizeof(D3DDescriptorHeap));
    memset(&other, 0x0, sizeof(D3DDescriptorHeap));
    return *this;
}

RTR::D3DDescriptorHandle RTR::D3DDescriptorHeap::First()
{
    return D3DDescriptorHandle(m_firstGpuHandle, m_firstCpuHandle);
}

RTR::D3DDescriptorHandle RTR::D3DDescriptorHeap::At(size_t idx)
{
    // Index was out of bounds
    if (idx >= m_handleCount)
        throw std::exception("Index out of bounds!");

    // Craft handle
    D3D12_GPU_DESCRIPTOR_HANDLE hGpu = { m_firstGpuHandle.ptr + m_handleIncrment * idx };
    D3D12_CPU_DESCRIPTOR_HANDLE hCpu = { m_firstCpuHandle.ptr + m_handleIncrment * idx };
    return D3DDescriptorHandle(hGpu, hCpu);
}

RTR::D3DDescriptorRange RTR::D3DDescriptorHeap::Range(size_t idx, size_t count)
{
    // Index was out of bounds
    if (idx + count >= m_handleCount)
        throw std::exception("Index out of bounds!");

    // Create range
    return D3DDescriptorRange(At(idx), count, m_handleIncrment);
}
