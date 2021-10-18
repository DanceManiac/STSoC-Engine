// xrRender_R2.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../xrRender/dxRenderFactory.h"
#include "../xrRender/dxUIRender.h"
#include "../xrRender/dxDebugRender.h"

void AttachR3()
{
	//	Can't call CreateDXGIFactory from DllMain
	//if (!xrRender_test_hw())	return FALSE;
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

extern "C"
{
	bool  SupportsDX10Rendering();
};

bool  SupportsDX10Rendering()
{
	return xrRender_test_hw()?true:false;
}