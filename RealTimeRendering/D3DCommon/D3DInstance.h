#pragma once

#include <WinInclude.h>
#include <Util/ComPointer.h>

namespace RTR
{
    // State functions (init / shutdown)
    bool InitD3D12();
    void ShutdownD3D12();

    // Get device pointers
    ID3D12Device9* GetD3D12DevicePtr();
    IDXGIFactory7* GetDXGIFactoryPtr();

    // Heap properties
    const D3D12_HEAP_PROPERTIES* GetD3D12DefaultHeapProperites();
    const D3D12_HEAP_PROPERTIES* GetD3D12UploadHeapProperites();
    const D3D12_HEAP_PROPERTIES* GetD3D12ReadbackHeapProperites();
}
