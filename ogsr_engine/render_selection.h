#pragma once

/*Отключает Второй рендер*/
	#define EXCLUDE_R2
/*-----------------------*/

/* Перед сборкой нужно активировать один из этих дефайнов */
#define BUILD_R4			//-> DirectX 11
//#define BUILD_R3			//-> DirectX 10

/* Оба сразу активировать нельзя! */
#if defined(BUILD_R4) && defined(BUILD_R3)
#error Select only one render define plz //БЕЗ ЭТОГО РЕНДЕРЫ НЕ РАБОТАЮТ! НЕ УБИРАТЬ!!!
#endif

#ifdef BUILD_R4
#pragma comment(lib, "xrRender_R4.lib")
#elif defined(BUILD_R3)
#pragma comment(lib, "xrRender_R3.lib")
#else
#error Select render define plz
#endif