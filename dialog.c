#include "framework.h"
#include "resource.h"
#include "debug.h"
#include "misc.h"


typedef struct tdstValStr
{
	int lType;
	char const *szName;
}
tdstValStr;


tdstValStr a_stAddNouns[] = {
	{ eN_HitPoints, "HitPoints" },
	{ eN_HitPointsMax, "HitPointsMax" }
};

tdstValStr a_stAddVerbs[] = {
	{ eV_Equal, "Is" },
	{ eV_NotEqual, "Is Not" },
	{ eV_Greater, "Greater" },
	{ eV_Lesser, "Lesser" },
	{ eV_Changed, "Changed" },
};


tdstNewBreakpoint g_stAddDlgData = { 0 };


void fn_vPopulateSposCB( HWND hCB )
{
	ComboBox_ResetContent(hCB);

	HIE_tdstSuperObject *pChild;
	LST_M_DynamicForEach(*GAM_g_p_stDynamicWorld, pChild)
	{
		if ( pChild->ulType != HIE_C_Type_Actor )
			continue;

		char szBuffer[80];
		char *szName = HIE_fn_szGetObjectPersonalName(pChild);

		if ( szName )
			snprintf(szBuffer, 80, "%s", szName);
		else
			snprintf(szBuffer, 80, "(0x%p)", pChild);

		int lIdx = ComboBox_AddString(hCB, szBuffer);
		ComboBox_SetItemData(hCB, lIdx, pChild);
	}
}

int fn_lPopulateDsgVarsCB( HWND hCB, tdeNoun eNoun, HIE_tdstSuperObject *p_stSpo )
{
	char szBuffer[20];
	
	HIE_tdstEngineObject *p_stPerso = HIE_M_hSuperObjectGetActor(p_stSpo);
	if ( !p_stPerso->hBrain || !p_stPerso->hBrain->p_stMind )
		return 0;

	AI_tdstDsgMem *p_stDsgMem = p_stPerso->hBrain->p_stMind->p_stDsgMem;
	if ( !p_stDsgMem )
		return 0;
	
	int lNbVar = (*p_stDsgMem->pp_stDsgVar)->ucNbDsgVar;
	for ( int i = 0; i < lNbVar; i++ )
	{
		snprintf(szBuffer, 20, "DsgVar_%d", i);
		int lIdx = ComboBox_AddString(hCB, szBuffer);
		ComboBox_SetItemData(hCB, lIdx, MAKELPARAM(eNoun, i));
	}

	return lNbVar;
}

void fn_vPopulateNounsCB( HWND hCB, HIE_tdstSuperObject *p_stSpo )
{
	ComboBox_ResetContent(hCB);

	for ( int i = 0; i < ARRAYSIZE(a_stAddNouns); i++ )
	{
		tdstValStr *pNoun = &a_stAddNouns[i];
		int lIdx = ComboBox_AddString(hCB, pNoun->szName);
		ComboBox_SetItemData(hCB, lIdx, MAKELPARAM(pNoun->lType, 0));
	}

	if ( p_stSpo )
		fn_lPopulateDsgVarsCB(hCB, eN_DsgVar, p_stSpo);

	ComboBox_SetCurSel(hCB, 0);
}

void fn_vPopulateVerbsCB( HWND hCB )
{
	ComboBox_ResetContent(hCB);

	for ( int i = 0; i < ARRAYSIZE(a_stAddVerbs); i++ )
	{
		tdstValStr *pVerb = &a_stAddVerbs[i];
		int lIdx = ComboBox_AddString(hCB, pVerb->szName);
		ComboBox_SetItemData(hCB, lIdx, pVerb->lType);
	}

	ComboBox_SetCurSel(hCB, 0);
}

void fn_vUpdateControlsStatus_AddDlg( HWND hWnd, HWND hSpos, HWND hNoun, HWND hVerb, HWND hValue )
{
	int lNounIdx = ComboBox_GetCurSel(hNoun);
	int lVerbIdx = ComboBox_GetCurSel(hVerb);

	BOOL bGotSpo = ComboBox_GetCurSel(hSpos) != CB_ERR;
	BOOL bGotNoun = lNounIdx != CB_ERR;
	BOOL bGotVerb = lVerbIdx != CB_ERR;
	BOOL bVerbHasValue = bGotVerb && a_stAddVerbs[lVerbIdx].lType != eV_Changed;
	BOOL bGotValue = bVerbHasValue ? Edit_GetTextLength(hValue) > 0 : TRUE;

	/* only enable when verb requires a value */
	EnableWindow(hValue, bVerbHasValue);

	/* all good == enable OK button */
	HWND hOk = GetDlgItem(hWnd, IDOK);
	EnableWindow(hOk, bGotSpo && bGotNoun && bGotVerb && bGotValue);
}

