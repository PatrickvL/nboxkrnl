/*
 * ergo720                Copyright (c) 2023
 * PatrickvL              Copyright (c) 2026
 */

#pragma once

#include "..\types.hpp"
#include "io.hpp"


#ifdef __cplusplus
extern "C" {
#endif

EXPORTNUM(184) DLLEXPORT NTSTATUS XBOXAPI NtAllocateVirtualMemory
(
	PVOID *BaseAddress,
	ULONG ZeroBits,
	PULONG AllocationSize,
	DWORD AllocationType,
	DWORD Protect
);

EXPORTNUM(187) DLLEXPORT NTSTATUS XBOXAPI NtClose
(
	HANDLE Handle
);

EXPORTNUM(188) DLLEXPORT NTSTATUS XBOXAPI NtCreateDirectoryObject
(
	PHANDLE DirectoryHandle,
	POBJECT_ATTRIBUTES ObjectAttributes
);

EXPORTNUM(190) DLLEXPORT NTSTATUS XBOXAPI NtCreateFile
(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER AllocationSize,
	ULONG FileAttributes,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions
);

EXPORTNUM(192) DLLEXPORT NTSTATUS XBOXAPI NtCreateMutant
(
	PHANDLE MutantHandle,
	POBJECT_ATTRIBUTES ObjectAttributes,
	BOOLEAN InitialOwner
);

EXPORTNUM(196) DLLEXPORT NTSTATUS XBOXAPI NtDeviceIoControlFile
(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG IoControlCode,
	PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
	ULONG OutputBufferLength
);

EXPORTNUM(199) DLLEXPORT NTSTATUS XBOXAPI NtFreeVirtualMemory
(
	PVOID *BaseAddress,
	PULONG FreeSize,
	ULONG FreeType
);

EXPORTNUM(202) DLLEXPORT NTSTATUS XBOXAPI NtOpenFile
(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG OpenOptions
);

EXPORTNUM(203) DLLEXPORT NTSTATUS XBOXAPI NtOpenSymbolicLinkObject
(
	PHANDLE LinkHandle,
	POBJECT_ATTRIBUTES ObjectAttributes
);

EXPORTNUM(204) DLLEXPORT NTSTATUS XBOXAPI NtProtectVirtualMemory
(
	PVOID *BaseAddress,
	PSIZE_T RegionSize,
	ULONG NewProtect,
	PULONG OldProtect
);

EXPORTNUM(211) DLLEXPORT NTSTATUS XBOXAPI NtQueryInformationFile
(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass
);

EXPORTNUM(218) DLLEXPORT NTSTATUS XBOXAPI NtQueryVolumeInformationFile
(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PFILE_FS_SIZE_INFORMATION FileInformation,
	ULONG Length,
	FS_INFORMATION_CLASS FileInformationClass
);

EXPORTNUM(219) DLLEXPORT NTSTATUS XBOXAPI NtReadFile
(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset
);

EXPORTNUM(221) DLLEXPORT NTSTATUS XBOXAPI NtReleaseMutant
(
	HANDLE MutantHandle,
	PLONG PreviousCount
);

EXPORTNUM(232) DLLEXPORT VOID XBOXAPI NtUserIoApcDispatcher
(
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG Reserved
);

EXPORTNUM(233) DLLEXPORT NTSTATUS XBOXAPI NtWaitForSingleObject
(
	HANDLE Handle,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
);

EXPORTNUM(234) DLLEXPORT NTSTATUS XBOXAPI NtWaitForSingleObjectEx
(
	HANDLE Handle,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
);

EXPORTNUM(236) DLLEXPORT NTSTATUS XBOXAPI NtWriteFile
(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset
);

EXPORTNUM(186) DLLEXPORT NTSTATUS XBOXAPI NtClearEvent
(
	HANDLE EventHandle
);

EXPORTNUM(189) DLLEXPORT NTSTATUS XBOXAPI NtCreateEvent
(
	PHANDLE EventHandle,
	POBJECT_ATTRIBUTES ObjectAttributes,
	EVENT_TYPE EventType,
	BOOLEAN InitialState
);

EXPORTNUM(193) DLLEXPORT NTSTATUS XBOXAPI NtCreateSemaphore
(
	PHANDLE SemaphoreHandle,
	POBJECT_ATTRIBUTES ObjectAttributes,
	LONG InitialCount,
	LONG MaximumCount
);

EXPORTNUM(195) DLLEXPORT NTSTATUS XBOXAPI NtDeleteFile
(
	POBJECT_ATTRIBUTES ObjectAttributes
);

EXPORTNUM(198) DLLEXPORT NTSTATUS XBOXAPI NtFlushBuffersFile
(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock
);

EXPORTNUM(200) DLLEXPORT NTSTATUS XBOXAPI NtFsControlFile
(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG FsControlCode,
	PVOID InputBuffer,
	ULONG InputBufferLength,
	PVOID OutputBuffer,
	ULONG OutputBufferLength
);

EXPORTNUM(207) DLLEXPORT NTSTATUS XBOXAPI NtQueryDirectoryFile
(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	PSTRING FileName,
	BOOLEAN RestartScan
);

EXPORTNUM(210) DLLEXPORT NTSTATUS XBOXAPI NtQueryFullAttributesFile
(
	POBJECT_ATTRIBUTES ObjectAttributes,
	PFILE_NETWORK_OPEN_INFORMATION FileInformation
);

EXPORTNUM(215) DLLEXPORT NTSTATUS XBOXAPI NtQuerySymbolicLinkObject
(
	HANDLE LinkHandle,
	PSTRING LinkTarget,
	PULONG ReturnedLength
);

EXPORTNUM(217) DLLEXPORT NTSTATUS XBOXAPI NtQueryVirtualMemory
(
	PVOID BaseAddress,
	PVOID MemoryInformation
);

EXPORTNUM(222) DLLEXPORT NTSTATUS XBOXAPI NtReleaseSemaphore
(
	HANDLE SemaphoreHandle,
	LONG ReleaseCount,
	PLONG PreviousCount
);

EXPORTNUM(224) DLLEXPORT NTSTATUS XBOXAPI NtResumeThread
(
	HANDLE ThreadHandle,
	PULONG PreviousSuspendCount
);

EXPORTNUM(225) DLLEXPORT NTSTATUS XBOXAPI NtSetEvent
(
	HANDLE EventHandle,
	PLONG PreviousState
);

EXPORTNUM(226) DLLEXPORT NTSTATUS XBOXAPI NtSetInformationFile
(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass
);

EXPORTNUM(228) DLLEXPORT NTSTATUS XBOXAPI NtSetSystemTime
(
	PLARGE_INTEGER SystemTime,
	PLARGE_INTEGER PreviousTime
);

EXPORTNUM(230) DLLEXPORT NTSTATUS XBOXAPI NtSignalAndWaitForSingleObjectEx
(
	HANDLE SignalHandle,
	HANDLE WaitHandle,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
);

EXPORTNUM(235) DLLEXPORT NTSTATUS XBOXAPI NtWaitForMultipleObjectsEx
(
	ULONG Count,
	HANDLE Handles[],
	WAIT_TYPE WaitType,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
);

EXPORTNUM(238) DLLEXPORT NTSTATUS XBOXAPI NtYieldExecution();

#ifdef __cplusplus
}
#endif
