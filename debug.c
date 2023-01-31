#include "framework.h"
#include "resource.h"
#include "debug.h"


LST_M_DynamicAnchorTo(tdstBreakpoint) g_stBreakpoints = { 0 };

BOOL g_bBreakpointsPaused = FALSE;
BOOL g_bInMenu = FALSE;

char g_szCurrentLevel[MAX_NAME_LEVEL] = "";

int g_lSaveCurrentFrame = 0;

unsigned long g_ulCurrFrame = 0;
unsigned long g_ulPrevFrame = 0;


tdstBreakpoint * fn_p_stAddBreakpoint( tdstNewBreakpoint *p_stNew )
{
	tdstBreakpoint *p_stBreakpoint = calloc(1, sizeof(tdstBreakpoint));

	p_stBreakpoint->p_stSpo = p_stNew->p_stSpo;
	p_stBreakpoint->eType = p_stNew->eType;
	p_stBreakpoint->eMode = p_stNew->eMode;

	char szType[20];
	char szMode[20];

	switch ( p_stNew->eType )
	{
		case e_BT_HitPoints:
			strcpy(szType, "HitPoints");
			break;

		case e_BT_HitPointsMax:
			strcpy(szType, "HitPointsMax");
			break;

		case e_BT_DsgVar:
			p_stBreakpoint->lDsgVarId = p_stNew->lDsgVarId;
			p_stBreakpoint->eDsgVarType = fn_eGetDsgVarType(p_stNew->p_stSpo, p_stNew->lDsgVarId);
			snprintf(szType, 20, "DsgVar_%d", p_stNew->lDsgVarId);
			break;
	}

	switch ( p_stNew->eMode )
	{
		case e_BM_Zero:
			strcpy(szMode, "== 0");
			break;

		case e_BM_NonZero:
			strcpy(szMode, "<> 0");
			break;

		case e_BM_Change:
			strcpy(szMode, "!!");
			break;
	}

	snprintf(p_stBreakpoint->szName, 120, "%s - %s %s", p_stNew->szSpoName, szType, szMode);
	p_stBreakpoint->bFirstInit = TRUE;
	p_stBreakpoint->bIsDead = FALSE;

	LST_M_DynamicAddTail(&g_stBreakpoints, p_stBreakpoint);
	return p_stBreakpoint;
}

void fn_vRemoveBreakpoint( tdstBreakpoint *p_stBreakpoint )
{
	LST_M_DynamicIsolate(p_stBreakpoint);
	free(p_stBreakpoint);
}

void * fn_pvGetDsgVar( HIE_tdstSuperObject *p_stSpo, int idx, AI_tdeDsgVarType *eType_Out )
{
	AI_tdstDsgMem *p_stDsgMem = p_stSpo->hLinkedObject.p_stCharacter->hBrain->p_stMind->p_stDsgMem;
	AI_tdstDsgVarInfo *p_stDsgInfos = (*p_stDsgMem->pp_stDsgVar)->a_stDsgVarInfo;

	if ( eType_Out )
		*eType_Out = p_stDsgInfos[idx].eTypeId;

	return &p_stDsgMem->p_cDsgMemBuffer[p_stDsgInfos[idx].ulOffsetInDsgMem];
}

void * fn_pvGetDsgVarValue( HIE_tdstSuperObject *p_stSpo, int idx )
{
	AI_tdstDsgMem *p_stDsgMem = p_stSpo->hLinkedObject.p_stCharacter->hBrain->p_stMind->p_stDsgMem;
	AI_tdstDsgVarInfo *p_stDsgInfos = (*p_stDsgMem->pp_stDsgVar)->a_stDsgVarInfo;

	return &p_stDsgMem->p_cDsgMemBuffer[p_stDsgInfos[idx].ulOffsetInDsgMem];
}

AI_tdeDsgVarType fn_eGetDsgVarType( HIE_tdstSuperObject *p_stSpo, int idx )
{
	AI_tdstDsgMem *p_stDsgMem = p_stSpo->hLinkedObject.p_stCharacter->hBrain->p_stMind->p_stDsgMem;
	AI_tdstDsgVarInfo *p_stDsgInfos = (*p_stDsgMem->pp_stDsgVar)->a_stDsgVarInfo;

	return p_stDsgInfos[idx].eTypeId;
}

