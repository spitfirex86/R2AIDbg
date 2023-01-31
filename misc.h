#pragma once

#include "framework.h"
#include "debug.h"


void * fn_pvGetDsgVar( HIE_tdstSuperObject *p_stSpo, int idx, AI_tdeDsgVarType *eType_Out );
void * fn_pvGetDsgVarValue( HIE_tdstSuperObject *p_stSpo, int idx );
AI_tdeDsgVarType fn_eGetDsgVarType( HIE_tdstSuperObject *p_stSpo, int idx );
void fn_vEnumDsgVar( HIE_tdstSuperObject *p_stSpo, tdfnDsgVarEnum p_fnEnum );

void fn_vBreakpointFormatValue( tdstBreakpoint *p_stBreakpoint, int lValue, char *szStr_Out );
