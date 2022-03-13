#pragma once

#include <WinInclude.h>
#include <Util/ComPointer.h>
#include <Util/HrException.h>
#include <D3DCommon/D3DInstance.h>

namespace RTR
{
    // Fwd decl
    class Window;

    // Window listener
    class WindowListener
    {
        public:
            // Virtual function that will handle the event
            virtual bool HandleWindowEvent(Window* wnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* ptrResult) = 0;

        protected:
            // Function
            bool __handleWindowEvent(Window* wnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* ptrResult);

            // Expose next listener
            friend class Window;
            WindowListener* ptrNextWindowListener = nullptr;
    };

    // Manager for window class
    class WindowCls
    {
        public:
            // Retrive name of this window class
            static LPCWSTR GetWindowClassName();

        private:
            // Singleton class management
            WindowCls();
            ~WindowCls();
            static WindowCls s_clsInstance;
    };

    // The window itself
    class Window
    {
        public:
            // Construct
            Window() = delete;
            Window(const Window&) = delete;
            Window(LPCWSTR windowName, ID3D12CommandQueue* ptrCmdQueue);

            // Destruct
            ~Window();

            // Assign
            Window& operator=(const Window&) = delete;

            // Set listener
            void AddCustomEventListener(WindowListener* listener);
            // Expose winapi handle
            inline HWND GetWindowHandle()
            {
                return m_windowHandle;
            }

            // Will return true while window has not been closed
            bool ProcessWindowEvents();
            // Return true if window size has changed and d3d12 requires resizing
            bool NeedsResize();
            // Will resize the DirectX components now
            void Resize();

            // Get current frame index
            int GetFrameIndex();
            // Retrive buffer
            ID3D12Resource* GetCurrentBackBuffer();
            // Retrive current buffers CPU handle
            D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentCPUHandle();

            // Present current frame
            void Present(bool vsync);

            // Bounds
            inline UINT GetWidth()
            {
                return m_bufferCurrentWidth;
            }
            inline UINT GetHeight()
            {
                return m_bufferCurrentHeight;
            }

        protected:
            friend class WindowCls;

            // Windows callable handlers
            static LRESULT windowMsgHandler__setup(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
            static LRESULT windowMsgHandler__run(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

            // Actual called function
            bool WindowEvent(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* ptrResult);

        private:
            // Release all open references to buffers
            void releaseBackBuffers();
            // Get buffers from swap chain
            void getBackBuffers();
            
        private:
            // DXGI Back Buffer states
            ComPointer<IDXGISwapChain1> m_swapChain;
            ComPointer<ID3D12Resource> m_backBuffers[2];
            int m_currentBackBufferIndex = -1;

            // RTV Heap
            ComPointer<ID3D12DescriptorHeap> m_rtvHeap;
            unsigned int m_rtvIncrementSize = 0;

            // Window flags
            bool m_shouldClose = false, m_needResize = false;
            unsigned int m_bufferCurrentWidth = 1920, m_bufferCurrentHeight = 1080;

            // Custom window listener
            WindowListener* m_ptrWindowListener = nullptr;

            // Handle to the widows window
            HWND m_windowHandle;
    };
}
