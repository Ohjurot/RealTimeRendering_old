#include "ModelBuffer.h"

D3D12_VERTEX_BUFFER_VIEW RTR::ModelPartView::CreateVertexBufferView(UINT64 sizeofVertex)
{
    D3D12_VERTEX_BUFFER_VIEW view;
    view.BufferLocation = ptrBuffer->GetAddress() + Offset;
    view.SizeInBytes = (UINT)Size;
    view.StrideInBytes = (UINT)sizeofVertex;
    return view;
}

D3D12_INDEX_BUFFER_VIEW RTR::ModelPartView::CreateIndexBufferView(UINT64 sizeofIndex)
{
    D3D12_INDEX_BUFFER_VIEW view;
    view.BufferLocation = ptrBuffer->GetAddress() + Offset;
    view.Format = sizeofIndex == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    view.SizeInBytes = (UINT)Size;
    return view;
}

RTR::ModelBuffer::ModelBuffer(UINT64 size)
{
    // Describe resource
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create resource
    RTR_CHECK_HRESULT(
        "ModellBuffer resource creation",
        GetD3D12DevicePtr()->CreateCommittedResource(GetD3D12DefaultHeapProperites(), D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_ptrResource))
    );
    SetResourceState(D3D12_RESOURCE_STATE_COMMON);

    // Set size
    m_size = size;
}

bool RTR::ModelBuffer::Alloc(UINT64 size, ModelPartView* ptrViewOut)
{
    bool canAlloc = m_size - m_usage >= size;
    if (canAlloc)
    {
        // Set details
        ptrViewOut->Offset = m_usage;
        ptrViewOut->Size = size;
        ptrViewOut->ptrBuffer = this;

        // Increment usage
        m_usage += size;
    }

    return canAlloc;
}

bool RTR::ModelBuffer::Upload(ModelPartView& view, UINT64 offset, UINT64 size, void* data, D3DUploadBuffer& uploader)
{
    bool canUpload = offset + size <= view.Size;
    if (canUpload)
    {
        uploader.CopyBufferData(data, size, (ID3D12Resource*)(view.ptrBuffer), view.Offset + offset);
    }

    return canUpload;
}
