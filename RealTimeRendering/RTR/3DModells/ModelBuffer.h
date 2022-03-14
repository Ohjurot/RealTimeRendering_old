#pragma once

#include <Util/ComPointer.h>
#include <D3DMemory/D3DResource.h>
#include <D3DMemory/D3DUploadBuffer.h>

namespace RTR
{
    // View of a model part (vertices / indices / ...)
    struct ModelPartView
    {
        D3DResource* ptrBuffer;
        UINT64 Offset;
        UINT64 Size;

        // Helper functions
        D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(UINT64 sizeofVertex);
        D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView(UINT64 sizeofIndex);
    };

    // Big stack that can hold model data (vertices and indices)
    class ModelBuffer : public D3DResource
    {
        public:
            // Construct
            ModelBuffer() = delete;
            ModelBuffer(const ModelBuffer&) = delete;
            ModelBuffer(UINT64 size);

            // Copy
            ModelBuffer& operator=(const ModelBuffer&) = delete;

            // Allocate space on the buffer
            bool Alloc(UINT64 size, ModelPartView* ptrViewOut);
            // Upload data to an allocated are
            static bool Upload(ModelPartView& view, UINT64 offset, UINT64 size, void* data, D3DUploadBuffer& uploader);

        private:
            // Size and current usage of the buffer
            UINT64 m_size = 0;
            UINT64 m_usage = 0;
    };
}
