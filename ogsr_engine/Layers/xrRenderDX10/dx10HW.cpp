// dx10HW.cpp: implementation of the DX10 specialisation of CHW.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#pragma warning(disable:4995)
#include <d3dx/d3dx9.h>
#pragma warning(default:4995)
#include "../xrRender/HW.h"
#include "../../xr_3da/XR_IOConsole.h"
#include "../../xr_3da/xr_input.h"
#include "../../Include/xrAPI/xrAPI.h"

#include "StateManager\dx10SamplerStateCache.h"
#include "StateManager\dx10StateCache.h"

#ifndef _EDITOR
void	fill_vid_mode_list			(CHW* _hw);
void	free_vid_mode_list			();

void	fill_render_mode_list		();
void	free_render_mode_list		();
#else
void	fill_vid_mode_list			(CHW* _hw)	{}
void	free_vid_mode_list			()			{}
void	fill_render_mode_list		()			{}
void	free_render_mode_list		()			{}
#endif

CHW			HW;

CHW::CHW() : 
	m_move_window(true)
{
	Device.seqAppActivate.Add(this);
	Device.seqAppDeactivate.Add(this);
}

CHW::~CHW()
{
	Device.seqAppActivate.Remove(this);
	Device.seqAppDeactivate.Remove(this);
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CHW::CreateD3D()
{
#ifdef USE_DX11
	// Минимально поддерживаемая версия Windows => Windows Vista SP2 или Windows 7.
	R_CHK(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory)));
	pFactory->EnumAdapters1(0, &m_pAdapter);
#else
	R_CHK( CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory)) );
	pFactory->EnumAdapters(0, &m_pAdapter);
#endif
}

void CHW::DestroyD3D()
{
	_SHOW_REF("refCount:m_pAdapter", m_pAdapter);
	_RELEASE(m_pAdapter);

#ifdef HAS_DX11_2
    _SHOW_REF("refCount:m_pFactory2", m_pFactory2);
    _RELEASE(m_pFactory2);
#endif

	_SHOW_REF("refCount:pFactory", pFactory);
	_RELEASE(pFactory);
}

