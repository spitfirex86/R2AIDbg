#pragma once

#include "framework.h"


typedef enum tdeBreakpointType
{
	e_BT_HitPoints,
	e_BT_HitPointsMax,
	e_BT_DsgVar
}
tdeBreakpointType;

typedef enum tdeBreakpointMode
{
	e_BM_Zero,
	e_BM_NonZero,
	e_BM_Change
}
tdeBreakpointMode;

typedef struct tdstBreakpoint tdstBreakpoint;
struct tdstBreakpoint
{
	LST_M_DynamicElementDecl(tdstBreakpoint)

	HIE_tdstSuperObject *p_stSpo;
	tdeBreakpointType eType;
	tdeBreakpointMode eMode;

	int lDsgVarId;
	AI_tdeDsgVarType eDsgVarType;

	char szName[120];

	int lPreviousValue;
	int lCurrentValue;
	BOOL bFirstInit;

	BOOL bIsDead;
};

LST_M_DynamicListDecl(tdstBreakpoint);

typedef struct tdstNewBreakpoint
{
	HIE_tdstSuperObject *p_stSpo;
	tdeBreakpointType eType;
	tdeBreakpointMode eMode;
	int lDsgVarId;
	char szSpoName[80];
}
tdstNewBreakpoint;

typedef BOOL (*tdfnDsgVarEnum)( int lId, void *Value, AI_tdeDsgVarType eType );


extern HINSTANCE g_hDllInst;
extern WNDPROC R2_WndProc;
extern AI_tdstNodeInterpret *(*R2_AI_fn_p_stEvalTree)(
	HIE_tdstSuperObject *p_stSpo,
	AI_tdstNodeInterpret *p_stNode,
	AI_tdstGetSetParam *p_stParam
	);

LRESULT CALLBACK MOD_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

AI_tdstNodeInterpret * MOD_AI_fn_p_stEvalTree(
	HIE_tdstSuperObject *p_stSpo,
	AI_tdstNodeInterpret *p_stNode,
	AI_tdstGetSetParam *p_stParam
	);


void (*R2_fn_vSaveEngineClock)( void );
void (*R2_fn_vLoadEngineClock)( void );

extern int *R2_HIE_gs_lCurrentFrame;

void fn_vPauseEngine( void );
void fn_vResumeEngine( void );


extern LST_M_DynamicAnchorTo(tdstBreakpoint) g_stBreakpoints;
extern BOOL g_bBreakpointsPaused;
extern BOOL g_bInMenu;

tdstBreakpoint * fn_p_stAddBreakpoint( tdstNewBreakpoint *p_stNew );
void fn_vRemoveBreakpoint( tdstBreakpoint *p_stBreakpoint );
