/*
* ergo720                Copyright (c) 2025
* PatrickvL              Copyright (c) 2026
*/

#include "nt.hpp"
#include "ob.hpp"
#include "rtl.hpp"


EXPORTNUM(196) NTSTATUS XBOXAPI NtDeviceIoControlFile
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
)
{
	return IopControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength, OutputBuffer,
		OutputBufferLength, IRP_MJ_DEVICE_CONTROL);
}


EXPORTNUM(215) NTSTATUS XBOXAPI NtQuerySymbolicLinkObject
(
	HANDLE LinkHandle,
	PSTRING LinkTarget,
	PULONG ReturnedLength
)
{
	PVOID LinkObject;
	NTSTATUS Status = ObReferenceObjectByHandle(LinkHandle, &ObSymbolicLinkObjectType, &LinkObject);

	if (NT_SUCCESS(Status)) {
		POBJECT_SYMBOLIC_LINK SymLink = (POBJECT_SYMBOLIC_LINK)LinkObject;
		ULONG RequiredLength = SymLink->LinkTarget.Length;

		if (ReturnedLength) {
			*ReturnedLength = RequiredLength;
		}

		if (LinkTarget->MaximumLength >= RequiredLength) {
			RtlCopyString(LinkTarget, &SymLink->LinkTarget);
		}
		else {
			Status = STATUS_BUFFER_TOO_SMALL;
		}

		ObfDereferenceObject(LinkObject);
	}

	return Status;
}

EXPORTNUM(217) NTSTATUS XBOXAPI NtQueryVirtualMemory
(
	PVOID BaseAddress,
	PVOID MemoryInformation
)
{
	// Stub: return STATUS_SUCCESS, the caller should check the output size
	return STATUS_SUCCESS;
}
