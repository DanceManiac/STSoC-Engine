// xrRender_R4.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../xrRender/dxRenderFactory.h"
#include "../xrRender/dxUIRender.h"
#include "../xrRender/dxDebugRender.h"

void AttachRender()
{
	//	Can't call CreateDXGIFactory from DllMain
	R4_Test_HW* test_hw = xr_new<R4_Test_HW>();
	R_ASSERT(test_hw->TestDX11Present() || test_hw->TestDX10Present());
	xr_delete(test_hw);
	::Render = &RImplementation;
	::RenderFactory = &RenderFactoryImpl;
	::DU = &DUImpl;
	//::vid_mode_token			= inited by HW;
	UIRender = &UIRenderImpl;
//#ifdef DEBUG
	DRender = &DebugRenderImpl;
//#endif	//	DEBUG
	xrRender_initconsole();
}