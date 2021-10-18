// xrRender_R4.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../xrRender/dxRenderFactory.h"
#include "../xrRender/dxUIRender.h"
#include "../xrRender/dxDebugRender.h"

void AttachRender()
{
	//	Can't call CreateDXGIFactory from DllMain
	//if (!xrRender_test_hw())	return FALSE;
	HW.DX10Only = pSettings->r_string_wb("engine_settings", "render_mode") == "r3";;
	::Render = &RImplementation;
	::RenderFactory = &RenderFactoryImpl;
	::DU = &DUImpl;
	//::vid_mode_token			= inited by HW;
	UIRender = &UIRenderImpl;
#ifdef DEBUG
	DRender = &DebugRenderImpl;
#endif	//	DEBUG
	xrRender_initconsole();
}

#include "r2_test_hw.cpp"
extern "C"
{
	bool SupportsDX11Rendering();
};

bool SupportsDX11Rendering()
{
	return xrRender_test_hw_DX11()?true:false;
}

extern "C"
{
	bool  SupportsDX10Rendering();
};

bool  SupportsDX10Rendering()
{
	return xrRender_test_hw_DX10()?true:false;
}