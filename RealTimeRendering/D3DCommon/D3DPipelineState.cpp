#include "D3DPipelineState.h"

RTR::GfxPsoManipulator::GfxPsoManipulator(D3D12_GRAPHICS_PIPELINE_STATE_DESC* ptrDesc, DescriptorBuffer<D3D12_SO_DECLARATION_ENTRY, 16>* ptrSoElements, DescriptorBuffer<UINT, 16>* ptrSoStrides, DescriptorBuffer<D3D12_INPUT_ELEMENT_DESC, 16>* ptrInputElements, Shader** ppVS, Shader** ppDS, Shader** ppHS, Shader** ppGS, Shader** ppPS, Shader** ppMS, Shader** ppAS) :
    m_ptrDesc(ptrDesc), m_ptrSoElements(ptrSoElements), m_ptrSoStrides(ptrSoStrides), m_ptrInputElements(ptrInputElements),
    m_ppVS(ppVS), m_ppDS(ppDS), m_ppHS(ppHS), m_ppGS(ppGS), m_ppPS(ppPS), m_ppMS(ppMS), m_ppAS(ppAS)
{
    // Reset buffers
    m_ptrSoElements->Reset();
    m_ptrSoStrides->Reset();
    m_ptrInputElements->Reset();

    // Create default description
    ptrDesc->StreamOutput.NumEntries = 0;
    ptrDesc->StreamOutput.pSODeclaration = nullptr;
    ptrDesc->StreamOutput.NumStrides = 0;
    ptrDesc->StreamOutput.pBufferStrides = nullptr;
    ptrDesc->StreamOutput.RasterizedStream = 0;
    ptrDesc->BlendState.AlphaToCoverageEnable = false;
    ptrDesc->BlendState.IndependentBlendEnable = false;
    ptrDesc->BlendState.RenderTarget[0].BlendEnable = true;
    ptrDesc->BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    ptrDesc->BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    ptrDesc->BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    ptrDesc->BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    ptrDesc->BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    ptrDesc->BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    ptrDesc->BlendState.RenderTarget[0].LogicOpEnable = false;
    ptrDesc->BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    ptrDesc->BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    ptrDesc->SampleMask = 0xFFFFFFFF;
    ptrDesc->RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    ptrDesc->RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    ptrDesc->RasterizerState.FrontCounterClockwise = false;
    ptrDesc->RasterizerState.DepthBias = 0;
    ptrDesc->RasterizerState.DepthBiasClamp = 0.0f;
    ptrDesc->RasterizerState.SlopeScaledDepthBias = 0.0f;
    ptrDesc->RasterizerState.DepthClipEnable = false;
    ptrDesc->RasterizerState.MultisampleEnable = false;
    ptrDesc->RasterizerState.AntialiasedLineEnable = false;
    ptrDesc->RasterizerState.ForcedSampleCount = 0;
    ptrDesc->RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    ptrDesc->DepthStencilState.DepthEnable = false;
    ptrDesc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    ptrDesc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    ptrDesc->DepthStencilState.StencilEnable = false;
    ptrDesc->DepthStencilState.StencilReadMask = 0x00;
    ptrDesc->DepthStencilState.StencilWriteMask = 0x00;
    ptrDesc->DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    ptrDesc->DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    ptrDesc->DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    ptrDesc->DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    ptrDesc->DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    ptrDesc->DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    ptrDesc->DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    ptrDesc->DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    ptrDesc->InputLayout.NumElements = 0;
    ptrDesc->InputLayout.pInputElementDescs = nullptr;
    ptrDesc->IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    ptrDesc->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    ptrDesc->NumRenderTargets = 0;
    ptrDesc->DSVFormat = DXGI_FORMAT_UNKNOWN;
    ptrDesc->SampleDesc.Count = 1;
    ptrDesc->SampleDesc.Quality = 0;
}

