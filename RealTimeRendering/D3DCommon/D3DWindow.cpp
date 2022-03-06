#include "D3DWindow.h"

RTR::WindowCls RTR::WindowCls::s_clsInstance;

LPCWSTR RTR::WindowCls::GetWindowClassName()
{
    return L"RTR\\Window\\Cls";
}

RTR::WindowCls::WindowCls()
{
    // Describe window class
    WNDCLASSEX cex = {};
    cex.cbSize = sizeof(WNDCLASSEX);
    cex.style = CS_OWNDC;
    cex.lpfnWndProc = &Window::windowMsgHandler__setup;
    cex.cbClsExtra = 0;
    cex.cbWndExtra = 0;
    cex.hInstance = GetModuleHandle(NULL);
    cex.hIcon = NULL;
    cex.hCursor = LoadCursor(NULL, IDC_ARROW);
    cex.hbrBackground = NULL;
    cex.lpszMenuName = nullptr;
    cex.lpszClassName = GetWindowClassName();
    cex.hIconSm = NULL;

    // Register window class
    RegisterClassEx(&cex);
}

RTR::WindowCls::~WindowCls()
{
    // Unregister class
    UnregisterClass(GetWindowClassName(), GetModuleHandle(NULL));
}

RTR::Window::Window(LPCWSTR windowName, ID3D12CommandQueue* ptrCmdQueue)
{
    // Create window
    m_windowHandle = CreateWindow
    (
        WindowCls::GetWindowClassName(),
        windowName,
        WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_VISIBLE,
        0, 0, 1920, 1080, NULL, NULL,
        GetModuleHandle(NULL),
        this
    );

    // Create descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
    dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    dhd.NumDescriptors = 2;
    dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dhd.NodeMask = 0;
    RTR_CHECK_HRESULT(
        "Descriptor heap creation",
        GetD3D12DevicePtr()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_rtvHeap))
    );

    // Get increment size
    m_rtvIncrementSize = GetD3D12DevicePtr()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    if (m_windowHandle && m_rtvIncrementSize)
    {
        // Get current window dimensions
        RECT cr;
        if (GetClientRect(m_windowHandle, &cr))
        {
            m_bufferCurrentWidth = cr.right - cr.left;
            m_bufferCurrentHeight = cr.bottom - cr.top;
        }

        // Describe swap chain
        DXGI_SWAP_CHAIN_DESC1 swd = {};
        swd.Width = m_bufferCurrentWidth;
        swd.Height = m_bufferCurrentHeight;
        swd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swd.Stereo = false;
        swd.SampleDesc.Count = 1;
        swd.SampleDesc.Quality = 0;
        swd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swd.BufferCount = 2;
        swd.Scaling = DXGI_SCALING_STRETCH;
        swd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        // Describe fullscreen
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsd = {};
        fsd.RefreshRate.Numerator = 1;
        fsd.RefreshRate.Denominator = 60;
        fsd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fsd.Scaling = DXGI_MODE_SCALING_STRETCHED;
        fsd.Windowed = true;

        // Create swap chain
        RTR_CHECK_HRESULT(
            "Swap chain creation",
            GetDXGIFactoryPtr()->CreateSwapChainForHwnd(ptrCmdQueue, m_windowHandle, &swd, &fsd, nullptr, &m_swapChain)
        );
       
        // Set state as valid (next call can invalidate it)
        m_currentBackBufferIndex = 0;
        getBackBuffers();
    }
    else
    {
        throw std::exception("Window handle or d3d12 desciptor increment not valid");
    }
}

RTR::Window::~Window()
{
    if (m_windowHandle)
    {
        DestroyWindow(m_windowHandle);
    }
}

