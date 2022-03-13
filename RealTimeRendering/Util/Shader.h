#pragma once

#include <WinInclude.h>
#include <Util/ComPointer.h>
#include <Util/HrException.h>
#include <Util/DirWatcher.h>

#include <dxcapi.h>
#include <d3d12shader.h>

#include <fstream>

// Pixel shader
#define RTR_SHADER_PS_6_0 L"ps_6_0"
#define RTR_SHADER_PS_6_1 L"ps_6_1"
#define RTR_SHADER_PS_6_2 L"ps_6_2"
#define RTR_SHADER_PS_6_3 L"ps_6_3"
#define RTR_SHADER_PS_6_4 L"ps_6_4"
#define RTR_SHADER_PS_6_5 L"ps_6_5"
#define RTR_SHADER_PS_6_6 L"ps_6_6"

// Vertex shader
#define RTR_SHADER_VS_6_0 L"vs_6_0"
#define RTR_SHADER_VS_6_1 L"vs_6_1"
#define RTR_SHADER_VS_6_2 L"vs_6_2"
#define RTR_SHADER_VS_6_3 L"vs_6_3"
#define RTR_SHADER_VS_6_4 L"vs_6_4"
#define RTR_SHADER_VS_6_5 L"vs_6_5"
#define RTR_SHADER_VS_6_6 L"vs_6_6"

// Geometry shader
#define RTR_SHADER_GS_6_0 L"gs_6_0"
#define RTR_SHADER_GS_6_1 L"gs_6_1"
#define RTR_SHADER_GS_6_2 L"gs_6_2"
#define RTR_SHADER_GS_6_3 L"gs_6_3"
#define RTR_SHADER_GS_6_4 L"gs_6_4"
#define RTR_SHADER_GS_6_5 L"gs_6_5"
#define RTR_SHADER_GS_6_6 L"gs_6_6"

// Hull shader
#define RTR_SHADER_HS_6_0 L"hs_6_0"
#define RTR_SHADER_HS_6_1 L"hs_6_1"
#define RTR_SHADER_HS_6_2 L"hs_6_2"
#define RTR_SHADER_HS_6_3 L"hs_6_3"
#define RTR_SHADER_HS_6_4 L"hs_6_4"
#define RTR_SHADER_HS_6_5 L"hs_6_5"
#define RTR_SHADER_HS_6_6 L"hs_6_6"

// Domain shader
#define RTR_SHADER_DS_6_0 L"ds_6_0"
#define RTR_SHADER_DS_6_1 L"ds_6_1"
#define RTR_SHADER_DS_6_2 L"ds_6_2"
#define RTR_SHADER_DS_6_3 L"ds_6_3"
#define RTR_SHADER_DS_6_4 L"ds_6_4"
#define RTR_SHADER_DS_6_5 L"ds_6_5"
#define RTR_SHADER_DS_6_6 L"ds_6_6"

// Compute shader
#define RTR_SHADER_CS_6_0 L"cs_6_0"
#define RTR_SHADER_CS_6_1 L"cs_6_1"
#define RTR_SHADER_CS_6_2 L"cs_6_2"
#define RTR_SHADER_CS_6_3 L"cs_6_3"
#define RTR_SHADER_CS_6_4 L"cs_6_4"
#define RTR_SHADER_CS_6_5 L"cs_6_5"
#define RTR_SHADER_CS_6_6 L"cs_6_6"

// Shader lib
#define RTR_SHADER_LIB_6_1 L"lib_6_1"
#define RTR_SHADER_LIB_6_2 L"lib_6_2"
#define RTR_SHADER_LIB_6_3 L"lib_6_3"
#define RTR_SHADER_LIB_6_4 L"lib_6_4"
#define RTR_SHADER_LIB_6_5 L"lib_6_5"
#define RTR_SHADER_LIB_6_6 L"lib_6_6"

// Mesh shader
#define RTR_SHADER_MS_6_5 L"ms_6_5"
#define RTR_SHADER_MS_6_6 L"ms_6_6"

// Amplification shader
#define RTR_SHADER_AS_6_5 L"as_6_5"
#define RTR_SHADER_AS_6_6 L"as_6_6"



// === Config shader compiler ===
#define __RTR_DXC_CONFIG_HLSL_VERSION L"2018"

namespace RTR
{
    // Shader type
    enum class ShaderType
    {
        // Pixel Shader
        PS,
        // Vertex Shader
        VS,
        // Geometry Shader
        GS,
        // Hull Shader
        HS,
        // Domain Shader
        DS,
        // Compute Shaders
        CS,
        // Mesh Shader
        MS,
        // Amplification
        AS,
    };

    class Shader
    {
        public:
            // Construct
            Shader() = delete;
            Shader(const Shader&) = delete;
            Shader(const wchar_t* path, const wchar_t* target, const wchar_t* entryPointName = L"main");

            // Destruct
            ~Shader();

            // Loading function
            bool Load();
            // Refreh the shader (hot reload) returns true if shader has changed!
            bool Refresh();

            // Metrics
            inline const wchar_t* GetShaderName() const noexcept
            {
                return m_path;
            }
            inline const void* GetShaderData() const noexcept
            {
                return m_ptrCompiledData;
            }
            inline size_t GetShaderSize() const noexcept
            {
                return m_compileSize;
            }
            inline const void* GetShaderRootData() const noexcept
            {
                return m_ptrRootData;
            }
            inline size_t GetShaderRootSize() const noexcept
            {
                return m_rooteSize;
            }

        private:
            // Get location of special cache folder
            const wchar_t* GetShaderCacheDir();
            
            // Compile the shader from the source file
            bool CompileShaderFromSoure(size_t* ptrSize = nullptr, void** ppData = nullptr, size_t* ptrSizeR = nullptr, void** ppDataR = nullptr);
            // Load shader from cached file
            bool LoadShaderFromCache();
            // Cache a shader
            void CacheShader(FILETIME cacheSrcTime);

            // Get last changed of sourcefile
            FILETIME GetSourceLastChanged();
            // Retrive the last change time when from when the source was compiled
            FILETIME GetCacheSrcLastChanged();

        private:
            // Static path to shader cache
            static wchar_t s_cachePath[MAX_PATH];

            // Store information
            const wchar_t* m_path;
            const wchar_t* m_target;
            const wchar_t* m_entryPointer;

            // Shader memory information
            void* m_ptrCompiledData = nullptr;
            size_t m_compileSize = 0;
            void* m_ptrRootData = nullptr;
            size_t m_rooteSize = 0;

            // Shader compile time
            FILETIME m_compileSrcTime = { 0,0 };
            UINT64 m_dirItteration = 0;
    };
}