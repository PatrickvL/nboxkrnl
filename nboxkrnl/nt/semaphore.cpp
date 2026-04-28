/*
 * PatrickvL              Copyright (c) 2026
 */

#include "nt.hpp"
#include "ke.hpp"
#include "ex.hpp"
#include "ob.hpp"


EXPORTNUM(193) NTSTATUS XBOXAPI NtCreateSemaphore
(
	PHANDLE SemaphoreHandle,
	POBJECT_ATTRIBUTES ObjectAttributes,
	LONG InitialCount,
	LONG MaximumCount
)
{
	if ((MaximumCount <= 0) || (InitialCount < 0) || (InitialCount > MaximumCount)) {
		return STATUS_INVALID_PARAMETER;
	}

	PVOID SemaphoreObject;
	NTSTATUS Status = ObCreateObject(&ExSemaphoreObjectType, ObjectAttributes, sizeof(KSEMAPHORE), &SemaphoreObject);

	if (NT_SUCCESS(Status)) {
		KeInitializeSemaphore((PKSEMAPHORE)SemaphoreObject, InitialCount, MaximumCount);
		Status = ObInsertObject(SemaphoreObject, ObjectAttributes, 0, SemaphoreHandle);
	}

	return Status;
}

EXPORTNUM(222) NTSTATUS XBOXAPI NtReleaseSemaphore
(
	HANDLE SemaphoreHandle,
	LONG ReleaseCount,
	PLONG PreviousCount
)
{
	PVOID SemaphoreObject;
	NTSTATUS Status = ObReferenceObjectByHandle(SemaphoreHandle, &ExSemaphoreObjectType, &SemaphoreObject);

	if (NT_SUCCESS(Status)) {
		LONG OldCount = KeReleaseSemaphore((PKSEMAPHORE)SemaphoreObject, PRIORITY_BOOST_SEMAPHORE, ReleaseCount, FALSE);
		if (PreviousCount) {
			*PreviousCount = OldCount;
		}
		ObfDereferenceObject(SemaphoreObject);
	}

	return Status;
}
