#include "framework.h"
#include "debug.h"
#include "misc.h"


void fn_vBreakpointFormatValue( tdstBreakpoint *p_stBreakpoint, int lValue, char *szStr_Out )
{
	if ( p_stBreakpoint->stNoun.eType == eN_DsgVar )
	{
		switch ( p_stBreakpoint->stNoun.eDsgVarType )
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
