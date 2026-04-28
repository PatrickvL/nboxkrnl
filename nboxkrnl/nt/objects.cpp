/*
 * ergo720                Copyright (c) 2023
 * PatrickvL              Copyright (c) 2026
 */

#include "nt.hpp"
#include "obp.hpp"
#include "rtl.hpp"
#include "ex.hpp"
#include "ps.hpp"
#include <string.h>

#ifndef MAXIMUM_WAIT_OBJECTS
#define MAXIMUM_WAIT_OBJECTS 64
#endif


EXPORTNUM(187) NTSTATUS XBOXAPI NtClose
(
	HANDLE Handle
)
{
	KIRQL OldIrql = ObLock();

	if (PVOID Object = ObpDestroyObjectHandle(Handle); Object != NULL_HANDLE) {
		POBJECT_HEADER Obj = GetObjHeader(Object);
		LONG HandleCount = Obj->HandleCount;
		--Obj->HandleCount;

		if (Obj->Type->CloseProcedure) {
			ObUnlock(OldIrql);
			Obj->Type->CloseProcedure(Object, HandleCount);
			OldIrql = ObLock();
		}

		if ((Obj->HandleCount == 0) && (Obj->Flags & OB_FLAG_ATTACHED_OBJECT) && !(Obj->Flags & OB_FLAG_PERMANENT_OBJECT)) {
			ObpDetachNamedObject(Object, OldIrql);
		}
		else {
			ObUnlock(OldIrql);
		}

		ObfDereferenceObject(Object);

		return STATUS_SUCCESS;
	}

	ObUnlock(OldIrql);

	return STATUS_INVALID_HANDLE;
}

EXPORTNUM(188) NTSTATUS XBOXAPI NtCreateDirectoryObject
(
	PHANDLE DirectoryHandle,
	POBJECT_ATTRIBUTES ObjectAttributes
)
{
	*DirectoryHandle = NULL_HANDLE;
	PVOID Object;
	NTSTATUS Status = ObCreateObject(&ObDirectoryObjectType, ObjectAttributes, sizeof(OBJECT_DIRECTORY), &Object);

	if (NT_SUCCESS(Status)) {
		memset(Object, 0, sizeof(OBJECT_DIRECTORY));
		Status = ObInsertObject(Object, ObjectAttributes, 0, DirectoryHandle);
	}

	return Status;
}

EXPORTNUM(203) NTSTATUS XBOXAPI NtOpenSymbolicLinkObject
(
	PHANDLE LinkHandle,
	POBJECT_ATTRIBUTES ObjectAttributes
)
{
	return ObOpenObjectByName(ObjectAttributes, &ObSymbolicLinkObjectType, nullptr, LinkHandle);
}

EXPORTNUM(233) NTSTATUS XBOXAPI NtWaitForSingleObject
(
	HANDLE Handle,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
)
{
	return NtWaitForSingleObjectEx(Handle, KernelMode, Alertable, Timeout);
}

EXPORTNUM(234) NTSTATUS XBOXAPI NtWaitForSingleObjectEx
(
	HANDLE Handle,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
)
{
	PVOID Object;
	NTSTATUS Status = ObReferenceObjectByHandle(Handle, nullptr, &Object);

	if (NT_SUCCESS(Status)) {
		// The following has to extract the member DISPATCHER_HEADER::Header of the object to be waited on. If Handle refers to an object that cannot be waited on,
		// then OBJECT_TYPE::DefaultObject holds the address of ObpDefaultObject, which is a global KEVENT always signalled, and thus the wait will be satisfied
		// immediately. Notice that, because the kernel has a fixed load address of 0x80010000, then &ObpDefaultObject will be interpreted as a negative number,
		// allowing us to distinguish the can't wait/can wait kinds of objects apart.

		POBJECT_HEADER Obj = GetObjHeader(Object); // get Ob header (NOT the same as DISPATCHER_HEADER)
		PVOID ObjectToWaitOn = Obj->Type->DefaultObject;

		if ((LONG_PTR)ObjectToWaitOn >= 0) {
			ObjectToWaitOn = (PCHAR)Object + (ULONG_PTR)ObjectToWaitOn; // DefaultObject is the offset of DISPATCHER_HEADER::Header
		}

		// KeWaitForSingleObject will raise an exception if the Handle is a mutant and its limit is exceeded
		__try {
			Status = KeWaitForSingleObject(ObjectToWaitOn, UserRequest, WaitMode, Alertable, Timeout);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			Status = GetExceptionCode();
		}

		ObfDereferenceObject(Object);
	}

	return Status;
}


