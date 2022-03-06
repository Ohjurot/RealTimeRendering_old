#pragma once

#include <WinInclude.h>

#include <Util/ComPointer.h>
#include <Util/HrException.h>
#include <Util/DirWatcher.h>
#include <Util/Shader.h>

#include <D3DCommon/D3DInstance.h>

#include <type_traits>

namespace RTR
{
    // Type of PSO
    enum class PipelineStateType
    {
        // Invalid state
        Invalid,
        // Graffics pipeline state
        Graffics,
        // Compute pipeline state
        Compute,
    };

    // D3D12 Descriptor buffer
    template<typename T, unsigned int N>
    class DescriptorBuffer
    {
        public:
            // Constructor
            DescriptorBuffer() = default;
            DescriptorBuffer(const DescriptorBuffer&) = delete;

            // Assign
            DescriptorBuffer& operator=(const DescriptorBuffer&) = delete;

            // Status functions
            unsigned int constexpr GetSize() const noexcept
            {
                return N;
            }
            unsigned int GetUsage() const noexcept
            {
                return m_used;
            }
            bool HasSpace() const noexcept
            {
                return GetUsage() < GetSize();
            }

            // Worker functions
            T* PeekNext() noexcept
            {
                T* ptrData = nullptr;
                if (HasSpace())
                {
                    ptrData = &m_data[m_used];
                }

                return ptrData;
            }
            void PushNext() noexcept
            {
                if (HasSpace())
                {
                    m_used++;
                }
            }
            T* PeekAndPushNext() noexcept
            {
                T* ptrData = PeekNext();
                PushNext();
                return ptrData;
            }

            // Access functions
            T* operator[](unsigned int idx) noexcept
            {
                T* ptrData = nullptr;
                if (idx < GetUsage())
                {
                    ptrData = &m_data[idx];
                }

                return ptrData;
            }

            // Reset function
            void Reset() noexcept
            {
                m_used = 0;
            }

        private:
            // Data buffer
            T m_data[N];
            // Used data
            unsigned int m_used = 0;
    };

    // Pso manipulator interface
    class IPsoManipulator
    {
        public:
            virtual PipelineStateType GetType() const noexcept = 0;
            virtual void BindShader(ShaderType type, Shader* ptrShader) = 0;
    };

    // Manipulator GFX
    class GfxPsoManipulator : public IPsoManipulator
    {
        public:
            // Construct
            GfxPsoManipulator() = delete;
            GfxPsoManipulator(const GfxPsoManipulator&) = delete;
            GfxPsoManipulator(D3D12_GRAPHICS_PIPELINE_STATE_DESC* ptrDesc, DescriptorBuffer<D3D12_SO_DECLARATION_ENTRY, 16>* ptrSoElements, DescriptorBuffer<UINT, 16>* ptrSoStrides, DescriptorBuffer<D3D12_INPUT_ELEMENT_DESC, 16>* ptrInputElements, Shader** ppVS, Shader** ppDS, Shader** ppHS, Shader** ppGS, Shader** ppPS, Shader** ppMS, Shader** ppAS);

            // Destruct
            ~GfxPsoManipulator();

            // Type function
            PipelineStateType GetType() const noexcept override;
            // Bind shader
            void BindShader(ShaderType type, Shader* ptrShader) override;

            // MSDN: The index number of the stream to be sent to the rasterizer stage.
            void SOSetRasterizedStream(UINT streamIndex);
            // Add an entrie to the stream output
            void SOAddEntrie(UINT streamIndex, LPCSTR semanticName, UINT semanticIndex, BYTE outputSlot = 0, BYTE startComponent = 0, BYTE componentCount = 4);
            // Add an stride to the stream output
            void SOAddStride(UINT stride);
            // Set global blend state
            void OMSetBlendState(bool enableAlphaToCoverage = false, bool enableIndependentBlend = false, UINT32 sampleMask = 0xFFFFFFFF);
            // Set per render target color blende
            void OMSetRTBlend(unsigned int rtIndex, D3D12_BLEND srcBlend, D3D12_BLEND destBlend, D3D12_BLEND_OP blendOp, D3D12_BLEND srcBlendAlpha, D3D12_BLEND destBlendAlpha, D3D12_BLEND_OP blendOpAlpha);
            // Set per render target logic blende
            void OMSetRTLogicBlend(unsigned int rtIndex, D3D12_LOGIC_OP operation);
            // Set per render target blend mask
            void OMSetRTBlendMask(unsigned int rtIndex, bool r = true, bool g = true, bool b = true, bool a = true);
            // Set rendertarget format
            void OMSetRenderTargetFormat(unsigned int rtIndex, DXGI_FORMAT format);
            // Sets the depth buffers & stencil format
            void OMSetDepthBufferStencilFormat(DXGI_FORMAT format);
            // Sets the rastreriers fill mode
            void RSSetFillMode(D3D12_FILL_MODE fillMode);
            // Sets the rastersizers cull mode
            void RSSetCullMode(D3D12_CULL_MODE cullMode, bool frontCounterClockwise = false);
            // Set depth biasisng
            void RSSetDepthBias(INT depthBias, float depthBiasClamp, float slopeScaledDepthBias);
            // Enabe depth clipping
            void RSSetDepthClip(bool enable);
            // Enabele ConservativeRaster
            void RSSetConservativeRaster(bool enable);
            // Antialiassed line
            void RSSetAntialiasedLine(bool enabele);
            // Enable MSAA on pipeline
            void EnableMSAA(unsigned quality, unsigned int count, unsigned int forcedSampleCount = 0);
            // Enable depth buffering
            void EnableDepthBuffer(D3D12_COMPARISON_FUNC depthFunction, bool writeDepth = true);
            // Enable depth stencil (setup stencil with the acording functions)
            void EnableDepthStencil(UINT8 readMask, UINT8 writeMask);
            // Set operations for faces
            void DepthStenciSetFaces(bool frontFaces, D3D12_STENCIL_OP stencilFail, 
                D3D12_STENCIL_OP stencilPassDepthFail, D3D12_STENCIL_OP stencilPassDepthPass, 
                D3D12_COMPARISON_FUNC stencilComparisonFunc);
            // Add a per vertex auto offset input element at slot 1
            void IAAddElement(const char* name, DXGI_FORMAT format);
            // Enable strip cut (by default 32bit max if parameter false 16bit max)
            void IAEnableIndexBufferStripeCut(bool is32bitWide = true);
            // Set the type of the primitive topology
            void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

