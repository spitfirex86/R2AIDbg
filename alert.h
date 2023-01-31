#pragma once

#include "framework.h"
#include "debug.h"


#define C_MaxMsg 512


typedef struct tdstAlertData
{
	ACP_tdxBool bFullDlg;
	ACP_tdxBool bCanDebug;

	tdstBreakpoint *p_stBreakpoint;
	char szMsg[C_MaxMsg];

	char *szSpoFamily;
	char *szSpoModel;
	char *szSpoInstance;
}
tdstAlertData;


BOOL fn_bAlertMsg( char *szMsg, BOOL bCanDebug );
BOOL fn_bAlertBreak( char *szMsg, tdstBreakpoint *p_stBreakpoint, BOOL bCanDebug );
