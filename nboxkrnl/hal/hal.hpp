/*
 * ergo720                Copyright (c) 2023
 * PatrickvL              Copyright (c) 2026
 */

#pragma once

#include "..\types.hpp"
#include "ke.hpp"


using PHAL_SHUTDOWN_NOTIFICATION = VOID(XBOXAPI *)(
	struct HAL_SHUTDOWN_REGISTRATION *ShutdownRegistration
	);

struct HAL_SHUTDOWN_REGISTRATION {
	PHAL_SHUTDOWN_NOTIFICATION NotificationRoutine;
	LONG Priority;
	LIST_ENTRY ListEntry;
};
using PHAL_SHUTDOWN_REGISTRATION = HAL_SHUTDOWN_REGISTRATION *;

#ifdef __cplusplus
extern "C" {
#endif

EXPORTNUM(40) DLLEXPORT extern ULONG HalDiskCachePartitionCount;

EXPORTNUM(43) DLLEXPORT VOID XBOXAPI HalEnableSystemInterrupt
(
	ULONG BusInterruptLevel,
	KINTERRUPT_MODE InterruptMode
);

EXPORTNUM(44) DLLEXPORT ULONG XBOXAPI HalGetInterruptVector
(
	ULONG BusInterruptLevel,
	PKIRQL Irql
);

EXPORTNUM(45) DLLEXPORT NTSTATUS XBOXAPI HalReadSMBusValue
(
	UCHAR SlaveAddress,
	UCHAR CommandCode,
	BOOLEAN ReadWordValue,
	ULONG *DataValue
);

EXPORTNUM(46) DLLEXPORT VOID XBOXAPI HalReadWritePCISpace
(
	ULONG BusNumber,
	ULONG SlotNumber,
	ULONG RegisterNumber,
	PVOID Buffer,
	ULONG Length,
	BOOLEAN WritePCISpace
);

EXPORTNUM(47) DLLEXPORT VOID XBOXAPI HalRegisterShutdownNotification
(
	PHAL_SHUTDOWN_REGISTRATION ShutdownRegistration,
	BOOLEAN Register
);

EXPORTNUM(48) DLLEXPORT VOID FASTCALL HalRequestSoftwareInterrupt
(
	KIRQL Request
);

EXPORTNUM(49) DLLEXPORT VOID XBOXAPI HalReturnToFirmware
(
	ULONG Routine
);

EXPORTNUM(50) DLLEXPORT NTSTATUS XBOXAPI HalWriteSMBusValue
(
	UCHAR SlaveAddress,
	UCHAR CommandCode,
	BOOLEAN WriteWordValue,
	ULONG DataValue
);

EXPORTNUM(9) DLLEXPORT VOID XBOXAPI HalReadSMCTrayState
(
	PULONG TrayState,
	PULONG TrayStateChangeCount
);

EXPORTNUM(41) DLLEXPORT extern UCHAR HalDiskModelNumber[40];

EXPORTNUM(42) DLLEXPORT extern UCHAR HalDiskSerialNumber[20];

EXPORTNUM(356) DLLEXPORT extern ULONG HalBootSMCVideoMode;

EXPORTNUM(358) DLLEXPORT BOOLEAN XBOXAPI HalIsResetOrShutdownPending();

EXPORTNUM(360) DLLEXPORT VOID XBOXAPI HalInitiateShutdown();

EXPORTNUM(365) DLLEXPORT VOID XBOXAPI HalEnableSecureTrayEject();

#ifdef __cplusplus
}
#endif

inline DWORD HalCounterPerMicroseconds;

BOOLEAN HalInitSystem();
