/*
 * ergo720                Copyright (c) 2023
 * PatrickvL              Copyright (c) 2026
 */

#include "zw.hpp"
#include "nt.hpp"
#include "obp.hpp"
#include "ps.hpp"


 // clang-format off
VOID XBOXAPI ZwContinue(PCONTEXT ContextRecord, BOOLEAN TestAlert)
{
	__asm {
		mov ecx, ContextRecord
		movzx edx, TestAlert
		int IDT_SERVICE_VECTOR_BASE + 8 // calls KiContinueService
	}
}

VOID XBOXAPI ZwRaiseException(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord, BOOLEAN FirstChance)
{
	__asm {
		mov ecx, ExceptionRecord
		mov edx, ContextRecord
		movzx eax, FirstChance
		int IDT_SERVICE_VECTOR_BASE + 9 // calls KiRaiseExceptionService
	}
}
// clang-format on


EXPORTNUM(238) NTSTATUS XBOXAPI NtYieldExecution()
{
	// On Xbox with a single CPU, yield is essentially a no-op
	return STATUS_NO_YIELD_PERFORMED;
}

EXPORTNUM(224) NTSTATUS XBOXAPI NtResumeThread
(
	HANDLE ThreadHandle,
	PULONG PreviousSuspendCount
)
{
	PVOID ThreadObject;
	NTSTATUS Status = ObReferenceObjectByHandle(ThreadHandle, &PsThreadObjectType, &ThreadObject);

	if (NT_SUCCESS(Status)) {
		ULONG Count = KeResumeThread((PKTHREAD)ThreadObject);
		if (PreviousSuspendCount) {
			*PreviousSuspendCount = Count;
		}
		ObfDereferenceObject(ThreadObject);
	}

	return Status;
}

EXPORTNUM(228) NTSTATUS XBOXAPI NtSetSystemTime
(
	PLARGE_INTEGER SystemTime,
	PLARGE_INTEGER PreviousTime
)
{
	LARGE_INTEGER CurrentTime;
	KeQuerySystemTime(&CurrentTime);

	if (PreviousTime) {
		*PreviousTime = CurrentTime;
	}

	// Setting the system time is not supported in emulation
	return STATUS_SUCCESS;
}

EXPORTNUM(195) NTSTATUS XBOXAPI NtDeleteFile
(
	POBJECT_ATTRIBUTES ObjectAttributes
)
{
	HANDLE FileHandle;
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Status = NtCreateFile(
		&FileHandle,
		DELETE,
		ObjectAttributes,
		&IoStatusBlock,
		nullptr,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_DELETE,
		FILE_OPEN,
		FILE_DELETE_ON_CLOSE
	);

	if (NT_SUCCESS(Status)) {
		NtClose(FileHandle);
	}

	return Status;
}
