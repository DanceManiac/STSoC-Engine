#include "stdafx.h"

void CRenderTarget::phase_rain()
{
   if( !RImplementation.o.dx10_msaa )
      u_setrt	(rt_Color,nullptr,nullptr,HW.pBaseZB);
   else
      u_setrt	(rt_Color,nullptr,nullptr,rt_MSAADepth->pZRT);
	//u_setrt	(rt_Normal,nullptr,nullptr,HW.pBaseZB);
	RImplementation.rmNormal();
}