#include "D3DInstance.h"

#ifdef _DEBUG
ComPointer<ID3D12Debug5> __global__d3dinstance_ptrDebug;
ComPointer<ID3D12DebugDevice2> __global__d3dinstance_ptrDebugDevice;
#endif
ComPointer<ID3D12Device9> __global__d3dinstance_ptrDevice;
ComPointer<IDXGIFactory7>__global__d3dinstance_ptrGIFactory;

bool RTR::InitD3D12()
{
    // Init debug layer
    #ifdef _DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&__global__d3dinstance_ptrDebug))))
    {
        __global__d3dinstance_ptrDebug->EnableDebugLayer();
        __global__d3dinstance_ptrDebug->SetEnableAutoName(TRUE);
    }
    #endif

    // Create DXGI Factory
    if (SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&__global__d3dinstance_ptrGIFactory))))
    {
        // Get high performance adpater
        ComPointer<IDXGIAdapter> ptrTargetAdpter;
        if (SUCCEEDED(__global__d3dinstance_ptrGIFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&ptrTargetAdpter))))
        {
            // Create D3D12 Device
            if (SUCCEEDED(D3D12CreateDevice(ptrTargetAdpter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&__global__d3dinstance_ptrDevice))))
            {
                // Create debug device
                #ifdef _DEBUG
                __global__d3dinstance_ptrDevice->SetName(L"D3D12 Device Instance (will live forever)");
                __global__d3dinstance_ptrDevice.queryInterface(__global__d3dinstance_ptrDebugDevice);
                #endif
            }
        }
    }

    // Only true if device is valid
    return (bool)__global__d3dinstance_ptrDevice;
}

void RTR::ShutdownD3D12()
{
    // Release normal objects first
    __global__d3dinstance_ptrDevice.release();
    __global__d3dinstance_ptrGIFactory.release();

    // Report live device objects
    #ifdef _DEBUG
    if (__global__d3dinstance_ptrDebugDevice)
    {
        OutputDebugString(L"D3D12 is reporting live device objects. One object is totaly fine! It's the device producing that output.\n");
        __global__d3dinstance_ptrDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL | D3D12_RLDO_DETAIL);
    }
    #endif
}

ID3D12Device9* RTR::GetD3D12DevicePtr()
{
    return __global__d3dinstance_ptrDevice;
}

IDXGIFactory7* RTR::GetDXGIFactoryPtr()
{
    return __global__d3dinstance_ptrGIFactory;
}

const D3D12_HEAP_PROPERTIES* RTR::GetD3D12DefaultHeapProperites()
{
    static D3D12_HEAP_PROPERTIES prop = {D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, NULL, NULL};
    return &prop;
}

const D3D12_HEAP_PROPERTIES* RTR::GetD3D12UploadHeapProperites()
{
    static D3D12_HEAP_PROPERTIES prop = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, NULL, NULL };
    return &prop;
}

const D3D12_HEAP_PROPERTIES* RTR::GetD3D12ReadbackHeapProperites()
{
    static D3D12_HEAP_PROPERTIES prop = { D3D12_HEAP_TYPE_READBACK, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, NULL, NULL };
    return &prop;
}
