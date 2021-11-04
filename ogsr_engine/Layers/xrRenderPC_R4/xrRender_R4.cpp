// xrRender_R4.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../xrRender/dxRenderFactory.h"
#include "../xrRender/dxUIRender.h"
#include "../xrRender/dxDebugRender.h"

R4_Test_HW test_hw;

void AttachRender()
{
	//	Can't call CreateDXGIFactory from DllMain
	CHECK_OR_EXIT(test_hw.TestDX11Present(), "DirectX 11 support required!");
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