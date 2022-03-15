#pragma once

#include <Util/ComPointer.h>
#include <D3DMemory/D3DResource.h>
#include <D3DMemory/D3DUploadBuffer.h>

#include <DirectXMath.h>

namespace RTR
{
    // View of a model part (vertices / indices / ...)
    struct Matrix
    {
        // User modifiable matrix copy (will be translated on UpdateCPU call)
        DirectX::XMMATRIX M;

        // Local offset
        UINT offset = 0;
    };

    // Big stack that can hold model data (vertices and indices)
    class MatrixBuffer : public D3DResource
    {
        public:
            // Construct
            MatrixBuffer() = delete;
            MatrixBuffer(const MatrixBuffer&) = delete;
            MatrixBuffer(UINT count);

            // Copy
            MatrixBuffer& operator=(const MatrixBuffer&) = delete;

            // Allocate a fresh new matrix
            Matrix GetMatrix();
            // Updates a change cpu matrix
            void UpdateCPU(Matrix& mat);
            // Update all matrix data to the gpu
            bool UpdateGPU(D3DUploadBuffer& uploader);

        private:
            // List of gpu read matrices
            DirectX::XMMATRIX* m_matricies = nullptr;

            // Total count and current usage of the buffer
            UINT m_count = 0;
            UINT m_usage = 0;
    };
}