RTR::GfxPsoManipulator::~GfxPsoManipulator()
{
    // Link in variadic size buffers
    m_ptrDesc->StreamOutput.NumEntries = m_ptrSoElements->GetUsage();
    m_ptrDesc->StreamOutput.pSODeclaration = m_ptrSoElements->operator[](0);
    m_ptrDesc->StreamOutput.NumStrides = m_ptrSoStrides->GetUsage();
    m_ptrDesc->StreamOutput.pBufferStrides = m_ptrSoStrides->operator[](0);
    m_ptrDesc->InputLayout.NumElements = m_ptrInputElements->GetUsage();
    m_ptrDesc->InputLayout.pInputElementDescs = m_ptrInputElements->operator[](0);
}

RTR::PipelineStateType RTR::GfxPsoManipulator::GetType() const noexcept
{
    return PipelineStateType::Graffics;
}

void RTR::GfxPsoManipulator::BindShader(ShaderType type, Shader* ptrShader)
{
    switch (type)
    {
        // Setup pointer on valid type
        case ShaderType::VS:
            *m_ppVS = ptrShader;
            break;
        case ShaderType::DS:
            *m_ppDS = ptrShader;
            break;
        case ShaderType::HS:
            *m_ppHS = ptrShader;
            break;
        case ShaderType::GS:
            *m_ppGS = ptrShader;
            break;
        case ShaderType::PS:
            *m_ppPS = ptrShader;
            break;
        case ShaderType::MS:
            *m_ppMS = ptrShader;
            break;
        case ShaderType::AS:
            *m_ppAS = ptrShader;
            break;

        // Throw execption if not reconized
        default:
            throw std::exception("Invalid shader type!");
    }
}

void RTR::GfxPsoManipulator::SOSetRasterizedStream(UINT streamIndex)
{
    m_ptrDesc->StreamOutput.RasterizedStream = streamIndex;
}

void RTR::GfxPsoManipulator::SOAddEntrie(UINT streamIndex, LPCSTR semanticName, UINT semanticIndex, BYTE outputSlot, BYTE startComponent, BYTE componentCount)
{
    auto* ptrSoDecl = m_ptrSoElements->PeekAndPushNext();
    ptrSoDecl->Stream = streamIndex;
    ptrSoDecl->SemanticName = semanticName;
    ptrSoDecl->SemanticIndex = semanticIndex;
    ptrSoDecl->OutputSlot = outputSlot;
    ptrSoDecl->StartComponent = startComponent;
    ptrSoDecl->ComponentCount = componentCount;
}

void RTR::GfxPsoManipulator::SOAddStride(UINT stride)
{
    auto* ptrSoStride = m_ptrSoStrides->PeekAndPushNext();
    *ptrSoStride = stride;
}

void RTR::GfxPsoManipulator::OMSetBlendState(bool enableAlphaToCoverage, bool enableIndependentBlend, UINT32 sampleMask)
{
    m_ptrDesc->BlendState.AlphaToCoverageEnable = enableAlphaToCoverage;
    m_ptrDesc->BlendState.IndependentBlendEnable = enableIndependentBlend;
    m_ptrDesc->SampleMask = sampleMask;
}

void RTR::GfxPsoManipulator::OMSetRTBlend(unsigned int rtIndex, D3D12_BLEND srcBlend, D3D12_BLEND destBlend, D3D12_BLEND_OP blendOp, D3D12_BLEND srcBlendAlpha, D3D12_BLEND destBlendAlpha, D3D12_BLEND_OP blendOpAlpha)
{
    if (rtIndex < 8)
    {
        m_ptrDesc->BlendState.RenderTarget[rtIndex].BlendEnable = true;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].SrcBlend = srcBlend;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].DestBlend = destBlend;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].BlendOp = blendOp;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].SrcBlendAlpha = srcBlendAlpha;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].DestBlendAlpha = destBlendAlpha;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].BlendOpAlpha = blendOpAlpha;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].LogicOpEnable = false;
    }
}

