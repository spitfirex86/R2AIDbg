#pragma once

#include "framework.h"


typedef enum tdeNoun
{
	eN_HitPoints,
	eN_HitPointsMax,
	eN_DsgVar
}
tdeNoun;

typedef enum tdeVerb
{
	eV_Equal,
	eV_NotEqual,
	eV_Greater,
	eV_Lesser,
	eV_Changed
}
tdeVerb;

typedef struct tdstNoun
{
	tdeNoun eType;
	int lDsgVarId;
	AI_tdeDsgVarType eDsgVarType;
}
tdstNoun;

typedef struct tdstVerb
{
	tdeVerb eType;
	int lValue;
}
tdstVerb;


typedef struct tdstBreakpoint tdstBreakpoint;
struct tdstBreakpoint
{
	LST_M_DynamicElementDecl(tdstBreakpoint)

	HIE_tdstSuperObject *p_stSpo;

	tdstNoun stNoun;
	tdstVerb stVerb;

	char szName[120];

	int lPreviousValue;
	int lCurrentValue;
	
	ACP_tdxBool bEnabled;
	ACP_tdxBool bFirstInit;
	ACP_tdxBool bIsDead;
};

LST_M_DynamicListDecl(tdstBreakpoint);

typedef struct tdstNewBreakpoint
{
	HIE_tdstSuperObject *p_stSpo;
	tdstNoun stNoun;
	tdstVerb stVerb;
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

void fn_vDebugDumpBreakpoints( void );