        private:
            // Pointer to data to be manimpulated
            D3D12_GRAPHICS_PIPELINE_STATE_DESC* m_ptrDesc;
            DescriptorBuffer<D3D12_SO_DECLARATION_ENTRY, 16>* m_ptrSoElements;
            DescriptorBuffer<UINT, 16>* m_ptrSoStrides;
            DescriptorBuffer<D3D12_INPUT_ELEMENT_DESC, 16>* m_ptrInputElements;

            // Shader linking
            Shader** m_ppVS = nullptr;
            Shader** m_ppDS = nullptr;
            Shader** m_ppHS = nullptr;
            Shader** m_ppGS = nullptr;
            Shader** m_ppPS = nullptr;
            Shader** m_ppMS = nullptr;
            Shader** m_ppAS = nullptr;
    };

    // Manipulator CS
    class CsPsoManipulator : public IPsoManipulator
    {
        public:
            // Construct
            CsPsoManipulator() = delete;
            CsPsoManipulator(const CsPsoManipulator&) = delete;
            CsPsoManipulator(D3D12_COMPUTE_PIPELINE_STATE_DESC* ptrDesc, Shader** ppCS);

            // Interface
            PipelineStateType GetType() const noexcept override;
            void BindShader(ShaderType type, Shader* ptrShader) override;

        private:
            D3D12_COMPUTE_PIPELINE_STATE_DESC* m_ptrDesc;

            // Shader linking
            Shader** m_ppCS = nullptr;
    };

    // Pipeline state
    class D3DPipelineState
    {
        public:
            // Construct
            D3DPipelineState() = delete;
            D3DPipelineState(const D3DPipelineState&) = delete;
            D3DPipelineState(PipelineStateType type);

            // Assign
            D3DPipelineState& operator=(const D3DPipelineState&) = delete;

            // Bind functions
            bool Bind(ID3D12GraphicsCommandList* ptrCmdList);

        protected:
            // === Functions that can / must be updated by implementation ===
            
            // Function is called when pso gets compiled must be implemented to properly fill out pso description
            virtual bool __internal_ConstructPso(IPsoManipulator* ptrManipulator) = 0;
            // Optional update function (NOT used for shader hot reloading). Called every frame (only when used!) return true to force reconstructing the pso.
            virtual inline bool __internal_Update(bool directoryChange) { return false; };

        private:
            // Type of PSO object
            PipelineStateType m_type = PipelineStateType::Invalid;

            // Directory state
            UINT64 m_lastDirIteration = 0;

            // Descriptors
            union
            {
                struct
                {
                    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDescGfx;
                    DescriptorBuffer<D3D12_SO_DECLARATION_ENTRY, 16> m_gfxSoEnties;
                    DescriptorBuffer<UINT, 16> m_gfxSoStrides;
                    DescriptorBuffer<D3D12_INPUT_ELEMENT_DESC, 16> m_gfxInputElements;
                };
                struct
                {
                    D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDescCompute;
                };
            };

            // Shader links
            union
            {
                Shader* m_ptrVSShader = nullptr;
                Shader* m_ptrCSShader;
            };
            Shader* m_ptrDSShader = nullptr;
            Shader* m_ptrHSShader = nullptr;
            Shader* m_ptrGSShader = nullptr;
            Shader* m_ptrPSShader = nullptr;
            Shader* m_ptrMSShader = nullptr;
            Shader* m_ptrASShader = nullptr;

            // PSO Object
            ComPointer<ID3D12PipelineState> m_ptrPso;
            ComPointer<ID3D12RootSignature> m_ptrRootSignature;
    };
}
