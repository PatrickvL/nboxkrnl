/*
 * PatrickvL              Copyright (c) 2026
 */

#pragma once

#include "..\types.hpp"


#ifdef __cplusplus
extern "C" {
#endif

EXPORTNUM(252) DLLEXPORT ULONG XBOXAPI PhyGetLinkState
(
	BOOLEAN Param
);

EXPORTNUM(253) DLLEXPORT BOOLEAN XBOXAPI PhyInitialize
(
	BOOLEAN Param1,
	PVOID Param2
);

#ifdef __cplusplus
}
#endif
