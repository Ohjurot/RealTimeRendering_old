#include "ModelContext.h"

RTR::ModelContext::ModelContext(UINT64 memoryBudget) :
    m_geometryDataBuffer(memoryBudget) // For now give all memory to the geometry data
{ }

RTR::ModelInfo RTR::ModelContext::LoadModel(const char* filePath, D3DUploadBuffer& uploader)
{
    // Start with an info with valid index and invalid size
    ModelInfo infoOut;
    infoOut.idx = m_sets.size();
    infoOut.count = 0;

    // Open an assimp scene
    Assimp::Importer asImport;
    const aiScene* asScene = asImport.ReadFile(filePath,
        aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType
    );

    // Process data form scene
    if (asScene)
    {
        // We need to load multiple meshes
        infoOut.count = asScene->mNumMeshes;

        // Load the meshes
        for (size_t i = 0; i < infoOut.count; i++)
        {
            const aiMesh* asMesh = asScene->mMeshes[i];

            // Get global count
            size_t indexCount = 0, vertexCount = 0;
            vertexCount = asMesh->mNumVertices;
            for (size_t i = 0; i < asMesh->mNumFaces; i++)
                indexCount += asMesh->mFaces->mNumIndices;

            // Compute required size
            size_t memorySizeVertices = sizeof(float) * 4 * vertexCount;
            size_t memorySizeIndices = sizeof(unsigned int) * indexCount;

            // Allocate memory buffers on gpu buffer
            ModelPartView vertexPart, indexPart;
            if (
                m_geometryDataBuffer.Alloc(memorySizeVertices, &vertexPart) &&
                m_geometryDataBuffer.Alloc(memorySizeIndices, &indexPart)
                )
            {
                // Create set
                MeshInfo set;
                set.name = asMesh->mName.C_Str();
                set.vertexBuffer = vertexPart;
                set.indexBuffer = indexPart;
                set.indexCount = indexCount;

                // Upload vertex data
                unsigned char* vertexData = (unsigned char*)uploader.ReserverUploadMemory(memorySizeVertices);
                if (vertexData)
                {
                    memset(vertexData, 0x0, memorySizeVertices);
                    size_t offset = 0;
                    for (size_t i = 0; i < asMesh->mNumVertices; i++)
                    {
                        memcpy(&vertexData[offset], &asMesh->mVertices[i], sizeof(aiVector3D));
                        offset += sizeof(float) * 4;
                    }

                    // Commit upload
                    uploader.CommitBufferCopy(vertexData, memorySizeVertices, vertexPart.ptrBuffer->Get(), vertexPart.Offset);
                }

                // Upload index data
                unsigned char* indexData = (unsigned char*)uploader.ReserverUploadMemory(memorySizeIndices);
                if (indexData)
                {
                    // Copy all faces
                    size_t offset = 0;
                    for (size_t i = 0; i < asMesh->mNumFaces; i++)
                    {
                        size_t copySize = sizeof(unsigned int) * asMesh->mFaces[i].mNumIndices;
                        memcpy(&indexData[offset], asMesh->mFaces[i].mIndices, copySize);
                        offset += copySize;
                    }

                    // Commit upload
                    uploader.CommitBufferCopy(indexData, memorySizeIndices, indexPart.ptrBuffer->Get(), indexPart.Offset);
                }

                // Store set
                m_sets.push_back(std::move(set));
            }
        }
    }

    return infoOut;
}

RTR::MeshInfo RTR::ModelContext::GetMeshInfo(ModelInfo& modelInfo, size_t idx)
{
    // Check offset
    if (idx >= modelInfo.count)
        throw std::exception("Mesh index out of bounds");
    return m_sets[modelInfo.idx + idx];
}
