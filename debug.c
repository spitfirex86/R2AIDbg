#include "framework.h"
#include "debug.h"
#include "misc.h"
#include "alert.h"


char const *a_szNouns[] = {
	[eN_HitPoints] = "HitPoints",
	[eN_HitPointsMax] = "HitPointsMax",
	[eN_DsgVar] = "DsgVar"
};

char const *a_szVerbs[] = {
	[eV_Equal] = "==",
	[eV_NotEqual] = "<>",
	[eV_Greater] = ">",
	[eV_Lesser] = "<",
	[eV_Changed] = "!!"
};


LST_M_DynamicAnchorTo(tdstBreakpoint) g_stBreakpoints = { 0 };

BOOL g_bBreakpointsPaused = FALSE;
BOOL g_bInMenu = FALSE;

char g_szCurrentLevel[MAX_NAME_LEVEL] = "";

int g_lSaveCurrentFrame = 0;

unsigned long g_ulCurrFrame = 0;
unsigned long g_ulPrevFrame = 0;


void fn_vGetNounName( tdstNoun *pNoun, char *szOut )
{
	if ( pNoun->eType == eN_DsgVar )
		sprintf(szOut, "%s_%d", a_szNouns[pNoun->eType], pNoun->lDsgVarId);
	else
		sprintf(szOut, "%s", a_szNouns[pNoun->eType]);
}

void fn_vGetVerbName( tdstVerb *pVerb, char *szOut )
{
	if ( pVerb->eType == eV_Changed )
		snprintf(szOut, 20, "%s", a_szVerbs[pVerb->eType]);
	else
		snprintf(szOut, 20, "%s %d", a_szVerbs[pVerb->eType], pVerb->lValue);
}

tdstBreakpoint * fn_p_stAddBreakpoint( tdstNewBreakpoint *p_stNew )
{
	tdstNoun *pNoun = &p_stNew->stNoun;
	tdstVerb *pVerb = &p_stNew->stVerb;

	char const *szVerbName = a_szVerbs[pVerb->eType];

	char szNoun[20];
	char szVerb[20];

	if ( pNoun->eType == eN_DsgVar )
	{
		AI_tdeDsgVarType eType;
		AI_fn_bGetDsgVar(p_stNew->p_stSpo, pNoun->lDsgVarId, &eType, NULL);
		pNoun->eDsgVarType = eType;
	}

	fn_vGetNounName(pNoun, szNoun);
	fn_vGetVerbName(pVerb, szVerb);

	tdstBreakpoint *p_stBreakpoint = calloc(1, sizeof(tdstBreakpoint));

	snprintf(p_stBreakpoint->szName, 120, "%s - %s %s", p_stNew->szSpoName, szNoun, szVerb);

	p_stBreakpoint->p_stSpo = p_stNew->p_stSpo;
	p_stBreakpoint->stNoun = *pNoun;
	p_stBreakpoint->stVerb = *pVerb;

	p_stBreakpoint->bEnabled = TRUE;
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

void fn_vBreakpointUpdateValues( tdstBreakpoint *p_stBreakpoint )
{
	p_stBreakpoint->lPreviousValue = p_stBreakpoint->lCurrentValue;

	switch ( p_stBreakpoint->stNoun.eType )
	{
		case eN_HitPoints:
			p_stBreakpoint->lCurrentValue = HIE_M_hSuperObjectGetStdGame(p_stBreakpoint->p_stSpo)->ucHitPoints;
			break;

		case eN_HitPointsMax:
			p_stBreakpoint->lCurrentValue = HIE_M_hSuperObjectGetStdGame(p_stBreakpoint->p_stSpo)->ucHitPointsMax;
			break;

		case eN_DsgVar:
		{
			AI_tdeDsgVarType eType;
			void *pv_Value;
			AI_fn_bGetDsgVar(p_stBreakpoint->p_stSpo, p_stBreakpoint->stNoun.lDsgVarId, &eType, &pv_Value);

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
	int lLeft = p_stBreakpoint->lCurrentValue;
	int lRight = p_stBreakpoint->stVerb.lValue;

	switch ( p_stBreakpoint->stVerb.eType )
	{
		case eV_Equal:
			return (lLeft == lRight && g_ulCurrFrame != g_ulPrevFrame);

		case eV_NotEqual:
			return (lLeft != lRight && g_ulCurrFrame != g_ulPrevFrame);

		case eV_Greater:
			return (lLeft > lRight && g_ulCurrFrame != g_ulPrevFrame);

		case eV_Lesser:
			return (lLeft < lRight && g_ulCurrFrame != g_ulPrevFrame);

		case eV_Changed:
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
	}
}

void fn_vCheckBreakpoints( void )
{
	tdstBreakpoint *pBreakpoint;
	LST_M_DynamicForEach(&g_stBreakpoints, pBreakpoint)
	{
		fn_vBreakpointVerify(pBreakpoint);
		if ( !pBreakpoint->bEnabled || pBreakpoint->bIsDead )
			continue;

		fn_vBreakpointUpdateValues(pBreakpoint);
		if ( fn_bBreakpointCheckCondition(pBreakpoint) )
		{
			fn_vPauseEngine();
			if ( fn_bAlertBreak("A breakpoint has been reached !\r\nIf you want to debug, attach the debugger now.", pBreakpoint, TRUE) )
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


void fn_vDebugDumpBreakpoints( void )
{
	tdstBreakpoint *pBP;
	int i;

	LOG_M_vLogInfo("Begin Breakpoint Dump :");
	LST_M_DynamicForEachIndex(&g_stBreakpoints, pBP, i)
	{
		char szName[20];
		char szMsg[512];
		char szNoun[20], szVerb[20];

		fn_vGetNounName(&pBP->stNoun, szNoun);
		fn_vGetVerbName(&pBP->stVerb, szVerb);

		void *pv_Value;
		AI_fn_bGetDsgVar(pBP->p_stSpo, pBP->stNoun.lDsgVarId, NULL, &pv_Value);

		sprintf(szName, "Breakpoint %d:", i);
		sprintf(
			szMsg,
			"\tName: '%s'\n"
			"\tCurrent Value: %d\n"
			"\tPrevious Value: %d\n"
			"\tNoun '%s':\n"
			"\t\tType: %d\n"
			"\t\tDsgVar Id: %d\n"
			"\t\tDsgVar Type: %d\n"
			"\t\t+ DsgVar Offset: %p\n"
			"\t\t+ DsgVar Value: char: %d, short: %d, int: %d\n"
			"\tVerb '%s':\n"
			"\t\tType: %d\n"
			"\t\tValue: %d\n"
			"\tEnabled: %s\n"
			"\tDead: %s\n",
			pBP->szName, pBP->lCurrentValue, pBP->lPreviousValue,
			szNoun, pBP->stNoun.eType, pBP->stNoun.lDsgVarId, pBP->stNoun.eDsgVarType,
			pv_Value, *(char*)pv_Value, *(short*)pv_Value, *(int*)pv_Value,
			szVerb, pBP->stVerb.eType, pBP->stVerb.lValue,
			(pBP->bEnabled ? "YES" : "NO"), (pBP->bIsDead ? "YES" : "NO")
		);
		LOG_M_vLogInfoEx(szName, szMsg);
	}
}