#include "framework.h"
#include "resource.h"
#include "debug.h"
#include "misc.h"


tdstNewBreakpoint g_stAddDlgData = { 0 };


int fn_lPopulateDsgVarsCB( HWND hCB, HIE_tdstSuperObject *p_stSpo )
{
	char szBuffer[20];

	ComboBox_ResetContent(hCB);

	HIE_tdstEngineObject *p_stPerso = p_stSpo->hLinkedObject.p_stCharacter;
	if ( !p_stPerso->hBrain || !p_stPerso->hBrain->p_stMind )
		return 0;

	AI_tdstDsgMem *p_stDsgMem = p_stPerso->hBrain->p_stMind->p_stDsgMem;
	if ( !p_stDsgMem )
		return 0;
	
	int lNbVar = (*p_stDsgMem->pp_stDsgVar)->ucNbDsgVar;
	for ( int i = 0; i < lNbVar; i++ )
	{
		snprintf(szBuffer, 20, "%d", i);
		int lIdx = ComboBox_AddString(hCB, szBuffer);
		ComboBox_SetItemData(hCB, lIdx, i);
	}

	return lNbVar;
}

void fn_vPopulateSposCB( HWND hCB )
{
	ComboBox_ResetContent(hCB);

	HIE_tdstSuperObject *pChild;
	LST_M_DynamicForEach(*GAM_pp_stDynamicWorld, pChild)
	{
		if ( pChild->ulType != HIE_C_Type_Actor )
			continue;

		char szBuffer[80];
		char *szName = XHIE_fn_szGetSuperObjectPersonalName(pChild);

		if ( szName )
			snprintf(szBuffer, 80, "%s", szName);
		else
			snprintf(szBuffer, 80, "(0x%p)", pChild);

		int lIdx = ComboBox_AddString(hCB, szBuffer);
		ComboBox_SetItemData(hCB, lIdx, pChild);
	}
}



void fn_vUpdateControlsStatus_AddDlg( HWND hWnd )
{
	HWND hOk = GetDlgItem(hWnd, IDOK);
	HWND hSpos = GetDlgItem(hWnd, IDC_SPOS);
	HWND hType_Dsg = GetDlgItem(hWnd, IDC_TYPE_DSG);
	HWND hDsgVars = GetDlgItem(hWnd, IDC_DSGVARS);

	BOOL bGotSpo = ComboBox_GetCurSel(hSpos) != CB_ERR;
	BOOL bSpoHasDsgVars = IsWindowEnabled(hType_Dsg);
	BOOL bDsgTypeSelected = Button_GetCheck(hType_Dsg);
	BOOL bGotDsgVar = ComboBox_GetCurSel(hDsgVars) != CB_ERR;

	/* No SPO selected - disable DsgVar options */
	if ( !bGotSpo )
	{
		EnableWindow(hOk, FALSE);
		EnableWindow(hType_Dsg, FALSE);
		EnableWindow(hDsgVars, FALSE);
		return;
	}

	/* SPO was changed after selecting Dsg type - reset type selection */
	if ( !bSpoHasDsgVars && bDsgTypeSelected )
	{
		CheckRadioButton(hWnd, IDC_TYPE_HP, IDC_TYPE_DSG, IDC_TYPE_HP);
		bDsgTypeSelected = FALSE;
	}

	/* OK when a type other than Dsg is selected, or a DsgVar has already been chosen */
	EnableWindow(hOk, !bDsgTypeSelected || bGotDsgVar);
	/* Only enable when SPO has DsgVars */
	EnableWindow(hType_Dsg, bSpoHasDsgVars);
	/* Only enable when DsgVar type is selected */
	EnableWindow(hDsgVars, bDsgTypeSelected);
}

