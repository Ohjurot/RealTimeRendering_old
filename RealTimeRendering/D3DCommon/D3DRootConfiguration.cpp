#include "D3DRootConfiguration.h"

RTR::RootConfiguration::RootConfiguration(PipelineStateType type, unsigned int count ...) :
    m_type(type)
{
    // Variadic start
    va_list vaList;
    va_start(vaList, count);

    for (unsigned int i = 0; i < count; i++)
    {
        if (!PushBack(va_arg(vaList, RootConfigurationEntry)))
        {
            throw std::exception("Failed to add all constructor arguments to RootConfiguration");
        }
    }

    // End va
    va_end(vaList);
}

bool RTR::RootConfiguration::PushBack(RootConfigurationEntry other) noexcept
{
    bool canPushBack = m_usage < 32;

    if (canPushBack)
    {
        // Move element
        m_entrys[m_usage++] = std::move(other);
    }

    return canPushBack;
}

RTR::RootConfigurationEntry& RTR::RootConfiguration::operator[](unsigned int index)
{
    // Check bounds
    if (index >= m_usage)
    {
        throw std::exception("Index out of range! In RootConfiguration::operator[]");
    }

    // Return ref
    return m_entrys[index];
}

bool RTR::RootConfiguration::Bind(ID3D12GraphicsCommandList* ptrCmdList) const
{
    // Only on valid instance
    if (m_type != PipelineStateType::Invalid)
    {
        // Bind all
        for (unsigned int i = 0; i < m_usage; i++)
        {
            __bindHelper(ptrCmdList, i, &m_entrys[i]);
        }

        // ok
        return true;
    }

    // Fallback
    return false;
}

void RTR::RootConfiguration::__bindHelper(ID3D12GraphicsCommandList* ptrCmdList, unsigned int index, const RootConfigurationEntry* ptrEntry) const
{
    // Swtich on type
    switch (ptrEntry->type)
    {
        // Root Constant
        case RootConfigurationEntry_t::RootConstant:
            m_type == PipelineStateType::Graffics ? 
                ptrCmdList->SetGraphicsRoot32BitConstants(index, ptrEntry->RootConstant.valueCount, ptrEntry->RootConstant.ptrData, 0) : 
                ptrCmdList->SetComputeRoot32BitConstants(index, ptrEntry->RootConstant.valueCount, ptrEntry->RootConstant.ptrData, 0);
            break;

        // ConstantBufferView
        case RootConfigurationEntry_t::ConstantBufferView:
            m_type == PipelineStateType::Graffics ?
                ptrCmdList->SetGraphicsRootConstantBufferView(index, ptrEntry->ConstantBufferView.dataAddress) :
                ptrCmdList->SetComputeRootConstantBufferView(index, ptrEntry->ConstantBufferView.dataAddress);
            break;

        // ShaderResourceView
        case RootConfigurationEntry_t::ShaderResourceView:
            m_type == PipelineStateType::Graffics ?
                ptrCmdList->SetGraphicsRootShaderResourceView(index, ptrEntry->ShaderResourceView.dataAddress) :
                ptrCmdList->SetComputeRootShaderResourceView(index, ptrEntry->ShaderResourceView.dataAddress);
            break;

        // ShaderResourceView
        case RootConfigurationEntry_t::UnorderedAccessView:
            m_type == PipelineStateType::Graffics ?
                ptrCmdList->SetGraphicsRootUnorderedAccessView(index, ptrEntry->UnorderedAccessView.dataAddress) :
                ptrCmdList->SetComputeRootUnorderedAccessView(index, ptrEntry->UnorderedAccessView.dataAddress);
            break;

        // DescriptorTable
        case RootConfigurationEntry_t::DescriptorTable:
            m_type == PipelineStateType::Graffics ?
                ptrCmdList->SetGraphicsRootDescriptorTable(index, ptrEntry->DescriptorTable.baseDescriptor) :
                ptrCmdList->SetComputeRootDescriptorTable(index, ptrEntry->DescriptorTable.baseDescriptor);
            break;

        // Invalid
        case RootConfigurationEntry_t::Invalid:
            [[fallthrough]];
        default:
            break;
    }
}