void fn_vEnumDsgVar( HIE_tdstSuperObject *p_stSpo, tdfnDsgVarEnum p_fnEnum )
{
	AI_tdstDsgMem *p_stDsgMem = p_stSpo->hLinkedObject.p_stCharacter->hBrain->p_stMind->p_stDsgMem;
	AI_tdstDsgVar *p_stDsgVars = (*p_stDsgMem->pp_stDsgVar);
	AI_tdstDsgVarInfo *p_stDsgInfos = p_stDsgVars->a_stDsgVarInfo;

	int lNbVar = p_stDsgVars->ucNbDsgVar;
	for ( int i = 0; i < lNbVar; i++ )
	{
		AI_tdeDsgVarType eType = p_stDsgInfos[i].eTypeId;
		void *pv_Value = &p_stDsgMem->p_cDsgMemBuffer[p_stDsgInfos[i].ulOffsetInDsgMem];

		if ( !p_fnEnum(i, pv_Value, eType) )
			break;
	}
}

void fn_vBreakpointFormatValue( tdstBreakpoint *p_stBreakpoint, int lValue, char *szStr_Out )
{
	if ( p_stBreakpoint->eType == e_BT_DsgVar )
	{
		switch ( p_stBreakpoint->eDsgVarType )
		{
			case AI_E_dvt_Boolean:
				sprintf(szStr_Out, "%s", lValue ? "TRUE" : "FALSE");
				break;
			case AI_E_dvt_Char:
			case AI_E_dvt_UChar:
			case AI_E_dvt_Short:
			case AI_E_dvt_UShort:
			case AI_E_dvt_Integer:
				sprintf(szStr_Out, "%d", lValue);
				break;
			case AI_E_dvt_PositiveInteger:
				sprintf(szStr_Out, "%d", (unsigned int)lValue);
				break;
			case AI_E_dvt_Float:
			case AI_E_dvt_Vector:
				sprintf(szStr_Out, "%f", *(float *)&lValue);
				break;
			default:
				sprintf(szStr_Out, "0x%p", (void *)lValue);
				break;
		}
	}
	else
	{
		sprintf(szStr_Out, "%d", lValue);
	}
}

void fn_vBreakpointUpdateValues( tdstBreakpoint *p_stBreakpoint )
{
	p_stBreakpoint->lPreviousValue = p_stBreakpoint->lCurrentValue;

	HIE_tdstEngineObject *p_stPerso = p_stBreakpoint->p_stSpo->hLinkedObject.p_stCharacter;
	switch ( p_stBreakpoint->eType )
	{
		case e_BT_HitPoints:
			p_stBreakpoint->lCurrentValue = p_stPerso->hStandardGame->ucHitPoints;
			break;

		case e_BT_HitPointsMax:
			p_stBreakpoint->lCurrentValue = p_stPerso->hStandardGame->ucHitPointsMax;
			break;

		case e_BT_DsgVar:
		{
			AI_tdeDsgVarType eType;
			void *pv_Value = fn_pvGetDsgVar(p_stBreakpoint->p_stSpo, p_stBreakpoint->lDsgVarId, &eType);

			switch ( eType )
			{
				case AI_E_dvt_Boolean:
					p_stBreakpoint->lCurrentValue = *(ACP_tdxBool *)pv_Value;
					break;
				case AI_E_dvt_Char:
					p_stBreakpoint->lCurrentValue = *(char *)pv_Value;
					break;
				case AI_E_dvt_UChar:
					p_stBreakpoint->lCurrentValue = *(unsigned char *)pv_Value;
					break;
				case AI_E_dvt_Short:
					p_stBreakpoint->lCurrentValue = *(short *)pv_Value;
					break;
				case AI_E_dvt_UShort:
					p_stBreakpoint->lCurrentValue = *(unsigned short *)pv_Value;
					break;
				case AI_E_dvt_Integer:
					p_stBreakpoint->lCurrentValue = *(int *)pv_Value;
					break;
				case AI_E_dvt_PositiveInteger:
					p_stBreakpoint->lCurrentValue = *(unsigned int *)pv_Value;
					break;
				default:
					p_stBreakpoint->lCurrentValue = *(int *)pv_Value;
					break;
			}

			break;
		}
	}

	if ( p_stBreakpoint->bFirstInit )
	{
		p_stBreakpoint->bFirstInit = FALSE;
		p_stBreakpoint->lPreviousValue = p_stBreakpoint->lCurrentValue;
	}
}

