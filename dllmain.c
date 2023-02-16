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

			DetourTransactionBegin();
			DetourAttach((PVOID *)&R2_WndProc, (PVOID)MOD_WndProc);
			DetourAttach((PVOID *)&R2_AI_fn_p_stEvalTree, (PVOID)MOD_AI_fn_p_stEvalTree);
			DetourTransactionCommit();
			break;
		}

		case DLL_PROCESS_DETACH:
		{
			DetourTransactionBegin();
			DetourDetach((PVOID *)&R2_WndProc, (PVOID)MOD_WndProc);
			DetourDetach((PVOID *)&R2_AI_fn_p_stEvalTree, (PVOID)MOD_AI_fn_p_stEvalTree);
			DetourTransactionCommit();
			break;
		}

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}
