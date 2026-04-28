/*
* ergo720                Copyright (c) 2025
* PatrickvL              Copyright (c) 2026
*/

#pragma once

#include "..\types.hpp"


#ifdef __cplusplus
extern "C" {
#endif

EXPORTNUM(1) DLLEXPORT PVOID XBOXAPI AvGetSavedDataAddress();

EXPORTNUM(2) DLLEXPORT VOID XBOXAPI AvSendTVEncoderOption
(
	PVOID RegisterBase,
	ULONG Option,
	ULONG Param,
	ULONG *Result
);

EXPORTNUM(3) DLLEXPORT ULONG XBOXAPI AvSetDisplayMode
(
	PVOID RegisterBase,
	ULONG Step,
	ULONG Mode,
	ULONG Format,
	ULONG Pitch,
	ULONG FrameBuffer
);

EXPORTNUM(4) DLLEXPORT VOID XBOXAPI AvSetSavedDataAddress
(
	PVOID Address
);

#ifdef __cplusplus
}
#endif