BOOL fn_bBreakpointCheckCondition( tdstBreakpoint *p_stBreakpoint )
{
	switch ( p_stBreakpoint->eMode )
	{
		case e_BM_Zero:
			return (p_stBreakpoint->lCurrentValue == 0 && g_ulCurrFrame != g_ulPrevFrame);

		case e_BM_NonZero:
			return (p_stBreakpoint->lCurrentValue != 0  && g_ulCurrFrame != g_ulPrevFrame);

		case e_BM_Change:
			return (p_stBreakpoint->lCurrentValue != p_stBreakpoint->lPreviousValue);
	}

	return FALSE;
}

void fn_vBreakpointVerify( tdstBreakpoint *p_stBreakpoint )
{
	if ( p_stBreakpoint->bIsDead )
		return;

	HIE_tdstSuperObject *pSpo = p_stBreakpoint->p_stSpo;
	HIE_tdstEngineObject *pActor = pSpo->hLinkedObject.p_stCharacter;

	if ( !pSpo->hFatherDyn || !pActor || !pActor->hBrain || !pActor->hBrain->p_stMind )
	{
		/* object was probably destroyed, mark as dead */
		p_stBreakpoint->bIsDead = TRUE;
		strcat(p_stBreakpoint->szName, " [DEAD]");
	}
}

void fn_vCheckBreakpoints( void )
{
	char szBuffer[512];
	char szBufferVal1[32];
	char szBufferVal2[32];

	tdstBreakpoint *pBreakpoint;
	LST_M_DynamicForEach(&g_stBreakpoints, pBreakpoint)
	{
		fn_vBreakpointVerify(pBreakpoint);
		if ( pBreakpoint->bIsDead )
			continue;

		fn_vBreakpointUpdateValues(pBreakpoint);
		BOOL bCondition = fn_bBreakpointCheckCondition(pBreakpoint);

		if ( bCondition )
		{
			fn_vPauseEngine();

			fn_vBreakpointFormatValue(pBreakpoint, pBreakpoint->lCurrentValue, szBufferVal1);
			fn_vBreakpointFormatValue(pBreakpoint, pBreakpoint->lPreviousValue, szBufferVal2);
			sprintf(szBuffer, "Breakpoint '%s':\n\n -->\tCurrent value : %s\t\n\tPrevious value : %s\n\nDo you want to debug?",
					pBreakpoint->szName, szBufferVal1, szBufferVal2);

			if ( MessageBox(NULL, szBuffer, "AI Debug", MB_YESNO | MB_ICONWARNING) == IDYES )
				__debugbreak();

			fn_vResumeEngine();
		}
	}
}

void fn_vUpdateLevelName( void )
{
	if ( strcmp(g_szCurrentLevel, GAM_fn_p_szGetLevelName()) != 0 )
	{
		strcpy(g_szCurrentLevel, GAM_fn_p_szGetLevelName());

		/* wipe breakpoints */
		while ( g_stBreakpoints.lNbOfElementsDyn )
			fn_vRemoveBreakpoint(g_stBreakpoints.hLastElementDyn);
	}
}

AI_tdstNodeInterpret * MOD_AI_fn_p_stEvalTree(
	HIE_tdstSuperObject *p_stSpo,
	AI_tdstNodeInterpret *p_stNode,
	AI_tdstGetSetParam *p_stParam
	)
{
	AI_tdstNodeInterpret *p_stResult = R2_AI_fn_p_stEvalTree(p_stSpo, p_stNode, p_stParam);

	if ( !g_bBreakpointsPaused && !g_bInMenu )
	{
		g_ulPrevFrame = g_ulCurrFrame;
		g_ulCurrFrame = GAM_g_stEngineStructure->stEngineTimer.ulFrameNumber;

		fn_vUpdateLevelName();
		fn_vCheckBreakpoints();
	}

	return p_stResult;
}

void fn_vPauseEngine( void )
{
	GAM_g_stEngineStructure->bEngineIsInPaused = TRUE;
	g_lSaveCurrentFrame = *R2_HIE_gs_lCurrentFrame;
	R2_fn_vSaveEngineClock();
}

void fn_vResumeEngine( void )
{
	GAM_g_stEngineStructure->bEngineIsInPaused = FALSE;
	*R2_HIE_gs_lCurrentFrame = g_lSaveCurrentFrame;
	R2_fn_vLoadEngineClock();
}
