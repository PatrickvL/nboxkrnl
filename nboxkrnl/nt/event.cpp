/*
 * PatrickvL              Copyright (c) 2026
 */

#include "nt.hpp"
#include "ke.hpp"
#include "ex.hpp"
#include "ob.hpp"
#include "rtl.hpp"


EXPORTNUM(189) NTSTATUS XBOXAPI NtCreateEvent
(
	PHANDLE EventHandle,
	POBJECT_ATTRIBUTES ObjectAttributes,
	EVENT_TYPE EventType,
	BOOLEAN InitialState
)
{
	PVOID EventObject;
	NTSTATUS Status = ObCreateObject(&ExEventObjectType, ObjectAttributes, sizeof(KEVENT), &EventObject);

	if (NT_SUCCESS(Status)) {
		KeInitializeEvent((PKEVENT)EventObject, EventType, InitialState);
		Status = ObInsertObject(EventObject, ObjectAttributes, 0, EventHandle);
	}

	return Status;
}

EXPORTNUM(225) NTSTATUS XBOXAPI NtSetEvent
(
	HANDLE EventHandle,
	PLONG PreviousState
)
{
	PVOID EventObject;
	NTSTATUS Status = ObReferenceObjectByHandle(EventHandle, &ExEventObjectType, &EventObject);

	if (NT_SUCCESS(Status)) {
		LONG OldState = KeSetEvent((PKEVENT)EventObject, PRIORITY_BOOST_EVENT, FALSE);
		if (PreviousState) {
			*PreviousState = OldState;
		}
		ObfDereferenceObject(EventObject);
	}

	return Status;
}

EXPORTNUM(186) NTSTATUS XBOXAPI NtClearEvent
(
	HANDLE EventHandle
)
{
	PVOID EventObject;
	NTSTATUS Status = ObReferenceObjectByHandle(EventHandle, &ExEventObjectType, &EventObject);

	if (NT_SUCCESS(Status)) {
		KeResetEvent((PKEVENT)EventObject);
		ObfDereferenceObject(EventObject);
	}

	return Status;
}
