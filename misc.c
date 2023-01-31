#include "framework.h"
#include "debug.h"
#include "misc.h"


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