void CHW::CreateDevice( HWND m_hWnd, bool move_window )
{
	m_move_window			= move_window;
	CreateD3D();

	// General - select adapter and device
	//BOOL  bWindowed			= !psDeviceFlags.is(rsFullscreen);

	m_DriverType = Caps.bForceGPU_REF ? 
		D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_HARDWARE;

	// Display the name of video board
#ifdef USE_DX11
	DXGI_ADAPTER_DESC1 Desc;
	R_CHK(m_pAdapter->GetDesc1(&Desc));
#else
	DXGI_ADAPTER_DESC Desc;
	R_CHK( m_pAdapter->GetDesc(&Desc) );
#endif
	//	Warning: Desc.Description is wide string
	Msg		("* GPU [vendor:%X]-[device:%X]: %S", Desc.VendorId, Desc.DeviceId, Desc.Description);

	Caps.id_vendor	= Desc.VendorId;
	Caps.id_device	= Desc.DeviceId;

	// Select back-buffer & depth-stencil format
	D3DFORMAT&	fTarget	= Caps.fTarget;
	D3DFORMAT&	fDepth	= Caps.fDepth;

	//	HACK: DX10: Embed hard target format.
	fTarget = D3DFMT_X8R8G8B8;	//	No match in DX10. D3DFMT_A8B8G8R8->DXGI_FORMAT_R8G8B8A8_UNORM
	fDepth = selectDepthStencil(fTarget);

	UINT createDeviceFlags = 0;
#ifdef DEBUG
	if (IsDebuggerPresent())
		createDeviceFlags |= D3D_CREATE_DEVICE_DEBUG;
#endif

	HRESULT R;
    D3D_FEATURE_LEVEL featureLevels2[] =
    {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL featureLevels3[] =
    {
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    const auto createDevice = [&](const D3D_FEATURE_LEVEL* level, const u32 levels)
    {
        return D3D11CreateDevice(m_pAdapter, D3D_DRIVER_TYPE_UNKNOWN,
            nullptr, createDeviceFlags, level, levels,
            D3D11_SDK_VERSION, &pDevice, &FeatureLevel, &pContext);
    };

    if (DX10Only())
        R = createDevice(featureLevels3, std::size(featureLevels3));
    else if(DX11Only())
	{
		//R = createDevice(featureLevels, std::size(featureLevels));
		//if(FAILED(R))
			R = createDevice(featureLevels2, std::size(featureLevels2));
	}

    if (FeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        ComputeShadersSupported = true;
    else
    {
        D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS data;
        pDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS,
            &data, sizeof(data));
        ComputeShadersSupported = data.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x;
    }

	//DXGI_SWAP_CHAIN_DESC& sd = m_ChainDesc;
	//R_CHK(pFactory->CreateSwapChain(pDevice, &sd, &m_pSwapChain));

	/*if (FeatureLevel != D3D_FEATURE_LEVEL_11_1)
	{
		R_ASSERT(FeatureLevel == D3D_FEATURE_LEVEL_11_0); //На всякий случай
		Msg("!![%s] DirectX 11.1 not supported!", __FUNCTION__);
	}*/

	// https://habr.com/ru/post/308980/
	IDXGIDevice1* pDeviceDXGI = nullptr;
	R_CHK(pDevice->QueryInterface(IID_PPV_ARGS(&pDeviceDXGI)));
	R_CHK(pDeviceDXGI->SetMaximumFrameLatency(1));
	_RELEASE(pDeviceDXGI);

	_SHOW_REF	("* CREATE: DeviceREF:",HW.pDevice);

#ifdef HAS_DX11_3
    pDevice->QueryInterface(__uuidof(ID3D11Device3), reinterpret_cast<void**>(&pDevice3));
#endif

    if (!CreateSwapChain2(m_hWnd))
    {
        CreateSwapChain(m_hWnd);
    }

	//	Create render target and depth-stencil views here
	UpdateViews();

	size_t	memory									= Desc.DedicatedVideoMemory;
	Msg		("*     Texture memory: %d M",		memory/(1024*1024));

	updateWindowProps							(m_hWnd);
	fill_vid_mode_list							(this);
}

void CHW::CreateSwapChain(HWND hwnd)
{
    // Set up the presentation parameters
    DXGI_SWAP_CHAIN_DESC& sd = m_ChainDesc;
    ZeroMemory(&sd, sizeof(sd));
    // Back buffer
    sd.BufferDesc.Width = Device.dwWidth;
    sd.BufferDesc.Height = Device.dwHeight;
    //  TODO: DX10: implement dynamic format selection
    constexpr DXGI_FORMAT formats[] =
    {
        //DXGI_FORMAT_R16G16B16A16_FLOAT, // Do we even need this?
        //DXGI_FORMAT_R10G10B10A2_UNORM, // D3DX11SaveTextureToMemory fails on this format
        DXGI_FORMAT_B8G8R8X8_UNORM,
        DXGI_FORMAT_B8G8R8X8_UNORM, // This is not supported for DXGI flip presentation model
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM,
    };

    // Select back-buffer format
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Caps.fTarget = D3DFMT_X8R8G8B8;
    sd.BufferCount = 1;
    // Multisample
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    // Windoze
    /* XXX:
       Probably the reason of weird tearing
       glitches reported by Shoker in windowed
       mode with VSync enabled.
       XXX: Fix this windoze stuff!!!
    */
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    /*SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(m_sdlWnd, &info))
    {*/
       sd.OutputWindow = hwnd;
/* }
    else
        Log("Couldn't get window information: ", SDL_GetError());*/
    sd.OutputWindow = hwnd;

	BOOL  bWindowed = !psDeviceFlags.is(rsFullscreen);
    sd.Windowed = bWindowed;

    //  Additional set up
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    R_CHK(pFactory->CreateSwapChain(pDevice, &sd, &m_pSwapChain));
}

bool CHW::CreateSwapChain2(HWND hwnd)
{
    if (strstr(Core.Params, "-no_dx11_2"))
        return false;

#ifdef HAS_DX11_2
    m_pAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&m_pFactory2);
    if (!m_pFactory2)
        return false;

    // Set up the presentation parameters
    DXGI_SWAP_CHAIN_DESC1 desc{};

    // Back buffer
    desc.Width = Device.dwWidth;
    desc.Height = Device.dwHeight;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fulldesc{};
    fulldesc.Windowed = !psDeviceFlags.is(rsFullscreen);

    constexpr DXGI_FORMAT formats[] =
    {
        //DXGI_FORMAT_R16G16B16A16_FLOAT,
        //DXGI_FORMAT_R10G10B10A2_UNORM,
        DXGI_FORMAT_B8G8R8X8_UNORM, // This is not supported for DXGI flip presentation model
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM,
    };

    // Select back-buffer format
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Caps.fTarget = D3DFMT_X8R8G8B8;

    // Multisample
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    // Buffering
    desc.BufferCount = 1; // For DXGI_SWAP_EFFECT_FLIP_DISCARD we need at least two
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Windoze
    //desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // XXX: tearing glitches with flip presentation model
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;

    IDXGISwapChain1* swapchain = nullptr;
    HRESULT result = m_pFactory2->CreateSwapChainForHwnd(pDevice, hwnd, &desc,
        fulldesc.Windowed ? nullptr : &fulldesc, nullptr, &swapchain);

    if (FAILED(result))
        return false;

    m_pSwapChain = swapchain;
    m_pSwapChain->GetDesc(&m_ChainDesc);

    m_pSwapChain->QueryInterface(__uuidof(IDXGISwapChain2), reinterpret_cast<void**>(&m_pSwapChain2));

    return true;
#else // #ifdef HAS_DX11_2
    UNUSED(hwnd);
#endif

    return false;
}

void CHW::DestroyDevice()
{
	//	Destroy state managers
	StateManager.Reset();
	RSManager.ClearStateArray();
	DSSManager.ClearStateArray();
	BSManager.ClearStateArray();
	SSManager.ClearStateArray();

	_SHOW_REF				("refCount:pBaseZB",pBaseZB);
	_RELEASE				(pBaseZB);

	_SHOW_REF				("refCount:pBaseRT",pBaseRT);
	_RELEASE				(pBaseRT);
//#ifdef DEBUG
//	_SHOW_REF				("refCount:dwDebugSB",dwDebugSB);
//	_RELEASE				(dwDebugSB);
//#endif

	//	Must switch to windowed mode to release swap chain
	if (!m_ChainDesc.Windowed) m_pSwapChain->SetFullscreenState( FALSE, nullptr);
	_SHOW_REF				("refCount:m_pSwapChain",m_pSwapChain);
	_RELEASE				(m_pSwapChain);
#ifdef HAS_DX11_2
    _SHOW_REF("refCount:m_pSwapChain2", m_pSwapChain2);
    _RELEASE(m_pSwapChain2);
#endif
#ifdef USE_DX11
	_RELEASE				(pContext);
#endif

#ifndef USE_DX11
	_RELEASE				(HW.pDevice1);
#endif
	_SHOW_REF				("DeviceREF:",HW.pDevice);
	_RELEASE				(HW.pDevice);

#ifdef HAS_DX11_3
    _SHOW_REF("refCount:HW.pDevice3:", HW.pDevice3);
    _RELEASE(HW.pDevice3);
#endif

	DestroyD3D				();

#ifndef _EDITOR
	free_vid_mode_list		();
#endif
}

//////////////////////////////////////////////////////////////////////
// Resetting device
//////////////////////////////////////////////////////////////////////
void CHW::Reset (HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC &cd = m_ChainDesc;

	BOOL	bWindowed		= !psDeviceFlags.is	(rsFullscreen);

	cd.Windowed = bWindowed;

	m_pSwapChain->SetFullscreenState(!bWindowed, nullptr);

	DXGI_MODE_DESC	&desc = m_ChainDesc.BufferDesc;

	selectResolution(desc.Width, desc.Height, bWindowed);

	if (bWindowed)
	{
		desc.RefreshRate.Numerator = 60;
		desc.RefreshRate.Denominator = 1;
	}
	else
		desc.RefreshRate = selectRefresh( desc.Width, desc.Height, desc.Format);

	CHK_DX(m_pSwapChain->ResizeTarget(&desc));


#ifdef DEBUG
	//	_RELEASE			(dwDebugSB);
#endif
	_SHOW_REF				("refCount:pBaseZB",pBaseZB);
	_SHOW_REF				("refCount:pBaseRT",pBaseRT);

	_RELEASE(pBaseZB);
	_RELEASE(pBaseRT);

	CHK_DX(m_pSwapChain->ResizeBuffers(
		cd.BufferCount,
		desc.Width,
		desc.Height,
		desc.Format,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	UpdateViews();

	updateWindowProps	(hwnd);
}

bool CHW::UsingFlipPresentationModel() const
{
    return m_ChainDesc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
        || m_ChainDesc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD;
}

D3DFORMAT CHW::selectDepthStencil	(D3DFORMAT fTarget)
{
	// R3 hack
#pragma todo("R3 need to specify depth format")
	return D3DFMT_D24S8;
}

void CHW::selectResolution( u32 &dwWidth, u32 &dwHeight, BOOL bWindowed )
{
	fill_vid_mode_list			(this);

	if(bWindowed)
	{
		dwWidth		= psCurrentVidMode[0];
		dwHeight	= psCurrentVidMode[1];
	}
	else //check
	{
		string64					buff;
		xr_sprintf					(buff,sizeof(buff),"%dx%d",psCurrentVidMode[0],psCurrentVidMode[1]);

		if(_ParseItem(buff,vid_mode_token)==u32(-1)) //not found
		{ //select safe
			xr_sprintf				(buff,sizeof(buff),"vid_mode %s",vid_mode_token[0].name);
			Console->Execute		(buff);
		}

		dwWidth						= psCurrentVidMode[0];
		dwHeight					= psCurrentVidMode[1];
	}
}

DXGI_RATIONAL CHW::selectRefresh(u32 dwWidth, u32 dwHeight, DXGI_FORMAT fmt)
{
	DXGI_RATIONAL	res;

	res.Numerator = 60;
	res.Denominator = 1;

	float	CurrentFreq = 60.0f;

	if (psDeviceFlags.is(rsRefresh60hz))	
	{
		return res;
	}
	else
	{
		xr_vector<DXGI_MODE_DESC>	modes;

		IDXGIOutput *pOutput;
		m_pAdapter->EnumOutputs(0, &pOutput);
		VERIFY(pOutput);

		UINT num = 0;
		DXGI_FORMAT format = fmt;
		UINT flags         = 0;

		// Get the number of display modes available
		pOutput->GetDisplayModeList( format, flags, &num, 0);

		// Get the list of display modes
		modes.resize(num);
		pOutput->GetDisplayModeList( format, flags, &num, &modes.front());

		_RELEASE(pOutput);

		for (u32 i=0; i<num; ++i)
		{
			DXGI_MODE_DESC &desc = modes[i];

			if( (desc.Width == dwWidth) 
				&& (desc.Height == dwHeight)
				)
			{
				VERIFY(desc.RefreshRate.Denominator);
				float TempFreq = float(desc.RefreshRate.Numerator)/float(desc.RefreshRate.Denominator);
				if ( TempFreq > CurrentFreq )
				{
					CurrentFreq = TempFreq;
					res = desc.RefreshRate;
				}
			}
		}

		return res;
	}
}

void CHW::OnAppActivate()
{
	if ( m_pSwapChain && !m_ChainDesc.Windowed )
	{
		ShowWindow( m_ChainDesc.OutputWindow, SW_RESTORE );
		m_pSwapChain->SetFullscreenState(psDeviceFlags.is(rsFullscreen), nullptr);
	}
}

void CHW::OnAppDeactivate()
{
	if ( m_pSwapChain && !m_ChainDesc.Windowed )
	{
		m_pSwapChain->SetFullscreenState( FALSE, nullptr );
		ShowWindow( m_ChainDesc.OutputWindow, SW_MINIMIZE );
	}
}


BOOL CHW::support( D3DFORMAT fmt, DWORD type, DWORD usage)
{
	//	TODO: DX10: implement stub for this code.
	VERIFY(!"Implement CHW::support");
	/*
	HRESULT hr		= pD3D->CheckDeviceFormat(DevAdapter,DevT,Caps.fTarget,usage,(D3DRESOURCETYPE)type,fmt);
	if (FAILED(hr))	return FALSE;
	else			return TRUE;
	*/
	return TRUE;
}

void CHW::updateWindowProps(HWND m_hWnd)
{
	//	BOOL	bWindowed				= strstr(Core.Params,"-dedicated") ? TRUE : !psDeviceFlags.is	(rsFullscreen);
	BOOL	bWindowed				= !psDeviceFlags.is	(rsFullscreen);

	LONG_PTR dwWindowStyle = 0;
	// Set window properties depending on what mode were in.
	if (bWindowed)		{
		if (m_move_window) {
			static const bool bBordersMode = !!strstr(Core.Params, "-draw_borders");
			dwWindowStyle = WS_VISIBLE;
			if (bBordersMode)
				dwWindowStyle |= WS_BORDER | WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX;
			SetWindowLongPtr(m_hWnd, GWL_STYLE, dwWindowStyle);

			// When moving from fullscreen to windowed mode, it is important to
			// adjust the window size after recreating the device rather than
			// beforehand to ensure that you get the window size you want.  For
			// example, when switching from 640x480 fullscreen to windowed with
			// a 1000x600 window on a 1024x768 desktop, it is impossible to set
			// the window size to 1000x600 until after the display mode has
			// changed to 1024x768, because windows cannot be larger than the
			// desktop.

			RECT			m_rcWindowBounds;
			int fYOffset = 0;
			static const bool bCenter = !!strstr(Core.Params, "-center_screen");

			if (bCenter) {
				RECT				DesktopRect;

				GetClientRect		(GetDesktopWindow(), &DesktopRect);

				SetRect(			&m_rcWindowBounds, 
					(DesktopRect.right-m_ChainDesc.BufferDesc.Width)/2, 
					(DesktopRect.bottom-m_ChainDesc.BufferDesc.Height)/2, 
					(DesktopRect.right+m_ChainDesc.BufferDesc.Width)/2, 
					(DesktopRect.bottom+m_ChainDesc.BufferDesc.Height)/2);
			}else{
				if (bBordersMode)
					fYOffset = GetSystemMetrics(SM_CYCAPTION); // size of the window title bar
				SetRect(			&m_rcWindowBounds,
					0, 
					0, 
					m_ChainDesc.BufferDesc.Width, 
					m_ChainDesc.BufferDesc.Height);
			};

			AdjustWindowRect		(	&m_rcWindowBounds, DWORD(dwWindowStyle), FALSE );

			SetWindowPos			(	m_hWnd, 
				HWND_NOTOPMOST,	
				m_rcWindowBounds.left, 
				m_rcWindowBounds.top + fYOffset,
				( m_rcWindowBounds.right - m_rcWindowBounds.left ),
				( m_rcWindowBounds.bottom - m_rcWindowBounds.top ),
				SWP_SHOWWINDOW|SWP_NOCOPYBITS|SWP_DRAWFRAME );
		}
	}
	else
	{
		SetWindowLongPtr( m_hWnd, GWL_STYLE, dwWindowStyle=(WS_POPUP|WS_VISIBLE) );
	}

	SetForegroundWindow( m_hWnd );
	pInput->clip_cursor(true);
}




struct _uniq_mode
{
	_uniq_mode(LPCSTR v):_val(v){}
	LPCSTR _val;
	bool operator() (LPCSTR _other) {return !stricmp(_val,_other);}
};

void free_vid_mode_list()
{
	for( int i=0; vid_mode_token[i].name; i++ )
	{
		xr_free					(vid_mode_token[i].name);
	}
	xr_free						(vid_mode_token);
	vid_mode_token				= nullptr;
}

void fill_vid_mode_list(CHW* _hw)
{
	if(vid_mode_token != nullptr)		return;
	xr_vector<LPCSTR>	_tmp;
	xr_vector<DXGI_MODE_DESC>	modes;

	IDXGIOutput *pOutput;
	//_hw->m_pSwapChain->GetContainingOutput(&pOutput);
	_hw->m_pAdapter->EnumOutputs(0, &pOutput);
	VERIFY(pOutput);

	UINT num = 0;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT flags         = 0;

	// Get the number of display modes available
	pOutput->GetDisplayModeList( format, flags, &num, 0);

	// Get the list of display modes
	modes.resize(num);
	pOutput->GetDisplayModeList( format, flags, &num, &modes.front());

	_RELEASE(pOutput);

	for (u32 i=0; i<num; ++i)
	{
		DXGI_MODE_DESC &desc = modes[i];
		string32		str;

		//-> Удаляем поддержку 4:3
		if(desc.Width <= 800 || (float(desc.Width) / float(desc.Height) <= (1024.f / 768.f + 0.01f))) continue;

		xr_sprintf(str, sizeof(str), "%dx%d", desc.Width, desc.Height);

		if(_tmp.end() != std::find_if(_tmp.begin(), _tmp.end(), _uniq_mode(str)))
			continue;

		_tmp.push_back				(nullptr);
		_tmp.back()					= xr_strdup(str);
	}
	


//	_tmp.push_back				(nullptr);
//	_tmp.back()					= xr_strdup("1024x768");

	u32 _cnt						= _tmp.size()+1;

	vid_mode_token					= xr_alloc<xr_token>(_cnt);

	vid_mode_token[_cnt-1].id			= -1;
	vid_mode_token[_cnt-1].name		= nullptr;

#ifdef DEBUG
	Msg("Available video modes[%d]:",_tmp.size());
#endif // DEBUG
	for( u32 i=0; i<_tmp.size(); ++i )
	{
		vid_mode_token[i].id		= i;
		vid_mode_token[i].name		= _tmp[i];
#ifdef DEBUG
		Msg							("[%s]",_tmp[i]);
#endif // DEBUG
	}
}

void CHW::UpdateViews()
{
	DXGI_SWAP_CHAIN_DESC &sd = m_ChainDesc;
	HRESULT R;

	// Create a render target view
	//R_CHK	(pDevice->GetRenderTarget			(0,&pBaseRT));
	ID3DTexture2D *pBuffer;
	R = m_pSwapChain->GetBuffer( 0, __uuidof( ID3DTexture2D ), (LPVOID*)&pBuffer );
	R_CHK(R);

	R = pDevice->CreateRenderTargetView( pBuffer, nullptr, &pBaseRT);
	pBuffer->Release();
	R_CHK(R);

	//	Create Depth/stencil buffer
	//	HACK: DX10: hard depth buffer format
	//R_CHK	(pDevice->GetDepthStencilSurface	(&pBaseZB));
	ID3DTexture2D* pDepthStencil = nullptr;
	D3D_TEXTURE2D_DESC descDepth;
	descDepth.Width = sd.BufferDesc.Width;
	descDepth.Height = sd.BufferDesc.Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D_USAGE_DEFAULT;
	descDepth.BindFlags = D3D_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	R = pDevice->CreateTexture2D( &descDepth,       // Texture desc
		nullptr,                  // Initial data
		&pDepthStencil ); // [out] Texture
	R_CHK(R);

	//	Create Depth/stencil view
	R = pDevice->CreateDepthStencilView( pDepthStencil, nullptr, &pBaseZB );
	R_CHK(R);

	pDepthStencil->Release();
}

bool CHW::DX10Only() const
{
	return ps_r_renderer_mode == RENDERER_MODE_DX10;
}

bool CHW::DX11Only() const
{
	return ps_r_renderer_mode == RENDERER_MODE_DX11;
}