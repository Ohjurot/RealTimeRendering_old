#include "D3DUploadBuffer.h"

RTR::D3DUploadBuffer::D3DUploadBuffer(UINT64 bufferSize) :
    m_uploadQueue(D3D12_COMMAND_LIST_TYPE_COPY)
{
    // Create command list and allocator pair
    RTR_CHECK_HRESULT(
        "Creating command allocator for upload command list",
        GetD3D12DevicePtr()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_uploadCommandAllocator))
    );

    // Create command list
    RTR_CHECK_HRESULT(
        "Creating command list for data uploading",
        GetD3D12DevicePtr()->CreateCommandList(NULL, D3D12_COMMAND_LIST_TYPE_COPY, m_uploadCommandAllocator, nullptr, IID_PPV_ARGS(&m_uploadCommandList))
    );

    // Describe heap for uploading
    D3D12_HEAP_PROPERTIES uploadHeapProps;
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProps.CreationNodeMask = NULL;
    uploadHeapProps.VisibleNodeMask = NULL;

    // Describe the resource itself
    D3D12_RESOURCE_DESC bufferDesc;
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    bufferDesc.Width = bufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create upload resource
    RTR_CHECK_HRESULT(
        "Creating upload resource buffer",
        GetD3D12DevicePtr()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_d3dResource))
    );

    // Map buffer
    RTR_CHECK_HRESULT(
        "Mapping upload buffer into CPU read/write area",
        m_d3dResource->Map(NULL, nullptr, (void**)&m_ptrMappedData)
    );

    // Set size
    m_bufferSize = bufferSize;
}

RTR::D3DUploadBuffer::~D3DUploadBuffer()
{
    // Unmap buffer
    if (m_ptrMappedData && m_d3dResource)
    {
        m_d3dResource->Unmap(NULL, nullptr);
        m_ptrMappedData = nullptr;
    }

    // Rollback cmd execution unit
    if(m_uploadQueue)
        m_uploadQueue.Flush();
    m_uploadQueue.~D3DQueue();
    m_uploadCommandList.release();
    m_uploadCommandAllocator.release();

    // Rollback resource
    m_d3dResource.release();
}

void* RTR::D3DUploadBuffer::ReserverUploadMemory(UINT64 reservationSize)
{
    unsigned char* ptrReservation = m_ptrMappedData;
    size_t bufferMemoryLeft = m_bufferSize - m_bufferUsage;

    // Only on valid mapping point and enough space
    if (m_ptrMappedData && bufferMemoryLeft >= reservationSize)
    {
        // Set pointer
        ptrReservation += m_bufferUsage;
        // Increment usage
        m_bufferUsage += reservationSize;

        // Count open reservations
        m_openMemoryReservations++;
    }

    return ptrReservation;
}

bool RTR::D3DUploadBuffer::CommitBufferCopy(void* ptrLocalMemory, UINT64 localMemorySize, ID3D12Resource* ptrTargetResource, UINT64 targetOffset /*= 0*/)
{
    bool canCopy = !m_isExecuting;
    if (canCopy)
    {
        // Copy a buffer region
        m_uploadCommandList->CopyBufferRegion(ptrTargetResource, targetOffset, m_d3dResource, ((unsigned char*)ptrLocalMemory) - m_ptrMappedData, localMemorySize);

        // Count open reservations
        m_openMemoryReservations--;
    }

    return canCopy;
}

bool RTR::D3DUploadBuffer::CommitTextureCopy(void* ptrLocalMemory, DXGI_FORMAT format, UINT width, UINT height, UINT depth, UINT rowStride, ID3D12Resource* ptrTargetTexture, UINT destX /*= 0*/, UINT destY /*= 0*/, UINT destZ /*= 0*/, UINT subresourceIndex /*= 0*/)
{
    bool canCopy = !m_isExecuting;
    if (canCopy)
    {
        // Source resource
        D3D12_TEXTURE_COPY_LOCATION srcLocation;
        srcLocation.pResource = m_d3dResource;
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLocation.PlacedFootprint.Offset = ((unsigned char*)ptrLocalMemory) - m_ptrMappedData;
        srcLocation.PlacedFootprint.Footprint.Format = format;
        srcLocation.PlacedFootprint.Footprint.Width = width;
        srcLocation.PlacedFootprint.Footprint.Height = height;
        srcLocation.PlacedFootprint.Footprint.Depth = depth;
        srcLocation.PlacedFootprint.Footprint.RowPitch = rowStride;

        // Destination resource
        D3D12_TEXTURE_COPY_LOCATION destLocation;
        destLocation.pResource = ptrTargetTexture;
        destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        destLocation.SubresourceIndex = subresourceIndex;

        // Copy box (copy all)
        D3D12_BOX copyBox;
        copyBox.left = 0;
        copyBox.top = 0;
        copyBox.front = 0;
        copyBox.bottom = height;
        copyBox.right = width;
        copyBox.back = depth;

        // Copy a texture region
        m_uploadCommandList->CopyTextureRegion(&destLocation, destX, destY, destZ, &srcLocation, &copyBox);

        // Count open reservations
        m_openMemoryReservations--;
    }

    return canCopy;
}

