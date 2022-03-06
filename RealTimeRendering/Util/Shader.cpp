#include "Shader.h"

wchar_t RTR::Shader::s_cachePath[MAX_PATH] = { 0 };

RTR::Shader::Shader(const wchar_t* path, const wchar_t* target, const wchar_t* entryPointName) :
    m_path(path), m_target(target), m_entryPointer(entryPointName)
{}

RTR::Shader::~Shader()
{
    if (m_ptrCompiledData)
    {
        free(m_ptrCompiledData);
        m_ptrCompiledData = nullptr;
    }
    if (m_ptrRootData)
    {
        free(m_ptrRootData);
        m_ptrRootData = nullptr;
    }
}

bool RTR::Shader::Load()
{
    bool result = false;

    // Get change timestamps
    auto srcChange = GetSourceLastChanged();
    auto cacheChange = GetCacheSrcLastChanged();

    // Recompile if not the same
    if (memcmp(&srcChange, &cacheChange, sizeof(FILETIME)) != 0)
    {
        result = CompileShaderFromSoure();
        m_compileSrcTime = srcChange;
        
        // Cache just compiled data
        if (result)
        {
            CacheShader(srcChange);
        }
    }
    else
    {
        // Load cached blob (compile if failed)
        result = LoadShaderFromCache();
        m_compileSrcTime = cacheChange;
        if (!result)
        {
            result = CompileShaderFromSoure();
        }
    }

    // Clear time on failure (allow hot reload to load shader after bug fix!)
    if (!result)
    {
        m_compileSrcTime = { 0,0 };
    }

    return result;
}

bool RTR::Shader::Refresh()
{
    bool outputChanged = false;

    // Get latest dir iteration
    const UINT64 currentDirRevesion = DirWatchGetRevision();
    if (currentDirRevesion != m_dirItteration)
    {
        // Check if source file is affected
        FILETIME newSrcTime = GetSourceLastChanged();
        if (memcmp(&m_compileSrcTime, &newSrcTime, sizeof(FILETIME)) != 0)
        {
            // Compile new source
            void* newData = nullptr;
            size_t newSize = 0;
            outputChanged = CompileShaderFromSoure(&newSize, &newData);

            // If output has changed
            if (outputChanged)
            {
                // Free old data
                if (m_ptrCompiledData)
                {
                    free(m_ptrCompiledData);
                }

                // Store new data
                m_ptrCompiledData = newData;
                m_compileSize = newSize;
                m_compileSrcTime = newSrcTime;

                // Cache new result
                CacheShader(newSrcTime);
            }
        }

        m_dirItteration = currentDirRevesion;
    }

    return outputChanged;
}

const wchar_t* RTR::Shader::GetShaderCacheDir()
{
    // Check if path need to be retrived
    if (!*s_cachePath)
    {
        RTR_CHECK_HRESULT(
            "Retriving shader cache folder",
            SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, L"LoetwigFusel\\RealTimeRendering\\", s_cachePath)
        );
    }

    // Retun path
    return s_cachePath;
}

