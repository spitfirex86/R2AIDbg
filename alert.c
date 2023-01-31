#include "framework.h"
#include "resource.h"
#include "alert.h"
#include "debug.h"
#include "misc.h"


BOOL CALLBACK AlertDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static tdstAlertData *pData = NULL;

	static HWND hText = NULL;
	static HWND hBreakpoint = NULL;
	static HWND hSpo = NULL;
	static HWND hCurrent = NULL;
	static HWND hPrevious = NULL;
	static HWND hDebug = NULL;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			char szBuffer[40];

			/* get data ptr from lParam */
			pData = (tdstAlertData *)lParam;
			if ( !pData )
				EndDialog(hWnd, IDABORT);

			/* set caption and taskbar icon */
			HICON hIcon = LoadIcon(g_hDllInst, MAKEINTRESOURCE(IDI_APPICON));
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			hText = GetDlgItem(hWnd, IDC_MSG_TEXT);
			hDebug = GetDlgItem(hWnd, ID_DEBUG);
			if ( pData->bFullDlg )
			{
				hBreakpoint = GetDlgItem(hWnd, IDC_MSG_BREAKPOINT);
				hSpo = GetDlgItem(hWnd, IDC_MSG_SPO);
				hCurrent = GetDlgItem(hWnd, IDC_MSG_CURRENTVAL);
				hPrevious = GetDlgItem(hWnd, IDC_MSG_PREVIOUSVAL);
			}

			/* set all details from data */
			SetWindowText(hText, pData->szMsg);
			if ( pData->bFullDlg )
			{
				char szSpoFullName[256];
				char *pPos = szSpoFullName;

				pPos += sprintf(pPos, "%s\\", (pData->szSpoFamily ? pData->szSpoFamily : "(unknown)"));
				pPos += sprintf(pPos, "%s\\", (pData->szSpoModel ? pData->szSpoModel : "(unknown)"));
				pPos += pData->szSpoInstance
					? sprintf(pPos, "%s", pData->szSpoInstance)
					: sprintf(pPos, "(0x%p)", pData->p_stBreakpoint->p_stSpo);

				SetWindowText(hBreakpoint, pData->p_stBreakpoint->szName);
				SetWindowText(hSpo, szSpoFullName);

				fn_vBreakpointFormatValue(pData->p_stBreakpoint, pData->p_stBreakpoint->lCurrentValue, szBuffer);
				SetWindowText(hCurrent, szBuffer);

				fn_vBreakpointFormatValue(pData->p_stBreakpoint, pData->p_stBreakpoint->lPreviousValue, szBuffer);
				SetWindowText(hPrevious, szBuffer);
			}

			EnableWindow(hDebug, pData->bCanDebug);
			RedrawWindow(hText, NULL, NULL, RDW_INVALIDATE);

			MessageBeep(MB_ICONWARNING);
			return TRUE;
		}

		case WM_SHOWWINDOW:
			if ( wParam == TRUE && lParam == 0 )
			{
				SetFocus(GetDlgItem(hWnd, IDOK));
				return TRUE;
			}
			break;

		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case ID_DEBUG:
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd, LOWORD(wParam));
					return TRUE;
			}
			break;

		case WM_DESTROY:
		{
			/* nuke vars */
			pData = NULL;
			hText = NULL;
			hBreakpoint = NULL;
			hSpo = NULL;
			hCurrent = NULL;
			hPrevious = NULL;
			hDebug = NULL;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL fn_bAlertMsg( char *szMsg, BOOL bCanDebug )
{
	tdstAlertData *pData = calloc(1, sizeof(tdstAlertData));

	pData->bFullDlg = FALSE;
	pData->bCanDebug = bCanDebug;

	strncpy(pData->szMsg, szMsg, C_MaxMsg - 1);
	pData->szMsg[C_MaxMsg - 1] = 0;

	int lResult = DialogBoxParam(g_hDllInst, MAKEINTRESOURCE(IDD_DEBUGMSG), NULL, AlertDlgProc, (LPARAM)pData);

	free(pData);
	return (lResult == ID_DEBUG);
}

BOOL fn_bAlertBreak( char *szMsg, tdstBreakpoint *p_stBreakpoint, BOOL bCanDebug )
{
	tdstAlertData *pData = calloc(1, sizeof(tdstAlertData));

	pData->bFullDlg = TRUE;
	pData->bCanDebug = bCanDebug;
	pData->p_stBreakpoint = p_stBreakpoint;

	strncpy(pData->szMsg, szMsg, C_MaxMsg - 1);
	pData->szMsg[C_MaxMsg - 1] = 0;

	pData->szSpoFamily = XHIE_fn_szGetSuperObjectFamilyName(p_stBreakpoint->p_stSpo);
	pData->szSpoModel = XHIE_fn_szGetSuperObjectModelName(p_stBreakpoint->p_stSpo);
	pData->szSpoInstance = XHIE_fn_szGetSuperObjectPersonalName(p_stBreakpoint->p_stSpo);

	int lResult = DialogBoxParam(g_hDllInst, MAKEINTRESOURCE(IDD_DEBUGBREAK), NULL, AlertDlgProc, (LPARAM)pData);

	free(pData);
	return (lResult == ID_DEBUG);
}