UINT RTR::D3DUploadBuffer::GetRequiredImageRowStride(UINT width, UINT bytesPerPixel)
{
    UINT rowSize = width * bytesPerPixel;
    if (rowSize % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
        rowSize += D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - (rowSize % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    return rowSize;
}

bool RTR::D3DUploadBuffer::CopyBufferData(void* ptrLocalMemory, UINT64 localMemorySize, ID3D12Resource* ptrTargetResource, UINT64 targetOffset /*= 0*/)
{
    bool success = false;

    // Reserve memory
    void* ptrMemory = ReserverUploadMemory(localMemorySize);
    if (ptrMemory)
    {
        // Copy to upload buffer
        memcpy(ptrMemory, ptrLocalMemory, localMemorySize);

        // Post copy
        success = CommitBufferCopy(ptrMemory, localMemorySize, ptrTargetResource, targetOffset);
    }

    return success;
}

bool RTR::D3DUploadBuffer::CopyTextureData(void* ptrLocalMemory, DXGI_FORMAT format, UINT width, UINT height, UINT depth, UINT rowStride, ID3D12Resource* ptrTargetTexture, UINT destX, UINT destY, UINT destZ, UINT subresourceIndex)
{
    bool success = false;

    // Adjust row stride
    UINT requiredRowStride = GetRequiredImageRowStride(width, rowStride / width);

    // Allocate memory
    unsigned char* ptrMemory = (unsigned char*)ReserverUploadMemory(requiredRowStride * height * depth);
    if (ptrMemory)
    {
        // Copy how required
        if (requiredRowStride == rowStride)
        {
            // Copy full texture
            memcpy(ptrMemory, ptrLocalMemory, rowStride * height * depth);
        }
        else
        {
            // Copy each row 
            for (UINT64 i = 0; i < height * depth; i++)
            {
                memcpy(&ptrMemory[requiredRowStride * i], &((unsigned char*)ptrLocalMemory)[rowStride * i], requiredRowStride);
            }
        }

        // Issue resource copy
        success = CommitTextureCopy(ptrMemory, format, width, height, depth, requiredRowStride, ptrTargetTexture, destX, destY, destZ, subresourceIndex);
    }

    return success;
}

void RTR::D3DUploadBuffer::Execute()
{
    if (!m_isExecuting)
    {
        // Unmap
        m_d3dResource->Unmap(0, nullptr);
        m_ptrMappedData = nullptr;

        // Dispatch command list on queue
        m_uploadCommandList->Close();
        m_queueWaitValue = m_uploadQueue.Execute(m_uploadCommandList);

        // Set
        m_isExecuting = true;

        // All reservations are closed now
        m_openMemoryReservations = 0;
    }
}

void RTR::D3DUploadBuffer::Wait()
{
    // Blocking wait
    m_uploadQueue.Wait(m_queueWaitValue);
    
    // Reset
    m_uploadCommandAllocator->Reset();
    m_uploadCommandList->Reset(m_uploadCommandAllocator, nullptr);
    m_bufferUsage = 0;
    m_isExecuting = false;

    // Remap resource
    RTR_CHECK_HRESULT(
        "Mapping upload buffer into CPU read/write area",
        m_d3dResource->Map(NULL, nullptr, (void**)&m_ptrMappedData)
    );
}

bool RTR::D3DUploadBuffer::CheckFinished()
{
    // Check finished state
    bool finished = m_uploadQueue.IsFinished(m_queueWaitValue);
    if (finished)
    {
        // Let this be handled by the wait function
        Wait();
    }

    return finished;
}