bool RTR::Shader::CompileShaderFromSoure(size_t* ptrSize, void** ppData, size_t* ptrSizeR, void** ppDataR)
{
    bool result = false;

    // Adjust invalid pointer
    if (!ptrSize) ptrSize = &m_compileSize;
    if (!ppData) ppData = &m_ptrCompiledData;
    if (!ptrSizeR) ptrSizeR = &m_rooteSize;
    if (!ppDataR) ppDataR = &m_ptrRootData;

    // Build compiler arguments
    const wchar_t* compilerArgs[] =
    {
        m_path,
        L"-E", m_entryPointer,
        L"-HV", __RTR_DXC_CONFIG_HLSL_VERSION,
        L"-T", m_target,
        L"-Zi",
        L"-Qembed_debug",
        // L"-extractrootsignature"
    };

    // DXC Objects
    ComPointer<IDxcUtils> ptrDxcUtils;
    ComPointer<IDxcCompiler3> ptrDxcCompiler;

    // Create DXC objects
    RTR_CHECK_HRESULT(
        "Creating CLSID_DxcUtils",
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&ptrDxcUtils))
    );
    RTR_CHECK_HRESULT(
        "Creating CLSID_DxcCompiler",
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&ptrDxcCompiler))
    );

    // Create default inculde handler
    ComPointer<IDxcIncludeHandler> ptrDefIncHandler;
    RTR_CHECK_HRESULT(
        "Creating DXC default include handler",
        ptrDxcUtils->CreateDefaultIncludeHandler(&ptrDefIncHandler)
    );

    // Open source file
    ComPointer<IDxcBlobEncoding> ptrSourceBlob;
    if (SUCCEEDED(ptrDxcUtils->LoadFile(m_path, nullptr, &ptrSourceBlob)))
    {
        // Encode in buffer
        DxcBuffer srcBuffer;
        srcBuffer.Ptr = ptrSourceBlob->GetBufferPointer();
        srcBuffer.Size = ptrSourceBlob->GetBufferSize();
        srcBuffer.Encoding = DXC_CP_ACP;

        // Compile
        ComPointer<IDxcResult> ptrCompileResult;
        RTR_CHECK_HRESULT(
            "Compiling shader from file",
            ptrDxcCompiler->Compile(&srcBuffer, compilerArgs, _countof(compilerArgs), ptrDefIncHandler, IID_PPV_ARGS(&ptrCompileResult))
        );

        // Check if compile failed
        HRESULT compilerStatus;
        RTR_CHECK_HRESULT(
            "Retriving compiler status",
            ptrCompileResult->GetStatus(&compilerStatus)
        );

        // Only load shader if compile succeeded
        if (SUCCEEDED(compilerStatus))
        {
            // Read shader blob
            ComPointer<IDxcBlob> ptrShaderBlob;
            RTR_CHECK_HRESULT(
                "Retriving compiled data",
                ptrCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&ptrShaderBlob), nullptr)
            );

            // Allocate memory internaly
            *ptrSize = ptrShaderBlob->GetBufferSize();
            *ppData = malloc(*ptrSize);
            if (!*ppData)
            {
                throw std::exception("Failed to allocate memory for CPU shader storage!");
            }

            // Copy shader
            memcpy(*ppData, ptrShaderBlob->GetBufferPointer(), *ptrSize);
            result = true;


            // Read root blob
            ComPointer<IDxcBlob> ptrRootBlob;
            RTR_CHECK_HRESULT(
                "Retriving compiled data",
                ptrCompileResult->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&ptrRootBlob), nullptr)
            );

            // Allocate memory internaly
            *ptrSizeR = ptrRootBlob->GetBufferSize();
            *ppDataR = malloc(*ptrSizeR);
            if (!*ppDataR)
            {
                throw std::exception("Failed to allocate memory for CPU rootsignature storage!");
            }

            // Copy shader
            memcpy(*ppDataR, ptrRootBlob->GetBufferPointer(), *ptrSizeR);
            result = true;

            // Show info
            #ifdef _DEBUG
            OutputDebugString(L"Recompiled Shader \"");
            OutputDebugString(m_path);
            OutputDebugString(L"\"\n");
            #endif
        }
        else
        {
            // Get errors 
            ComPointer<IDxcBlobUtf8> ptrErrorBlob;
            if (SUCCEEDED(ptrCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&ptrErrorBlob), nullptr)))
            {
                #ifdef _DEBUG
                OutputDebugString(L"Errors while compiling shader \"");
                OutputDebugString(m_path);
                OutputDebugString(L"\"\n");
                OutputDebugStringA(ptrErrorBlob->GetStringPointer());
                OutputDebugString(L"\n");
                #endif
            }
        }
    }

    return result;
}