void RTR::GfxPsoManipulator::OMSetRTLogicBlend(unsigned int rtIndex, D3D12_LOGIC_OP operation)
{
    if (rtIndex < 8)
    {
        m_ptrDesc->BlendState.RenderTarget[rtIndex].BlendEnable = false;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].LogicOpEnable = true;
        m_ptrDesc->BlendState.RenderTarget[rtIndex].LogicOp = operation;
    }
}

void RTR::GfxPsoManipulator::OMSetRTBlendMask(unsigned int rtIndex, bool r, bool g, bool b, bool a)
{
    if (rtIndex < 8)
    {
        m_ptrDesc->BlendState.RenderTarget[rtIndex].RenderTargetWriteMask =
            (r ? D3D12_COLOR_WRITE_ENABLE_RED : 0) |
            (g ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0) |
            (b ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0) |
            (a ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
    }
}

void RTR::GfxPsoManipulator::OMSetRenderTargetFormat(unsigned int rtIndex, DXGI_FORMAT format)
{
    if (rtIndex < 8)
    {
        m_ptrDesc->RTVFormats[rtIndex] = format;
        m_ptrDesc->NumRenderTargets = std::max<unsigned int>(rtIndex + 1, m_ptrDesc->NumRenderTargets);
    }
}

void RTR::GfxPsoManipulator::OMSetDepthBufferStencilFormat(DXGI_FORMAT format)
{
    m_ptrDesc->DSVFormat = format;
}

void RTR::GfxPsoManipulator::RSSetFillMode(D3D12_FILL_MODE fillMode)
{
    m_ptrDesc->RasterizerState.FillMode = fillMode;
}

void RTR::GfxPsoManipulator::RSSetCullMode(D3D12_CULL_MODE cullMode, bool frontCounterClockwise)
{
    m_ptrDesc->RasterizerState.CullMode = cullMode;
    m_ptrDesc->RasterizerState.FrontCounterClockwise = frontCounterClockwise;
}

void RTR::GfxPsoManipulator::RSSetDepthBias(INT depthBias, float depthBiasClamp, float slopeScaledDepthBias)
{
    m_ptrDesc->RasterizerState.DepthBias = depthBias;
    m_ptrDesc->RasterizerState.DepthBiasClamp = depthBiasClamp;
    m_ptrDesc->RasterizerState.SlopeScaledDepthBias = slopeScaledDepthBias;
}

void RTR::GfxPsoManipulator::RSSetDepthClip(bool enable)
{
    m_ptrDesc->RasterizerState.DepthClipEnable = enable;
}

void RTR::GfxPsoManipulator::RSSetConservativeRaster(bool enable)
{
    m_ptrDesc->RasterizerState.ConservativeRaster = enable ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

void RTR::GfxPsoManipulator::RSSetAntialiasedLine(bool enabele)
{
    m_ptrDesc->RasterizerState.AntialiasedLineEnable = enabele;
}

void RTR::GfxPsoManipulator::EnableMSAA(unsigned quality, unsigned int count, unsigned int forcedSampleCount)
{
    m_ptrDesc->RasterizerState.MultisampleEnable = true;
    m_ptrDesc->RasterizerState.ForcedSampleCount = forcedSampleCount;
    m_ptrDesc->SampleDesc.Quality = quality;
    m_ptrDesc->SampleDesc.Count = count;
}

void RTR::GfxPsoManipulator::EnableDepthBuffer(D3D12_COMPARISON_FUNC depthFunction, bool writeDepth)
{
    m_ptrDesc->DepthStencilState.DepthEnable = true;
    m_ptrDesc->DepthStencilState.DepthFunc = depthFunction;
    m_ptrDesc->DepthStencilState.DepthWriteMask = writeDepth ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
}

void RTR::GfxPsoManipulator::EnableDepthStencil(UINT8 readMask, UINT8 writeMask)
{
    m_ptrDesc->DepthStencilState.StencilEnable = true;
    m_ptrDesc->DepthStencilState.StencilReadMask = readMask;
    m_ptrDesc->DepthStencilState.StencilWriteMask = writeMask;
}

void RTR::GfxPsoManipulator::DepthStenciSetFaces(bool frontFaces, D3D12_STENCIL_OP stencilFail, D3D12_STENCIL_OP stencilPassDepthFail, D3D12_STENCIL_OP stencilPassDepthPass, D3D12_COMPARISON_FUNC stencilComparisonFunc)
{
    // Get requested object
    D3D12_DEPTH_STENCILOP_DESC* ptrStencilOp = frontFaces ? 
        &m_ptrDesc->DepthStencilState.FrontFace : &m_ptrDesc->DepthStencilState.BackFace;

    // Setup 
    ptrStencilOp->StencilFailOp = stencilFail;
    ptrStencilOp->StencilDepthFailOp = stencilPassDepthFail;
    ptrStencilOp->StencilPassOp = stencilPassDepthPass;
    ptrStencilOp->StencilFunc = stencilComparisonFunc;
}

void RTR::GfxPsoManipulator::IAAddElement(const char* name, DXGI_FORMAT format)
{
    auto* ptrElement = m_ptrInputElements->PeekAndPushNext();
    ptrElement->SemanticName = name;
    ptrElement->SemanticIndex = m_ptrInputElements->GetUsage() - 1;
    ptrElement->Format = format;
    ptrElement->InputSlot = 0;
    ptrElement->AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    ptrElement->InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    ptrElement->InstanceDataStepRate = 0;
}

void RTR::GfxPsoManipulator::IAEnableIndexBufferStripeCut(bool is32bitWide)
{
    m_ptrDesc->IBStripCutValue = is32bitWide ? 
        D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
}

void RTR::GfxPsoManipulator::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
{
    m_ptrDesc->PrimitiveTopologyType = topologyType;
}

RTR::CsPsoManipulator::CsPsoManipulator(D3D12_COMPUTE_PIPELINE_STATE_DESC* ptrDesc, Shader** ppCS) :
    m_ptrDesc(ptrDesc), m_ppCS(ppCS)
{}

RTR::PipelineStateType RTR::CsPsoManipulator::GetType() const noexcept
{
    return PipelineStateType::Compute;
}

void RTR::CsPsoManipulator::BindShader(ShaderType type, Shader* ptrShader)
{
    switch (type)
    {
        // Setup pointer on valid type
        case ShaderType::CS:
            *m_ppCS = ptrShader;
            break;

        // Throw exception if not recognized
        default:
            throw std::exception("Invalid shader type!");
    }
}

RTR::D3DPipelineState::D3DPipelineState(PipelineStateType type) :
    m_type(type)
{}

bool RTR::D3DPipelineState::Bind(ID3D12GraphicsCommandList * ptrCmdList)
{
    // Check if directory changed
    const bool dirChange = DirWatchGetRevision() != m_lastDirIteration;

    // Call object dir function
    bool reloadPso = !m_ptrPso || __internal_Update(dirChange);
    if (reloadPso)
    {
        // Construct description
        if (m_type == PipelineStateType::Graffics)
        {
            // Call internal construct pso
            GfxPsoManipulator manipulator(&m_psoDescGfx, &m_gfxSoEnties, &m_gfxSoStrides, &m_gfxInputElements, 
                &m_ptrVSShader, &m_ptrDSShader, &m_ptrHSShader, &m_ptrGSShader, &m_ptrPSShader, &m_ptrMSShader, &m_ptrASShader);
            __internal_ConstructPso(&manipulator);
        }
        else
        {
            // Call internal construct pso
            CsPsoManipulator manipulator(&m_psoDescCompute, &m_ptrCSShader);
            __internal_ConstructPso(&manipulator);
        }
    }

    // Refresh shader (magically works for compute shaders!)
    if (dirChange)
    {
        if (m_ptrVSShader && m_ptrVSShader->GetShaderData()) reloadPso = m_ptrVSShader->Refresh() || reloadPso;
        if (m_ptrDSShader && m_ptrDSShader->GetShaderData()) reloadPso = m_ptrDSShader->Refresh() || reloadPso;
        if (m_ptrHSShader && m_ptrHSShader->GetShaderData()) reloadPso = m_ptrHSShader->Refresh() || reloadPso;
        if (m_ptrGSShader && m_ptrGSShader->GetShaderData()) reloadPso = m_ptrGSShader->Refresh() || reloadPso;
        if (m_ptrPSShader && m_ptrPSShader->GetShaderData()) reloadPso = m_ptrPSShader->Refresh() || reloadPso;
        //if (m_ptrMSShader && m_ptrMSShader->GetShaderData()) reloadPso = m_ptrMSShader->Refresh() || reloadPso;
        //if (m_ptrASShader && m_ptrASShader->GetShaderData()) reloadPso = m_ptrASShader->Refresh() || reloadPso;
    }

    // Reload if required
    if (reloadPso)
    {
        // Build pso
        if (m_type == PipelineStateType::Graffics)
        {
            bool loadFailed = false;

            // Load shader
            if (m_ptrVSShader && !m_ptrVSShader->GetShaderData()) loadFailed = m_ptrVSShader->Load() || loadFailed;
            if (m_ptrDSShader && !m_ptrDSShader->GetShaderData()) loadFailed = m_ptrDSShader->Load() || loadFailed;
            if (m_ptrHSShader && !m_ptrHSShader->GetShaderData()) loadFailed = m_ptrHSShader->Load() || loadFailed;
            if (m_ptrGSShader && !m_ptrGSShader->GetShaderData()) loadFailed = m_ptrGSShader->Load() || loadFailed;
            if (m_ptrPSShader && !m_ptrPSShader->GetShaderData()) loadFailed = m_ptrPSShader->Load() || loadFailed;
            //if (m_ptrMSShader && !m_ptrMSShader->GetShaderData()) loadFailed = m_ptrMSShader->Load() || loadFailed;
            //if (m_ptrASShader && !m_ptrASShader->GetShaderData()) loadFailed = m_ptrASShader->Load() || loadFailed;

            // Update shader links
            m_psoDescGfx.VS.BytecodeLength = m_ptrVSShader ? m_ptrVSShader->GetShaderSize() : 0;
            m_psoDescGfx.VS.pShaderBytecode = m_ptrVSShader ? m_ptrVSShader->GetShaderData() : nullptr;
            m_psoDescGfx.DS.BytecodeLength = m_ptrDSShader ? m_ptrDSShader->GetShaderSize() : 0;
            m_psoDescGfx.DS.pShaderBytecode = m_ptrDSShader ? m_ptrDSShader->GetShaderData() : nullptr;
            m_psoDescGfx.HS.BytecodeLength = m_ptrHSShader ? m_ptrHSShader->GetShaderSize() : 0;
            m_psoDescGfx.HS.pShaderBytecode = m_ptrHSShader ? m_ptrHSShader->GetShaderData() : nullptr;
            m_psoDescGfx.GS.BytecodeLength = m_ptrGSShader ? m_ptrGSShader->GetShaderSize() : 0;
            m_psoDescGfx.GS.pShaderBytecode = m_ptrGSShader ? m_ptrGSShader->GetShaderData() : nullptr;
            m_psoDescGfx.PS.BytecodeLength = m_ptrPSShader ? m_ptrPSShader->GetShaderSize() : 0;
            m_psoDescGfx.PS.pShaderBytecode = m_ptrPSShader ? m_ptrPSShader->GetShaderData() : nullptr;
            //m_psoDescGfx.MS.BytecodeLength = m_ptrMSShader ? m_ptrMSShader->GetShaderSize() : 0;              STILL NOT IN THE REGUALAR WINDOWS SDK?!  
            //m_psoDescGfx.MS.pShaderBytecode = m_ptrMSShader ? m_ptrMSShader->GetShaderData() : nullptr;       IT IS BUT IN D3DX12_MESH_SHADER_PIPELINE_STATE_DESC lol no regular d3d12.h construct?!
            //m_psoDescGfx.AS.BytecodeLength = m_ptrASShader ? m_ptrASShader->GetShaderSize() : 0;
            //m_psoDescGfx.AS.pShaderBytecode = m_ptrASShader ? m_ptrASShader->GetShaderData() : nullptr;

            // Update and build pso
            m_psoDescGfx.NodeMask = 0;
            m_psoDescGfx.CachedPSO.CachedBlobSizeInBytes = 0;
            m_psoDescGfx.CachedPSO.pCachedBlob = nullptr;
            m_psoDescGfx.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

            // Only con continue on succeed
            if (!loadFailed)
            {
                // Release old objects
                m_ptrPso.release();
                m_ptrRootSignature.release();

                // Build new objects
                RTR_CHECK_HRESULT(
                    "Creating root signature",
                    GetD3D12DevicePtr()->CreateRootSignature(0, m_ptrVSShader->GetShaderRootData(), m_ptrVSShader->GetShaderRootSize(), IID_PPV_ARGS(&m_ptrRootSignature))
                );
                m_psoDescCompute.pRootSignature = m_ptrRootSignature;

                RTR_CHECK_HRESULT(
                    "Creating GFX pso",
                    GetD3D12DevicePtr()->CreateGraphicsPipelineState(&m_psoDescGfx, IID_PPV_ARGS(&m_ptrPso))
                );
            }
        }
        else
        {
            bool loadFailed = false;

            // Load shader
            if (m_ptrCSShader && !m_ptrCSShader->GetShaderData()) loadFailed = m_ptrCSShader->Load();

            // Update shader links
            m_psoDescCompute.CS.BytecodeLength = m_ptrCSShader ? m_ptrCSShader->GetShaderSize() : 0;
            m_psoDescCompute.CS.pShaderBytecode = m_ptrCSShader ? m_ptrCSShader->GetShaderData() : nullptr;

            // Update and build pso
            m_psoDescCompute.NodeMask = 0;
            m_psoDescCompute.CachedPSO.CachedBlobSizeInBytes = 0;
            m_psoDescCompute.CachedPSO.pCachedBlob = nullptr;
            m_psoDescCompute.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

            // Only con continue on succeed
            if (!loadFailed)
            {
                // Release old objects
                m_ptrPso.release();
                m_ptrRootSignature.release();

                // Build new objects
                RTR_CHECK_HRESULT(
                    "Creating root signature",
                    GetD3D12DevicePtr()->CreateRootSignature(0, m_ptrVSShader->GetShaderRootData(), m_ptrVSShader->GetShaderRootSize(), IID_PPV_ARGS(&m_ptrRootSignature))
                );
                m_psoDescCompute.pRootSignature = m_ptrRootSignature;

                RTR_CHECK_HRESULT(
                    "Creating compute pso",
                    GetD3D12DevicePtr()->CreateComputePipelineState(&m_psoDescCompute, IID_PPV_ARGS(&m_ptrPso))
                );
            }
        }
    }

    // Reflect dir changes
    m_lastDirIteration = DirWatchGetRevision();

    // Bind to command list
    if (m_ptrPso && m_ptrRootSignature)
    {
        ptrCmdList->SetPipelineState(m_ptrPso);
        if (m_type == PipelineStateType::Graffics)
        {
            ptrCmdList->SetGraphicsRootSignature(m_ptrRootSignature);
        }
        else
        {
            ptrCmdList->SetComputeRootSignature(m_ptrRootSignature);
        }

        // OK
        return true;
    }

    // FAILED
    return false;
}
