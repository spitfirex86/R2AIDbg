#include "framework.h"
#include "debug.h"


HINSTANCE g_hDllInst = NULL;
WNDPROC R2_WndProc = NULL;
AI_tdstNodeInterpret *(*R2_AI_fn_p_stEvalTree)(
	HIE_tdstSuperObject *p_stSpo,
	AI_tdstNodeInterpret *p_stNode,
	AI_tdstGetSetParam *p_stParam
	) = NULL;

void (*R2_fn_vSaveEngineClock)( void ) = OFFSET(0x409E90);
void (*R2_fn_vLoadEngineClock)( void ) = OFFSET(0x409F00);

int *R2_HIE_gs_lCurrentFrame = OFFSET(0x514EA0);


BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	switch ( dwReason )
	{
		case DLL_PROCESS_ATTACH:
		{
			g_hDllInst = hModule;
			R2_WndProc = GAM_fn_WndProc;
			R2_AI_fn_p_stEvalTree = AI_fn_p_stEvalTree;

			FHK_M_lCreateHook(&R2_WndProc, MOD_WndProc);
			FHK_M_lCreateHook(&R2_AI_fn_p_stEvalTree, MOD_AI_fn_p_stEvalTree);

			break;
		}

		case DLL_PROCESS_DETACH:
		{
			FHK_M_lDestroyHook(&R2_WndProc, MOD_WndProc);
			FHK_M_lDestroyHook(&R2_AI_fn_p_stEvalTree, MOD_AI_fn_p_stEvalTree);
			break;
		}

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}