bool RTR::Shader::LoadShaderFromCache()
{
    bool result = false;

    // Copy source string
    wchar_t filePath[MAX_PATH];
    wcscpy_s<MAX_PATH>(filePath, GetShaderCacheDir());

    // Append name to path
    RTR_CHECK_HRESULT(
        "Appending filename",
        PathCchAppend(filePath, MAX_PATH, m_path)
    );

    // Append entry point name
    auto* dotPos = wcsrchr(filePath, L'.');
    if (dotPos) *dotPos = L'\0';
    wcscat_s<MAX_PATH>(filePath, m_entryPointer);
    wcscat_s<MAX_PATH>(filePath, L".cso");

    // Check and open file
    HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // Get size of file
        DWORD fileSize = GetFileSize(hFile, nullptr);
        if (fileSize)
        {
            // Allocat memory
            m_compileSize = fileSize;
            m_ptrCompiledData = malloc(m_compileSize);
            if (m_ptrCompiledData)
            {
                // Read file
                if (ReadFile(hFile, m_ptrCompiledData, fileSize, nullptr, nullptr))
                {
                    result = true;
                }
            }
        }

        CloseHandle(hFile);
    }

    // Rename extension to rs "root signature"
    RTR_CHECK_HRESULT(
        "Change file extension",
        PathCchRenameExtension(filePath, MAX_PATH, L"rs")
    );

    // Check and open file
    HANDLE hFile2 = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL);
    if (result && hFile2 != INVALID_HANDLE_VALUE)
    {
        // Get size of file
        DWORD fileSize = GetFileSize(hFile2, nullptr);
        if (fileSize)
        {
            // Allocat memory
            m_rooteSize = fileSize;
            m_ptrRootData = malloc(m_rooteSize);
            if (m_ptrRootData)
            {
                // Read file
                if (ReadFile(hFile2, m_ptrRootData, fileSize, nullptr, nullptr))
                {
                    result = true;
                }
            }
        }

        CloseHandle(hFile2);
    }

    return result;
}

void RTR::Shader::CacheShader(FILETIME cacheSrcTime)
{
    // Copy source string
    wchar_t filePath[MAX_PATH];
    wcscpy_s<MAX_PATH>(filePath, GetShaderCacheDir());

    // Append name to path
    RTR_CHECK_HRESULT(
        "Appending filename",
        PathCchAppend(filePath, MAX_PATH, m_path)
    );

    // Append entry point name
    auto* dotPos = wcsrchr(filePath, L'.');
    if (dotPos) *dotPos = L'\0';
    wcscat_s<MAX_PATH>(filePath, m_entryPointer);
    wcscat_s<MAX_PATH>(filePath, L".lci");

    // Create / override file
    HANDLE hFile = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // Write data to file
        WriteFile(hFile, &cacheSrcTime, sizeof(DWORD) * 2, nullptr, nullptr);
        CloseHandle(hFile);
    }

    // Rename extension to cso "compile shader object"
    RTR_CHECK_HRESULT(
        "Change file extension",
        PathCchRenameExtension(filePath, MAX_PATH, L"cso")
    );

    // Create / override file
    HANDLE hFile2 = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, NULL);
    if (hFile2 != INVALID_HANDLE_VALUE)
    {
        // Write data to file
        WriteFile(hFile2, m_ptrCompiledData, (DWORD)m_compileSize, nullptr, nullptr);
        CloseHandle(hFile2);
    }

    // Rename extension to rs "root signature"
    RTR_CHECK_HRESULT(
        "Change file extension",
        PathCchRenameExtension(filePath, MAX_PATH, L"rs")
    );

    // Create / override file
    HANDLE hFile3 = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, NULL);
    if (hFile3 != INVALID_HANDLE_VALUE)
    {
        // Write data to file
        WriteFile(hFile3, m_ptrRootData, (DWORD)m_rooteSize, nullptr, nullptr);
        CloseHandle(hFile3);
    }
}

FILETIME RTR::Shader::GetSourceLastChanged()
{
    FILETIME t = {0,0};

    // Get Attributes for file
    WIN32_FILE_ATTRIBUTE_DATA fileAttributes = {};
    if (GetFileAttributesEx(m_path, GetFileExInfoStandard, &fileAttributes))
    {
        t = fileAttributes.ftLastWriteTime;
    }

    return t;
}

FILETIME RTR::Shader::GetCacheSrcLastChanged()
{
    FILETIME result = {0,0};

    // Copy source string
    wchar_t filePath[MAX_PATH];
    wcscpy_s<MAX_PATH>(filePath, GetShaderCacheDir());

    // Append name to path
    RTR_CHECK_HRESULT(
        "Appending filename",
        PathCchAppend(filePath, MAX_PATH, m_path)
    );

    // Append entry point name
    auto* dotPos = wcsrchr(filePath, L'.');
    if (dotPos) *dotPos = L'\0';
    wcscat_s<MAX_PATH>(filePath, m_entryPointer);
    wcscat_s<MAX_PATH>(filePath, L".lci");

    // Check and open file
    HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // Read exeactly two dwords
        if (!ReadFile(hFile, &result, sizeof(DWORD) * 2, nullptr, nullptr))
        {
            result = { 0,0 };
        }
        CloseHandle(hFile);
    }

    return result;
}