bool RTR::Window::ProcessWindowEvents()
{
    // Will windows has events in the queue handle them
    MSG msg = {};
    while (PeekMessage(&msg, m_windowHandle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return !m_shouldClose;
}

bool RTR::Window::NeedsResize()
{
    return m_needResize;
}

void RTR::Window::Resize()
{
    releaseBackBuffers();

    // Get new dimentsion
    RECT cr;
    if (GetClientRect(m_windowHandle, &cr))
    {
        m_bufferCurrentWidth = cr.right - cr.left;
        m_bufferCurrentHeight = cr.bottom - cr.top;
    }

    // Resize swap chain buffers
    RTR_CHECK_HRESULT(
        "Resizing of swap chain",
        m_swapChain->ResizeBuffers(0, m_bufferCurrentWidth, m_bufferCurrentHeight, DXGI_FORMAT_UNKNOWN,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING)
    );

    getBackBuffers();

    // Reset index
    m_currentBackBufferIndex = 0;
}

int RTR::Window::GetFrameIndex()
{
    return m_currentBackBufferIndex;
}

ID3D12Resource* RTR::Window::GetCurrentBackBuffer()
{
    ID3D12Resource* ptrBuffer = nullptr;
    if (m_currentBackBufferIndex >= 0)
    {
        ptrBuffer = m_backBuffers[m_currentBackBufferIndex];
    }

    return ptrBuffer;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTR::Window::GetCurrentCPUHandle()
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = { 0 };
    if (m_currentBackBufferIndex >= 0)
    {
        handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += m_rtvIncrementSize * (UINT64)m_currentBackBufferIndex;
    }
    return handle;
}

void RTR::Window::Present(bool vsync)
{
    m_swapChain->Present(vsync ? 1 : 0, vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
}

LRESULT RTR::Window::windowMsgHandler__setup(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Wait until we get NC_CREATE to parse crational data for linking to instance
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT* ptrCr = (CREATESTRUCT*)lParam;

        // Replace userdata with supplied pointer
        SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)ptrCr->lpCreateParams);
        // Update window proc to call run message handler on next message
        SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)&Window::windowMsgHandler__run);

        // Let the run proc handle the event
        return windowMsgHandler__run(wnd, msg, wParam, lParam);
    }

    // By default windows will handle the message
    return DefWindowProc(wnd, msg, wParam, lParam);
}

LRESULT RTR::Window::windowMsgHandler__run(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* ptrWindow = (Window*)GetWindowLongPtr(wnd, GWLP_USERDATA);
    LRESULT result = 0;
    
    // If window will not handle the event call windows default event handler
    if (!ptrWindow->WindowEvent(msg, wParam, lParam, &result))
    {
        result = DefWindowProc(wnd, msg, wParam, lParam);
    }

    return result;
}

bool RTR::Window::WindowEvent(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* ptrResult)
{
    switch (msg)
    {
        // On close event flag the window so it will close
        case WM_CLOSE:
            m_shouldClose = true;
            return true;

        // Resize
        case WM_SIZE:
            m_needResize = (LOWORD(lParam) != m_bufferCurrentWidth || HIWORD(lParam) != m_bufferCurrentHeight);
            // We are just listening - windows should still do its work
            return false;

        // By default dont handle message here
        default:
            return false;
    }
}

void RTR::Window::releaseBackBuffers()
{
    // Release all buffer
    m_backBuffers[0].release();
    m_backBuffers[1].release();
}

void RTR::Window::getBackBuffers()
{
    // We use 2 buffers loop over them
    for (unsigned int i = 0; i < 2; i++)
    {
        // Be save and release again (just in case)
        m_backBuffers[i].release();

        // If getting back buffer from swap chain fails set buffer index to -1 to represent error condition
        if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_backBuffers[i]))))
        {
            m_currentBackBufferIndex = -1;
        }
        else
        {
            // Get current cpu descriptor handle
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
            rtvHandle.ptr += m_rtvIncrementSize * i;

            GetD3D12DevicePtr()->CreateRenderTargetView(m_backBuffers[i], nullptr, rtvHandle);
        }
    }
}
