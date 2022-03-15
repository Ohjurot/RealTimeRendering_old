#include "MatrixBuffer.h"

RTR::MatrixBuffer::MatrixBuffer(UINT count)
{
    // Allocate cpu sided buffer
    m_matricies = (DirectX::XMMATRIX*)_aligned_malloc(count * sizeof(DirectX::XMMATRIX), 16);

    // Describe d3d12 resource
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = sizeof(DirectX::XMMATRIX) * count;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create d3d12 resource
    RTR_CHECK_HRESULT(
        "Creating matrix resource buffer",
        GetD3D12DevicePtr()->CreateCommittedResource(GetD3D12DefaultHeapProperites(), D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_ptrResource))
    );

    // Set count
    m_count = count;
}

RTR::Matrix RTR::MatrixBuffer::GetMatrix()
{
    // Invalid matrix
    Matrix m;
    m.offset = -1;

    // Check if possible
    if (m_usage < m_count)
    {
        m.offset = m_usage++;
    }

    return m;
}

void RTR::MatrixBuffer::UpdateCPU(Matrix& mat)
{
    m_matricies[mat.offset] = DirectX::XMMatrixTranspose(mat.M);
}

bool RTR::MatrixBuffer::UpdateGPU(D3DUploadBuffer& uploader)
{
    return uploader.CopyBufferData(m_matricies, sizeof(DirectX::XMMATRIX) * m_count, Get());
}
