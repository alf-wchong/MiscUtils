#include "pch.h"

using Microsoft::WRL::ComPtr;

namespace
{
    constexpr int kWindowBorder = 5;
    constexpr int kTextMargin = 30;
    constexpr int kPaneGap = 20;
    constexpr int kMinPaneWidth = 260;
    constexpr wchar_t kFrameClassName[] = L"MfcProtectedPanePoCFrame";
    constexpr wchar_t kAppTitle[] = L"MFC Protected Pane PoC";
    constexpr COLORREF kNeonBlue = RGB(0, 170, 255);

    CString HResultToString(HRESULT hr)
    {
        CString s;
        s.Format(L"0x%08X", static_cast<unsigned>(hr));
        return s;
    }

    void ThrowIfFailed(HRESULT hr, const wchar_t* what)
    {
        if (FAILED(hr))
        {
            CString msg;
            msg.Format(L"%s failed with HRESULT %s", what, HResultToString(hr).GetString());
            AfxThrowUserException();
        }
    }

    class CPlainPaneWnd : public CWnd
    {
    public:
        CString m_text;

        BOOL CreatePane(CWnd* parent, const RECT& rc, UINT id)
        {
            CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_ARROW),
                                              (HBRUSH)(COLOR_WINDOW + 1), nullptr);
            return CreateEx(0, cls, L"", WS_CHILD | WS_VISIBLE, rc, parent, id);
        }

    protected:
        afx_msg BOOL OnEraseBkgnd(CDC* pDC)
        {
            CRect rc;
            GetClientRect(&rc);
            pDC->FillSolidRect(&rc, RGB(250, 250, 250));
            return TRUE;
        }

        afx_msg void OnPaint()
        {
            CPaintDC dc(this);
            CRect rc;
            GetClientRect(&rc);
            dc.FillSolidRect(&rc, RGB(250, 250, 250));

            CRect textRc = rc;
            textRc.DeflateRect(kTextMargin, kTextMargin);

            CFont font;
            font.CreatePointFont(160, L"Segoe UI");
            CFont* old = dc.SelectObject(&font);
            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(RGB(20, 20, 20));
            dc.DrawText(m_text, &textRc, DT_LEFT | DT_TOP | DT_WORDBREAK);
            dc.SelectObject(old);
        }

        DECLARE_MESSAGE_MAP()
    };

    BEGIN_MESSAGE_MAP(CPlainPaneWnd, CWnd)
        ON_WM_ERASEBKGND()
        ON_WM_PAINT()
    END_MESSAGE_MAP()

    class CProtectedPaneWnd : public CWnd
    {
    public:
        BOOL CreatePane(CWnd* parent, const RECT& rc, UINT id)
        {
            CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_ARROW),
                                              (HBRUSH)::GetStockObject(BLACK_BRUSH), nullptr);
            return CreateEx(0, cls, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, rc, parent, id);
        }

        ~CProtectedPaneWnd() override
        {
            Cleanup();
        }

    protected:
        struct BufferSlot
        {
            ComPtr<ID3D11Texture2D> texture;
            ComPtr<IPresentationBuffer> buffer;
            HANDLE availableEvent = nullptr;
        };

        std::vector<BufferSlot> m_buffers;
        size_t m_nextBuffer = 0;
        UINT m_bufferWidth = 0;
        UINT m_bufferHeight = 0;

        ComPtr<ID3D11Device> m_d3dDevice;
        ComPtr<ID3D11DeviceContext> m_d3dContext;
        ComPtr<IDXGIDevice> m_dxgiDevice;
        ComPtr<IPresentationFactory> m_presentationFactory;
        ComPtr<IPresentationManager> m_presentationManager;
        ComPtr<IPresentationSurface> m_presentationSurface;
        HANDLE m_surfaceHandle = nullptr;

        ComPtr<IDCompositionDesktopDevice> m_dcompDevice;
        ComPtr<IDCompositionTarget> m_dcompTarget;
        ComPtr<IDCompositionVisual> m_rootVisual;
        ComPtr<IUnknown> m_wrappedSurface;

        ComPtr<ID2D1Factory1> m_d2dFactory;
        ComPtr<ID2D1Device> m_d2dDevice;
        ComPtr<ID2D1DeviceContext> m_d2dContext;
        ComPtr<IDWriteFactory> m_dwriteFactory;
        ComPtr<IDWriteTextFormat> m_textFormat;
        ComPtr<ID2D1SolidColorBrush> m_textBrush;

        bool m_ready = false;
        CString m_errorText;

        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct)
        {
            if (CWnd::OnCreate(lpCreateStruct) == -1)
                return -1;

            if (!InitializeGraphics())
            {
                m_ready = false;
            }
            return 0;
        }

        afx_msg void OnDestroy()
        {
            Cleanup();
            CWnd::OnDestroy();
        }

        afx_msg void OnSize(UINT nType, int cx, int cy)
        {
            CWnd::OnSize(nType, cx, cy);
            if (cx <= 0 || cy <= 0)
                return;

            if (m_ready)
            {
                if (!ResizeBuffers(static_cast<UINT>(cx), static_cast<UINT>(cy)))
                {
                    m_ready = false;
                    Invalidate();
                }
            }
        }

        afx_msg BOOL OnEraseBkgnd(CDC* pDC)
        {
            UNREFERENCED_PARAMETER(pDC);
            return TRUE;
        }

        afx_msg void OnPaint()
        {
            CPaintDC dc(this);
            if (!m_ready)
            {
                CRect rc;
                GetClientRect(&rc);
                dc.FillSolidRect(&rc, RGB(0, 0, 0));
                rc.DeflateRect(kTextMargin, kTextMargin);
                dc.SetBkMode(TRANSPARENT);
                dc.SetTextColor(RGB(255, 120, 120));
                dc.DrawText(m_errorText.IsEmpty() ? L"Protected pane initialization failed." : m_errorText,
                            &rc, DT_LEFT | DT_TOP | DT_WORDBREAK);
            }
        }

        DECLARE_MESSAGE_MAP()

        void Cleanup()
        {
            for (auto& b : m_buffers)
            {
                if (b.availableEvent)
                {
                    ::CloseHandle(b.availableEvent);
                    b.availableEvent = nullptr;
                }
            }
            m_buffers.clear();
            if (m_surfaceHandle)
            {
                ::CloseHandle(m_surfaceHandle);
                m_surfaceHandle = nullptr;
            }
            m_wrappedSurface.Reset();
            m_rootVisual.Reset();
            m_dcompTarget.Reset();
            m_dcompDevice.Reset();
            m_presentationSurface.Reset();
            m_presentationManager.Reset();
            m_presentationFactory.Reset();
            m_d2dContext.Reset();
            m_d2dDevice.Reset();
            m_d2dFactory.Reset();
            m_dwriteFactory.Reset();
            m_textBrush.Reset();
            m_textFormat.Reset();
            m_dxgiDevice.Reset();
            m_d3dContext.Reset();
            m_d3dDevice.Reset();
        }

        bool InitializeGraphics()
        {
            HRESULT hr = S_OK;
            try
            {
                UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
                flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
                D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
                hr = D3D11CreateDevice(
                    nullptr,
                    D3D_DRIVER_TYPE_HARDWARE,
                    nullptr,
                    flags,
                    &fl,
                    1,
                    D3D11_SDK_VERSION,
                    &m_d3dDevice,
                    nullptr,
                    &m_d3dContext);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"D3D11CreateDevice failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_d3dDevice.As(&m_dxgiDevice);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"QI IDXGIDevice failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = DCompositionCreateDevice3(
                    m_dxgiDevice.Get(),
                    __uuidof(IDCompositionDesktopDevice),
                    reinterpret_cast<void**>(m_dcompDevice.GetAddressOf()));
                if (FAILED(hr))
                {
                    m_errorText.Format(L"DCompositionCreateDevice3 failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_dcompDevice->CreateTargetForHwnd(m_hWnd, TRUE, &m_dcompTarget);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreateTargetForHwnd failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_dcompDevice->CreateVisual(&m_rootVisual);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreateVisual failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = CreatePresentationFactory(m_d3dDevice.Get(), IID_PPV_ARGS(&m_presentationFactory));
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreatePresentationFactory failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                if (!m_presentationFactory->IsPresentationSupported())
                {
                    m_errorText = L"Composition swapchain is unsupported. Requires Windows 11 build 22000.194+ and WDDM 2.0+.";
                    return false;
                }

                hr = m_presentationFactory->CreatePresentationManager(&m_presentationManager);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreatePresentationManager failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = DCompositionCreateSurfaceHandle(COMPOSITIONOBJECT_ALL_ACCESS, nullptr, &m_surfaceHandle);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"DCompositionCreateSurfaceHandle failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_presentationManager->CreatePresentationSurface(m_surfaceHandle, &m_presentationSurface);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreatePresentationSurface failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_presentationSurface->SetDisableReadback(TRUE);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"SetDisableReadback(TRUE) failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_presentationSurface->SetAlphaMode(DXGI_ALPHA_MODE_IGNORE);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"SetAlphaMode failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_presentationSurface->SetColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"SetColorSpace failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_dcompDevice->CreateSurfaceFromHandle(m_surfaceHandle, &m_wrappedSurface);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreateSurfaceFromHandle failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_rootVisual->SetContent(m_wrappedSurface.Get());
                if (FAILED(hr))
                {
                    m_errorText.Format(L"Visual::SetContent failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_dcompTarget->SetRoot(m_rootVisual.Get());
                if (FAILED(hr))
                {
                    m_errorText.Format(L"Target::SetRoot failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                D2D1_FACTORY_OPTIONS fo = {};
#if defined(_DEBUG)
                fo.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
                hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, fo, &m_d2dFactory);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"D2D1CreateFactory failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_d2dFactory->CreateDevice(m_dxgiDevice.Get(), &m_d2dDevice);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"ID2D1Factory1::CreateDevice failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dContext);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreateDeviceContext failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwriteFactory);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"DWriteCreateFactory failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                hr = m_dwriteFactory->CreateTextFormat(
                    L"Segoe UI",
                    nullptr,
                    DWRITE_FONT_WEIGHT_SEMI_BOLD,
                    DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL,
                    28.0f,
                    L"en-us",
                    &m_textFormat);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreateTextFormat failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                m_textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

                CRect rc;
                GetClientRect(&rc);
                if (!ResizeBuffers(static_cast<UINT>(max(1, rc.Width())), static_cast<UINT>(max(1, rc.Height()))))
                {
                    return false;
                }

                hr = m_dcompDevice->Commit();
                if (FAILED(hr))
                {
                    m_errorText.Format(L"Initial DComp Commit failed: %s", HResultToString(hr).GetString());
                    return false;
                }

                m_ready = true;
                return true;
            }
            catch (...)
            {
                m_errorText = L"Unexpected initialization failure.";
                return false;
            }
        }

        bool ResizeBuffers(UINT width, UINT height)
        {
            if (width == 0 || height == 0)
                return true;

            for (auto& b : m_buffers)
            {
                if (b.availableEvent)
                {
                    ::CloseHandle(b.availableEvent);
                    b.availableEvent = nullptr;
                }
            }
            m_buffers.clear();
            m_bufferWidth = width;
            m_bufferHeight = height;

            for (int i = 0; i < 2; ++i)
            {
                BufferSlot slot;
                HRESULT hr = CreatePresentationBuffer(width, height, slot);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreatePresentationBuffer failed: %s", HResultToString(hr).GetString());
                    return false;
                }
                m_buffers.push_back(std::move(slot));
            }
            m_nextBuffer = 0;
            return RenderProtectedContent();
        }

        HRESULT CreatePresentationBuffer(UINT width, UINT height, BufferSlot& out)
        {
            D3D11_TEXTURE2D_DESC td = {};
            td.Width = width;
            td.Height = height;
            td.MipLevels = 1;
            td.ArraySize = 1;
            td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            td.SampleDesc.Count = 1;
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            td.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_DISPLAYABLE;

            HRESULT hr = m_d3dDevice->CreateTexture2D(&td, nullptr, &out.texture);
            if (FAILED(hr))
                return hr;

            ComPtr<IDXGIResource1> dxgiRes;
            hr = out.texture.As(&dxgiRes);
            if (FAILED(hr))
                return hr;

            HANDLE sharedHandle = nullptr;
            hr = dxgiRes->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &sharedHandle);
            if (FAILED(hr))
                return hr;

            hr = m_presentationManager->AddBufferFromSharedHandle(sharedHandle, &out.buffer);
            ::CloseHandle(sharedHandle);
            if (FAILED(hr))
                return hr;

            hr = out.buffer->GetAvailableEvent(&out.availableEvent);
            return hr;
        }

        bool RenderProtectedContent()
        {
            if (m_buffers.empty())
                return false;

            BufferSlot& slot = m_buffers[m_nextBuffer % m_buffers.size()];
            ++m_nextBuffer;

            if (slot.availableEvent)
            {
                DWORD wr = ::WaitForSingleObject(slot.availableEvent, 1000);
                if (wr != WAIT_OBJECT_0)
                {
                    m_errorText = L"Timed out waiting for a presentation buffer.";
                    return false;
                }
            }

            ComPtr<IDXGISurface> dxgiSurface;
            HRESULT hr = slot.texture.As(&dxgiSurface);
            if (FAILED(hr))
            {
                m_errorText.Format(L"QI IDXGISurface failed: %s", HResultToString(hr).GetString());
                return false;
            }

            D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
                96.0f,
                96.0f);

            ComPtr<ID2D1Bitmap1> targetBitmap;
            hr = m_d2dContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &props, &targetBitmap);
            if (FAILED(hr))
            {
                m_errorText.Format(L"CreateBitmapFromDxgiSurface failed: %s", HResultToString(hr).GetString());
                return false;
            }

            m_d2dContext->SetTarget(targetBitmap.Get());
            m_d2dContext->BeginDraw();
            m_d2dContext->Clear(D2D1::ColorF(0.02f, 0.02f, 0.02f, 1.0f));

            if (!m_textBrush)
            {
                hr = m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.95f, 0.35f, 1.0f), &m_textBrush);
                if (FAILED(hr))
                {
                    m_errorText.Format(L"CreateSolidColorBrush failed: %s", HResultToString(hr).GetString());
                    return false;
                }
            }

            D2D1_RECT_F rc = D2D1::RectF(
                static_cast<float>(kTextMargin),
                static_cast<float>(kTextMargin),
                static_cast<float>(m_bufferWidth - kTextMargin),
                static_cast<float>(m_bufferHeight - kTextMargin));

            const wchar_t* text = L"If this portion is also capturable, the PoC has failed";
            m_d2dContext->DrawTextW(
                text,
                static_cast<UINT32>(wcslen(text)),
                m_textFormat.Get(),
                rc,
                m_textBrush.Get(),
                D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
                DWRITE_MEASURING_MODE_NATURAL);

            hr = m_d2dContext->EndDraw();
            if (FAILED(hr))
            {
                m_errorText.Format(L"D2D EndDraw failed: %s", HResultToString(hr).GetString());
                return false;
            }
            m_d2dContext->SetTarget(nullptr);

            hr = m_presentationSurface->SetBuffer(slot.buffer.Get());
            if (FAILED(hr))
            {
                m_errorText.Format(L"SetBuffer failed: %s", HResultToString(hr).GetString());
                return false;
            }

            RECT src = { 0, 0, static_cast<LONG>(m_bufferWidth), static_cast<LONG>(m_bufferHeight) };
            hr = m_presentationSurface->SetSourceRect(&src);
            if (FAILED(hr))
            {
                m_errorText.Format(L"SetSourceRect failed: %s", HResultToString(hr).GetString());
                return false;
            }

            hr = m_presentationManager->Present();
            if (FAILED(hr))
            {
                m_errorText.Format(L"Present failed: %s", HResultToString(hr).GetString());
                return false;
            }

            hr = m_dcompDevice->Commit();
            if (FAILED(hr))
            {
                m_errorText.Format(L"DComp Commit failed: %s", HResultToString(hr).GetString());
                return false;
            }

            return true;
        }
    };

    BEGIN_MESSAGE_MAP(CProtectedPaneWnd, CWnd)
        ON_WM_CREATE()
        ON_WM_DESTROY()
        ON_WM_SIZE()
        ON_WM_ERASEBKGND()
        ON_WM_PAINT()
    END_MESSAGE_MAP()

    class CMainFrame : public CFrameWnd
    {
    public:
        CPlainPaneWnd m_leftPane;
        CProtectedPaneWnd m_rightPane;

        CMainFrame()
        {
            Create(nullptr,
                   kAppTitle,
                   WS_OVERLAPPEDWINDOW,
                   CRect(100, 100, 1200, 700),
                   nullptr,
                   nullptr,
                   0,
                   nullptr);
        }

    protected:
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct)
        {
            if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
                return -1;

            CRect dummy(0, 0, 100, 100);
            if (!m_leftPane.CreatePane(this, dummy, 1001))
                return -1;
            if (!m_rightPane.CreatePane(this, dummy, 1002))
                return -1;

            m_leftPane.m_text = L"This portion is capturable";
            LayoutPanes();
            return 0;
        }

        afx_msg void OnSize(UINT nType, int cx, int cy)
        {
            CFrameWnd::OnSize(nType, cx, cy);
            LayoutPanes();
        }

        afx_msg BOOL OnEraseBkgnd(CDC* pDC)
        {
            CRect rc;
            GetClientRect(&rc);
            pDC->FillSolidRect(&rc, kNeonBlue);
            return TRUE;
        }

        afx_msg void OnPaint()
        {
            CPaintDC dc(this);
            CRect rc;
            GetClientRect(&rc);
            dc.FillSolidRect(&rc, kNeonBlue);

            CRect inner = rc;
            inner.DeflateRect(kWindowBorder, kWindowBorder);
            dc.FillSolidRect(&inner, RGB(235, 235, 235));
        }

        afx_msg BOOL OnNcActivate(BOOL bActive)
        {
            return CFrameWnd::OnNcActivate(bActive);
        }

        DECLARE_MESSAGE_MAP()

        void LayoutPanes()
        {
            if (!::IsWindow(m_hWnd) || !::IsWindow(m_leftPane.m_hWnd) || !::IsWindow(m_rightPane.m_hWnd))
                return;

            CRect rc;
            GetClientRect(&rc);
            rc.DeflateRect(kWindowBorder, kWindowBorder);

            const int width = rc.Width();
            const int height = rc.Height();
            if (width <= 0 || height <= 0)
                return;

            const int paneWidth = max(kMinPaneWidth, (width - kPaneGap) / 2);
            CRect left(rc.left, rc.top, rc.left + paneWidth, rc.bottom);
            CRect right(left.right + kPaneGap, rc.top, rc.right, rc.bottom);

            m_leftPane.MoveWindow(&left);
            m_rightPane.MoveWindow(&right);
            Invalidate(FALSE);
        }
    };

    BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
        ON_WM_CREATE()
        ON_WM_SIZE()
        ON_WM_ERASEBKGND()
        ON_WM_PAINT()
        ON_WM_NCACTIVATE()
    END_MESSAGE_MAP()

    class CProtectedPanePoCApp : public CWinApp
    {
    public:
        BOOL InitInstance() override
        {
            CWinApp::InitInstance();
            AfxEnableControlContainer();
            SetRegistryKey(L"OpenAI");

            auto* frame = new CMainFrame();
            m_pMainWnd = frame;
            frame->ShowWindow(SW_SHOW);
            frame->UpdateWindow();
            return TRUE;
        }
    };
}

CProtectedPanePoCApp theApp;