BOOL CALLBACK AddDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hSpos;
	static HWND hNoun;
	static HWND hVerb;
	static HWND hValue;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			hSpos = GetDlgItem(hWnd, IDC_SPOS);
			hNoun = GetDlgItem(hWnd, IDC_NOUN);
			hVerb = GetDlgItem(hWnd, IDC_VERB);
			hValue = GetDlgItem(hWnd, IDC_VALUE);

			fn_vPopulateSposCB(hSpos);
			fn_vPopulateNounsCB(hNoun, NULL);
			fn_vPopulateVerbsCB(hVerb);
			Edit_SetText(hValue, "0");
			Edit_LimitText(hValue, 10);

			fn_vUpdateControlsStatus_AddDlg(hWnd, hSpos, hNoun, hVerb, hValue);
			return TRUE;
		}

		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_SPOS:
					if ( HIWORD(wParam) == CBN_SELCHANGE )
					{
						int lIdx = ComboBox_GetCurSel(hSpos);
						HIE_tdstSuperObject *p_stSpo = ( lIdx != CB_ERR )
							? (HIE_tdstSuperObject *)ComboBox_GetItemData(hSpos, lIdx)
							: NULL;

						fn_vPopulateNounsCB(hNoun, p_stSpo);
						fn_vUpdateControlsStatus_AddDlg(hWnd, hSpos, hNoun, hVerb, hValue);
						return TRUE;
					}
					break;

				case IDC_NOUN:
				case IDC_VERB:
					if ( HIWORD(wParam) == CBN_SELCHANGE )
					{
						fn_vUpdateControlsStatus_AddDlg(hWnd, hSpos, hNoun, hVerb, hValue);
						return TRUE;
					}
					break;

				case IDC_VALUE:
					if ( HIWORD(wParam) == EN_UPDATE )
					{
						fn_vUpdateControlsStatus_AddDlg(hWnd, hSpos, hNoun, hVerb, hValue);
						return TRUE;
					}
					break;

				case IDOK:
				{
					char szBuffer[20], *pEnd;
					Edit_GetText(hValue, szBuffer, sizeof(szBuffer));

					int lValue = strtol(szBuffer, &pEnd, 0);
					if ( pEnd == szBuffer || *pEnd != '\0' )
					{
						SetFocus(hValue);
						Edit_SetSel(hValue, 0, -1);
						return TRUE;
					}

					int lIdx = ComboBox_GetCurSel(hSpos);
					g_stAddDlgData.p_stSpo = (HIE_tdstSuperObject *)ComboBox_GetItemData(hSpos, lIdx);
					ComboBox_GetLBText(hSpos, lIdx, g_stAddDlgData.szSpoName);

					lIdx = ComboBox_GetCurSel(hNoun);
					LPARAM lIdNoun = (LPARAM)ComboBox_GetItemData(hNoun, lIdx);
					g_stAddDlgData.stNoun.eType = LOWORD(lIdNoun);
					g_stAddDlgData.stNoun.lDsgVarId = HIWORD(lIdNoun);

					lIdx = ComboBox_GetCurSel(hVerb);
					g_stAddDlgData.stVerb.eType = (tdeVerb)ComboBox_GetItemData(hVerb, lIdx);
					g_stAddDlgData.stVerb.lValue = lValue;
				}
				case IDCANCEL:
					hSpos = NULL;
					hNoun = NULL;
					hVerb = NULL;
					hValue = NULL;
					EndDialog(hWnd, LOWORD(wParam));
					return TRUE;
			}
			break;
		/* modal dialog, therefore no WM_CLOSE or WM_DESTROY */
	}

	return FALSE;
}

void fn_vPopulateBreakpointsLB( HWND hLB )
{
	ListBox_ResetContent(hLB);

	tdstBreakpoint *pItem;
	LST_M_DynamicForEach(&g_stBreakpoints, pItem)
	{
		char szName[200];
		snprintf(
			szName, 200, "%s%s%s",
			(pItem->bEnabled) ? "" : "-- ",
			pItem->szName,
			(pItem->bIsDead) ? " [DEAD]" : ""
		);

		int lIdx = ListBox_AddString(hLB, szName);
		ListBox_SetItemData(hLB, lIdx, pItem);
	}
}