EXPORTNUM(235) NTSTATUS XBOXAPI NtWaitForMultipleObjectsEx
(
	ULONG Count,
	HANDLE Handles[],
	WAIT_TYPE WaitType,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
)
{
	PVOID Objects[MAXIMUM_WAIT_OBJECTS];
	PVOID WaitObjects[MAXIMUM_WAIT_OBJECTS];

	if ((Count == 0) || (Count > MAXIMUM_WAIT_OBJECTS)) {
		return STATUS_INVALID_PARAMETER;
	}

	// Reference all objects
	for (ULONG i = 0; i < Count; i++) {
		NTSTATUS Status = ObReferenceObjectByHandle(Handles[i], nullptr, &Objects[i]);
		if (!NT_SUCCESS(Status)) {
			for (ULONG j = 0; j < i; j++) {
				ObfDereferenceObject(Objects[j]);
			}
			return Status;
		}

		POBJECT_HEADER Obj = GetObjHeader(Objects[i]);
		PVOID ObjectToWaitOn = Obj->Type->DefaultObject;
		if ((LONG_PTR)ObjectToWaitOn >= 0) {
			ObjectToWaitOn = (PCHAR)Objects[i] + (ULONG_PTR)ObjectToWaitOn;
		}
		WaitObjects[i] = ObjectToWaitOn;
	}

	NTSTATUS Status;
	__try {
		Status = KeWaitForMultipleObjects(Count, WaitObjects, WaitType, UserRequest, WaitMode, Alertable, Timeout, nullptr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		Status = GetExceptionCode();
	}

	for (ULONG i = 0; i < Count; i++) {
		ObfDereferenceObject(Objects[i]);
	}

	return Status;
}

EXPORTNUM(230) NTSTATUS XBOXAPI NtSignalAndWaitForSingleObjectEx
(
	HANDLE SignalHandle,
	HANDLE WaitHandle,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
)
{
	PVOID SignalObject;
	NTSTATUS Status = ObReferenceObjectByHandle(SignalHandle, nullptr, &SignalObject);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	PVOID WaitObject;
	Status = ObReferenceObjectByHandle(WaitHandle, nullptr, &WaitObject);
	if (!NT_SUCCESS(Status)) {
		ObfDereferenceObject(SignalObject);
		return Status;
	}

	POBJECT_HEADER SignalObjHeader = GetObjHeader(SignalObject);
	POBJECT_HEADER WaitObjHeader = GetObjHeader(WaitObject);

	PVOID ObjectToWaitOn = WaitObjHeader->Type->DefaultObject;
	if ((LONG_PTR)ObjectToWaitOn >= 0) {
		ObjectToWaitOn = (PCHAR)WaitObject + (ULONG_PTR)ObjectToWaitOn;
	}

	// Signal the signal object
	DISPATCHER_HEADER *SignalHeader = (DISPATCHER_HEADER *)((PCHAR)SignalObject + (ULONG_PTR)SignalObjHeader->Type->DefaultObject);
	if (SignalObjHeader->Type == &ExEventObjectType) {
		KeSetEvent((PKEVENT)SignalHeader, PRIORITY_BOOST_EVENT, TRUE);
	}
	else if (SignalObjHeader->Type == &ExMutantObjectType) {
		KeReleaseMutant((PKMUTANT)SignalHeader, PRIORITY_BOOST_MUTANT, FALSE, TRUE);
	}
	else {
		KeReleaseSemaphore((PKSEMAPHORE)SignalHeader, PRIORITY_BOOST_SEMAPHORE, 1, TRUE);
	}

	__try {
		Status = KeWaitForSingleObject(ObjectToWaitOn, UserRequest, WaitMode, Alertable, Timeout);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		Status = GetExceptionCode();
	}

	ObfDereferenceObject(WaitObject);
	ObfDereferenceObject(SignalObject);
	return Status;
}