BOOL CALLBACK AddDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hOk = NULL;
	static HWND hSpos = NULL;
	static HWND hDsgVars = NULL;
	static HWND hType_DsgVar = NULL;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			hOk = GetDlgItem(hWnd, IDOK);
			hSpos = GetDlgItem(hWnd, IDC_SPOS);
			hDsgVars = GetDlgItem(hWnd, IDC_DSGVARS);
			hType_DsgVar = GetDlgItem(hWnd, IDC_TYPE_DSG);

			fn_vPopulateSposCB(hSpos);
			EnableWindow(hDsgVars, FALSE);
			EnableWindow(hOk, FALSE);

			CheckRadioButton(hWnd, IDC_TYPE_HP, IDC_TYPE_DSG, IDC_TYPE_HP);
			CheckRadioButton(hWnd, IDC_MODE_ZERO, IDC_MODE_CHANGE, IDC_MODE_ZERO);
			EnableWindow(hType_DsgVar, FALSE);
			
			return TRUE;
		}

		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_SPOS:
					if ( HIWORD(wParam) == CBN_SELCHANGE )
					{
						int lIdx = ComboBox_GetCurSel(hSpos);
						if ( lIdx != CB_ERR )
						{
							HIE_tdstSuperObject *p_stSpo = (HIE_tdstSuperObject *)ComboBox_GetItemData(hSpos, lIdx);

							int lNbVars = fn_lPopulateDsgVarsCB(hDsgVars, p_stSpo);

							EnableWindow(hType_DsgVar, (lNbVars > 0));
							fn_vUpdateControlsStatus_AddDlg(hWnd);
						}
						return TRUE;
					}
					break;

				case IDC_DSGVARS:
					if ( HIWORD(wParam) == CBN_SELCHANGE )
					{
						if ( ComboBox_GetCurSel(hDsgVars) != CB_ERR )
						{
							fn_vUpdateControlsStatus_AddDlg(hWnd);
						}
						return TRUE;
					}
				break;

				case IDC_TYPE_HP:
				case IDC_TYPE_HPM:
				case IDC_TYPE_DSG:
					fn_vUpdateControlsStatus_AddDlg(hWnd);

				case IDC_MODE_ZERO:
				case IDC_MODE_NZERO:
				case IDC_MODE_CHANGE:
					fn_vUpdateControlsStatus_AddDlg(hWnd);
					return TRUE;

				case IDOK:
				{
					int lIdx = ComboBox_GetCurSel(hSpos);
					g_stAddDlgData.p_stSpo = (HIE_tdstSuperObject *)ComboBox_GetItemData(hSpos, lIdx);
					ComboBox_GetLBText(hSpos, lIdx, g_stAddDlgData.szSpoName);

					lIdx = ComboBox_GetCurSel(hDsgVars);
					g_stAddDlgData.lDsgVarId =ComboBox_GetItemData(hDsgVars, lIdx);

					if ( Button_GetCheck(GetDlgItem(hWnd, IDC_TYPE_HP)) ) g_stAddDlgData.eType = e_BT_HitPoints;
					if ( Button_GetCheck(GetDlgItem(hWnd, IDC_TYPE_HPM)) ) g_stAddDlgData.eType = e_BT_HitPointsMax;
					if ( Button_GetCheck(GetDlgItem(hWnd, IDC_TYPE_DSG)) ) g_stAddDlgData.eType = e_BT_DsgVar;

					if ( Button_GetCheck(GetDlgItem(hWnd, IDC_MODE_ZERO)) ) g_stAddDlgData.eMode = e_BM_Zero;
					if ( Button_GetCheck(GetDlgItem(hWnd, IDC_MODE_NZERO)) ) g_stAddDlgData.eMode = e_BM_NonZero;
					if ( Button_GetCheck(GetDlgItem(hWnd, IDC_MODE_CHANGE)) ) g_stAddDlgData.eMode = e_BM_Change;
				}
				case IDCANCEL:
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
		int lIdx = ListBox_AddString(hLB, pItem->szName);
		ListBox_SetItemData(hLB, lIdx, pItem);
	}
}

BOOL CALLBACK MainDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hList = NULL;
	static HWND hRemove = NULL;
	static HWND hPauseAll = NULL;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			/* set caption and taskbar icon */
			HICON hIcon = LoadIcon(g_hDllInst, MAKEINTRESOURCE(IDI_APPICON));
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			hRemove = GetDlgItem(hWnd, IDC_REMOVE);
			hList = GetDlgItem(hWnd, IDC_LIST);
			hPauseAll = GetDlgItem(hWnd, IDC_PAUSEALL);

			fn_vPopulateBreakpointsLB(hList);
			EnableWindow(hRemove, (ListBox_GetCurSel(hList) != LB_ERR));
			Button_SetCheck(hPauseAll, g_bBreakpointsPaused);

			return TRUE;
		}

		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_LIST:
					if ( HIWORD(wParam) == LBN_SELCHANGE )
					{
						EnableWindow(hRemove, (ListBox_GetCurSel(hList) != LB_ERR));
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
	if ( uMsg == WM_KEYDOWN && wParam == 'D' )
	{
		fn_vShowMainDialog();
		return 0;
	}

	return R2_WndProc(hWnd, uMsg, wParam, lParam);
}