BOOL CALLBACK MainDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hList;
	static HWND hRemove;
	static HWND hEnable;
	static HWND hPauseAll;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			/* set caption and taskbar icon */
			HICON hIcon = LoadIcon(g_hDllInst, MAKEINTRESOURCE(IDI_APPICON));
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			hList = GetDlgItem(hWnd, IDC_LIST);
			hRemove = GetDlgItem(hWnd, IDC_REMOVE);
			hEnable = GetDlgItem(hWnd, IDC_ENABLE);
			hPauseAll = GetDlgItem(hWnd, IDC_PAUSEALL);

			fn_vPopulateBreakpointsLB(hList);
			EnableWindow(hRemove, (ListBox_GetCurSel(hList) != LB_ERR));
			EnableWindow(hEnable, (ListBox_GetCurSel(hList) != LB_ERR));
			Button_SetCheck(hPauseAll, g_bBreakpointsPaused);

			return TRUE;
		}

		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_LIST:
					if ( HIWORD(wParam) == LBN_SELCHANGE )
					{
						int lIdx = ListBox_GetCurSel(hList);
						BOOL bSel = lIdx != LB_ERR;
						EnableWindow(hRemove, bSel);
						EnableWindow(hEnable, bSel);

						tdstBreakpoint *p_stBreakpoint = (tdstBreakpoint *)ListBox_GetItemData(hList, lIdx);
						Button_SetCheck(hEnable, bSel ? p_stBreakpoint->bEnabled : FALSE);

						return TRUE;
					}
					break;

				case IDC_ADD:
				{
					ZeroMemory(&g_stAddDlgData, sizeof(tdstNewBreakpoint));
					if ( DialogBox(g_hDllInst, MAKEINTRESOURCE(IDD_ADDNEW), hWnd, AddDlgProc) == IDOK )
					{
						fn_p_stAddBreakpoint(&g_stAddDlgData);
						fn_vPopulateBreakpointsLB(hList);
					}
					return TRUE;
				}

				case IDC_REMOVE:
				{
					int lIdx = ListBox_GetCurSel(hList);
					if ( lIdx != LB_ERR )
					{
						tdstBreakpoint *p_stBreakpoint = (tdstBreakpoint *)ListBox_GetItemData(hList, lIdx);
						fn_vRemoveBreakpoint(p_stBreakpoint);
						fn_vPopulateBreakpointsLB(hList);
						EnableWindow(hRemove, (ListBox_GetCurSel(hList) != LB_ERR));
						EnableWindow(hEnable, (ListBox_GetCurSel(hList) != LB_ERR));
					}
					return TRUE;
				}

				case IDC_ENABLE:
				{
					int lIdx = ListBox_GetCurSel(hList);
					if ( lIdx != LB_ERR )
					{
						tdstBreakpoint *p_stBreakpoint = (tdstBreakpoint *)ListBox_GetItemData(hList, lIdx);
						p_stBreakpoint->bEnabled = Button_GetCheck(hEnable);
					}
					return TRUE;
				}

				case IDC_PAUSEALL:
				{
					g_bBreakpointsPaused = Button_GetCheck(hPauseAll);
					return TRUE;
				}

				case IDOK:
				case IDCANCEL:
					hList = NULL;
					hRemove = NULL;
					hEnable = NULL;
					hPauseAll = NULL;
					EndDialog(hWnd, LOWORD(wParam));
					return TRUE;
			}
			break;
		/* modal dialog, therefore no WM_CLOSE or WM_DESTROY */
	}

	return FALSE;
}

void fn_vShowMainDialog( void )
{
	fn_vPauseEngine();
	ShowCursor(TRUE);
	g_bInMenu = TRUE;

	int lResult = DialogBox(g_hDllInst, MAKEINTRESOURCE(IDD_MAIN), GAM_fn_hGetWindowHandle(), MainDlgProc);

	g_bInMenu = FALSE;
	ShowCursor(FALSE);
	fn_vResumeEngine();
}

LRESULT CALLBACK MOD_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_KEYDOWN && wParam == 'D' && IPT_g_stInputStructure->ulNumberOfEntryElement )
	{
		if ( GetKeyState(VK_CONTROL) & 0x8000 )
			fn_vDebugDumpBreakpoints();
		fn_vShowMainDialog();
		return 0;
	}

	return R2_WndProc(hWnd, uMsg, wParam, lParam);
}
