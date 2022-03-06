#pragma once

#include <WinInclude.h>
#include <Util/HrException.h>
#include <D3DCommon/D3DPipelineState.h>

namespace RTR
{
    // Root configuration entry
    enum class RootConfigurationEntry_t
    {
        // Invalid entry
        Invalid = 0,
        // Root Constant
        RootConstant,
        // CBV
        ConstantBufferView,
        // SRV
        ShaderResourceView,
        // UAV
        UnorderedAccessView,
        // Descriptor Table
        DescriptorTable,
    };

    // Defines a root element
    struct RootConfigurationEntry
    {
        // Construct
        inline RootConfigurationEntry() {};

        // Entity type
        RootConfigurationEntry_t type = RootConfigurationEntry_t::Invalid;
        
        // Data container
        union 
        {
            // For type == RootConstant
            struct
            {
                // Count of 32-Bit values
                UINT valueCount = 0;
                // Pointer to cpu source data
                const void* ptrData = nullptr;
            }
            RootConstant;

            // For type == ConstantBufferView
            struct
            {
                // Address of CBV data
                D3D12_GPU_VIRTUAL_ADDRESS dataAddress = 0;
            }
            ConstantBufferView;

            // For type == ShaderResourceView
            struct
            {
                // Address of SRV data
                D3D12_GPU_VIRTUAL_ADDRESS dataAddress = 0;
            }
            ShaderResourceView;

            // For type == UnorderedAccessView
            struct
            {
                // Address of the UAV data
                D3D12_GPU_VIRTUAL_ADDRESS dataAddress = 0;
            }
            UnorderedAccessView;

            // For type == DescriptorTable
            struct
            {
                // Address of the base descriptor
                D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor = {0};
            }
            DescriptorTable;
        };

        // Factory functions
        inline static RootConfigurationEntry MakeRootConstant(unsigned int num, const void* data)
        {
            RootConfigurationEntry e;
            e.type = RootConfigurationEntry_t::RootConstant;
            e.RootConstant = {num, data};
            return e;
        }
        inline static RootConfigurationEntry MakeConstantBufferView(D3D12_GPU_VIRTUAL_ADDRESS addr)
        {
            RootConfigurationEntry e;
            e.type = RootConfigurationEntry_t::ConstantBufferView;
            e.ConstantBufferView.dataAddress = addr;
            return e;
        }
        inline static RootConfigurationEntry MakeShaderResourceView(D3D12_GPU_VIRTUAL_ADDRESS addr)
        {
            RootConfigurationEntry e;
            e.type = RootConfigurationEntry_t::ShaderResourceView;
            e.ShaderResourceView.dataAddress = addr;
            return e;
        }
        inline static RootConfigurationEntry MakeUnorderedAccessView(D3D12_GPU_VIRTUAL_ADDRESS addr)
        {
            RootConfigurationEntry e;
            e.type = RootConfigurationEntry_t::UnorderedAccessView;
            e.UnorderedAccessView.dataAddress = addr;
            return e;
        }
        inline static RootConfigurationEntry MakeDescriptorTable(D3D12_GPU_DESCRIPTOR_HANDLE handle)
        {
            RootConfigurationEntry e;
            e.type = RootConfigurationEntry_t::DescriptorTable;
            e.DescriptorTable.baseDescriptor = handle;
            return e;
        }
    };

    // Root signature configuration
    class RootConfiguration
    {
        public:
            // Construct
            inline RootConfiguration(PipelineStateType type) : m_type(type) {};
            RootConfiguration(const RootConfiguration&) = default;
            RootConfiguration(PipelineStateType type, unsigned int count...);

            // Push back new entry
            bool PushBack(RootConfigurationEntry other) noexcept;

            // Get element to edit
            RootConfigurationEntry& operator[](unsigned int index);

            // Current size
            inline unsigned int Size() const noexcept
            {
                return m_usage;
            }

            // Total max capacity
            static constexpr unsigned int Capacity()
            {
                return 32;
            }

            // Bind to CommandList
            bool Bind(ID3D12GraphicsCommandList* ptrCmdList) const;

        private:
            // Binding helper
            void __bindHelper(ID3D12GraphicsCommandList* ptrCmdList, unsigned int index, const RootConfigurationEntry* ptrEntry) const;

        private:
            // Type of list
            PipelineStateType m_type = PipelineStateType::Invalid;

            // Entry buffer
            RootConfigurationEntry m_entrys[32] = {};

            // Entry usage
            unsigned int m_usage = 0;
    };
}
