/*
 * ergo720                Copyright (c) 2023
 * LukeUsher              Copyright (c) 2018
 * PatrickvL              Copyright (c) 2026
 */

#include "ki.hpp"
#include "rtl.hpp"
#include "ex.hpp"

#ifndef MAXIMUM_WAIT_OBJECTS
#define MAXIMUM_WAIT_OBJECTS 64
#endif


// Source: Cxbx-Reloaded
static VOID KiWaitSatisfyMutant(PKMUTANT Mutant, PKTHREAD Thread)
{
	Mutant->Header.SignalState -= 1;
	if (Mutant->Header.SignalState == 0) {
		Mutant->OwnerThread = Thread;
		if (Mutant->Abandoned == TRUE) {
			Mutant->Abandoned = FALSE;
			Thread->WaitStatus = STATUS_ABANDONED;
		}

		InsertHeadList(Thread->MutantListHead.Blink, &Mutant->MutantListEntry);
	}
}

// Source: Cxbx-Reloaded
static VOID KiWaitSatisfyAny(PVOID Object, PKTHREAD Thread)
{
	PKMUTANT Mutant = (PKMUTANT)Object;
	if ((Mutant->Header.Type & SYNCHRONIZATION_OBJECT_TYPE_MASK) == EventSynchronizationObject) {
		Mutant->Header.SignalState = 0;
	}
	else if (Mutant->Header.Type == SemaphoreObject) {
		Mutant->Header.SignalState -= 1;
	}
	else if (Mutant->Header.Type == MutantObject) {
		KiWaitSatisfyMutant(Mutant, Thread);
	}
}

// Source: Cxbx-Reloaded
static VOID KiWaitSatisfyAll(PKWAIT_BLOCK WaitBlock)
{
	PKWAIT_BLOCK CurrWaitBlock = WaitBlock;
	PKTHREAD Thread = CurrWaitBlock->Thread;
	do {
		if (CurrWaitBlock->WaitKey != (USHORT)STATUS_TIMEOUT) {
			KiWaitSatisfyAny(CurrWaitBlock->Object, Thread);
		}
		CurrWaitBlock = CurrWaitBlock->NextWaitBlock;
	} while (CurrWaitBlock != WaitBlock);
}

EXPORTNUM(99) NTSTATUS XBOXAPI KeDelayExecutionThread
(
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Interval
)
{
	PKTHREAD Thread = KeGetCurrentThread();
	if (Thread->WaitNext) {
		Thread->WaitNext = FALSE;
	}
	else {
		Thread->WaitIrql = KeRaiseIrqlToDpcLevel();
	}

	NTSTATUS Status;
	LARGE_INTEGER DueTime, DummyTime;
	PLARGE_INTEGER CapturedTimeout = Interval;
	BOOLEAN HasWaited = FALSE;
	while (true) {
		Thread->WaitStatus = STATUS_SUCCESS;

		if (Alertable) {
			RIP_API_MSG("Thread alerts are not supported");
		}
		else if ((WaitMode == UserMode) && Thread->ApcState.UserApcPending) {
			Status = STATUS_USER_APC;
			break;
		}

		PKTIMER Timer = &Thread->Timer;
		PKWAIT_BLOCK WaitTimer = &Thread->TimerWaitBlock;
		Timer->Header.WaitListHead.Flink = &WaitTimer->WaitListEntry;
		Timer->Header.WaitListHead.Blink = &WaitTimer->WaitListEntry;
		WaitTimer->NextWaitBlock = WaitTimer;
		Thread->WaitBlockList = WaitTimer;
		if (KiInsertTimer(Timer, *Interval) == FALSE) {
			Status = STATUS_SUCCESS;
			break;
		}

		DueTime.QuadPart = Timer->DueTime.QuadPart;

		if (Thread->Queue) {
			RIP_API_MSG("Thread queues are not supported");
		}

		Thread->Alertable = Alertable;
		Thread->WaitMode = WaitMode;
		Thread->WaitReason = DelayExecution;
		Thread->WaitTime = KeTickCount;
		Thread->State = Waiting;
		InsertTailList(&KiWaitInListHead, &Thread->WaitListEntry);

		Status = KiSwapThread(); // returns either with a kernel APC or when the wait is satisfied
		HasWaited = TRUE;

		if (Status == STATUS_USER_APC) {
			RIP_API_MSG("User APCs are not supported");
		}

		if (Status != STATUS_KERNEL_APC) {
			if (Status == STATUS_TIMEOUT) {
				return STATUS_SUCCESS;
			}
			return Status;
		}

		Interval = KiRecalculateTimerDueTime(CapturedTimeout, &DueTime, &DummyTime);

		Thread->WaitIrql = KeRaiseIrqlToDpcLevel();
	}

	if (HasWaited == FALSE) {
		KiAdjustQuantumThread();
	}

	KiUnlockDispatcherDatabase(Thread->WaitIrql);
	if (Status == STATUS_USER_APC) {
		RIP_API_MSG("User APCs are not supported");
	}

	return Status;
}

// Source: partially from Cxbx-Reloaded
EXPORTNUM(159) DLLEXPORT NTSTATUS XBOXAPI KeWaitForSingleObject
(
	PVOID Object,
	KWAIT_REASON WaitReason,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
)
{
	PKTHREAD Thread = KeGetCurrentThread();
	if (Thread->WaitNext) {
		Thread->WaitNext = FALSE;
	}
	else {
		Thread->WaitIrql = KeRaiseIrqlToDpcLevel();
	}

	NTSTATUS Status;
	KWAIT_BLOCK LocalWaitBlock;
	LARGE_INTEGER DueTime, DummyTime;
	PLARGE_INTEGER CapturedTimeout = Timeout;
	PKWAIT_BLOCK WaitBlock = &LocalWaitBlock;
	BOOLEAN HasWaited = FALSE;
	while (true) {
		PKMUTANT Mutant = (PKMUTANT)Object;
		Thread->WaitStatus = STATUS_SUCCESS;

		if (Mutant->Header.Type == MutantObject) {
			if ((Mutant->Header.SignalState > 0) || (Thread == Mutant->OwnerThread)) { // not owned or owned by current thread
				if (Mutant->Header.SignalState != MUTANT_LIMIT) {
					KiWaitSatisfyMutant(Mutant, Thread);
					Status = Thread->WaitStatus;
					break;
				}
				else {
					KiUnlockDispatcherDatabase(Thread->WaitIrql);
					ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED); // won't return
				}
			}
		}
		else if (Mutant->Header.SignalState > 0) {
			if ((Mutant->Header.Type & SYNCHRONIZATION_OBJECT_TYPE_MASK) == EventSynchronizationObject) {
				Mutant->Header.SignalState = 0;
			}
			else if (Mutant->Header.Type == SemaphoreObject) {
				Mutant->Header.SignalState -= 1;
			}
			Status = STATUS_SUCCESS;
			break;
		}

		Thread->WaitBlockList = WaitBlock;
		WaitBlock->Object = Object;
		WaitBlock->WaitKey = (USHORT)STATUS_SUCCESS;
		WaitBlock->WaitType = WaitAny;
		WaitBlock->Thread = Thread;

		if (Alertable) {
			RIP_API_MSG("Thread alerts are not supported");
		}
		else if ((WaitMode == UserMode) && Thread->ApcState.UserApcPending) {
			Status = STATUS_USER_APC;
			break;
		}

		if (Timeout) {
			if (Timeout->QuadPart == 0) {
				Status = STATUS_TIMEOUT;
				break;
			}

			PKTIMER Timer = &Thread->Timer;
			PKWAIT_BLOCK WaitTimer = &Thread->TimerWaitBlock;
			WaitBlock->NextWaitBlock = WaitTimer;
			Timer->Header.WaitListHead.Flink = &WaitTimer->WaitListEntry;
			Timer->Header.WaitListHead.Blink = &WaitTimer->WaitListEntry;
			WaitTimer->NextWaitBlock = WaitBlock;
			if (KiInsertTimer(Timer, *Timeout) == FALSE) {
				Status = STATUS_TIMEOUT;
				break;
			}

			DueTime.QuadPart = Timer->DueTime.QuadPart;
		}
		else {
			WaitBlock->NextWaitBlock = WaitBlock;
		}

		InsertTailList(&Mutant->Header.WaitListHead, &WaitBlock->WaitListEntry);

		if (Thread->Queue) {
			RIP_API_MSG("Thread queues are not supported");
		}

		Thread->Alertable = Alertable;
		Thread->WaitMode = WaitMode;
		Thread->WaitReason = (UCHAR)WaitReason;
		Thread->WaitTime = KeTickCount;
		Thread->State = Waiting;
		InsertTailList(&KiWaitInListHead, &Thread->WaitListEntry);

		Status = KiSwapThread(); // returns either with a kernel APC or when the wait is satisfied
		HasWaited = TRUE;

		if (Status == STATUS_USER_APC) {
			RIP_API_MSG("User APCs are not supported");
		}

		if (Status != STATUS_KERNEL_APC) {
			return Status;
		}

		if (Timeout) {
			Timeout = KiRecalculateTimerDueTime(CapturedTimeout, &DueTime, &DummyTime);
		}

		Thread->WaitIrql = KeRaiseIrqlToDpcLevel();
	}

	if (HasWaited == FALSE) {
		KiAdjustQuantumThread();
	}

	KiUnlockDispatcherDatabase(Thread->WaitIrql);
	if (Status == STATUS_USER_APC) {
		RIP_API_MSG("User APCs are not supported");
	}

	return Status;
}

VOID KiWaitTest(PVOID Object, KPRIORITY Increment)
{
	PKMUTANT FirstObject = (PKMUTANT)Object;
	PLIST_ENTRY WaitList = &FirstObject->Header.WaitListHead;
	PLIST_ENTRY WaitEntry = WaitList->Flink;

	while ((FirstObject->Header.SignalState > 0) && (WaitEntry != WaitList)) {
		PKWAIT_BLOCK WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
		PKTHREAD WaitThread = WaitBlock->Thread;

		if (WaitBlock->WaitType == WaitAny) {
			KiWaitSatisfyAny(FirstObject, WaitThread);
		}
		else {
			PKWAIT_BLOCK NextBlock = WaitBlock->NextWaitBlock;
			while (NextBlock != WaitBlock) {
				if (NextBlock->WaitKey != STATUS_TIMEOUT) {
					PKMUTANT Mutant = (PKMUTANT)NextBlock->Object;
					if (Mutant->Header.Type == MutantObject) {
						RIP_API_MSG("Mutant objects are not supported");
					}
					else if (Mutant->Header.SignalState <= 0) {
						goto NextWaitEntry;
					}
				}

				NextBlock = NextBlock->NextWaitBlock;
			}

			KiWaitSatisfyAll(WaitBlock);
		}

		KiUnwaitThread(WaitThread, WaitBlock->WaitKey, Increment);
	NextWaitEntry:
		WaitEntry = WaitEntry->Flink;
	}
}

VOID KiUnwaitThread(PKTHREAD Thread, LONG_PTR WaitStatus, KPRIORITY Increment)
{
	Thread->WaitStatus |= WaitStatus;
	PKWAIT_BLOCK WaitBlock = Thread->WaitBlockList;
	do {
		RemoveEntryList(&WaitBlock->WaitListEntry);
		WaitBlock = WaitBlock->NextWaitBlock;
	} while (WaitBlock != Thread->WaitBlockList);

	RemoveEntryList(&Thread->WaitListEntry);

	if (Thread->Timer.Header.Inserted) {
		KiRemoveTimer(&Thread->Timer);
	}

	if (Thread->Queue) {
		RIP_API_MSG("Thread queues are not supported");
	}

	if (Thread->Priority < LOW_REALTIME_PRIORITY) {
		KPRIORITY NewPriority = Thread->BasePriority + Increment;
		if (NewPriority > Thread->Priority) {
			if (NewPriority >= LOW_REALTIME_PRIORITY) {
				Thread->Priority = LOW_REALTIME_PRIORITY - 1;
			}
			else {
				Thread->Priority = (SCHAR)NewPriority;
			}
		}

		if (Thread->BasePriority >= TIME_CRITICAL_BASE_PRIORITY) {
			Thread->Quantum = Thread->ApcState.Process->ThreadQuantum;
		}
		else {
			Thread->Quantum -= WAIT_QUANTUM_DECREMENT;
			if (Thread->Quantum <= 0) {
				Thread->Quantum = Thread->ApcState.Process->ThreadQuantum;
				Thread->Priority -= 1;
				if (Thread->Priority < Thread->BasePriority) {
					Thread->Priority = Thread->BasePriority;
				}
			}
		}
	}
	else {
		Thread->Quantum = Thread->ApcState.Process->ThreadQuantum;
	}

	KiScheduleThread(Thread);
}


EXPORTNUM(158) NTSTATUS XBOXAPI KeWaitForMultipleObjects
(
	ULONG Count,
	PVOID Object[],
	WAIT_TYPE WaitType,
	KWAIT_REASON WaitReason,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout,
	PKWAIT_BLOCK WaitBlockArray
)
{
	// For single object, delegate to KeWaitForSingleObject
	if (Count == 1) {
		return KeWaitForSingleObject(Object[0], WaitReason, WaitMode, Alertable, Timeout);
	}

	PKTHREAD Thread = KeGetCurrentThread();
	if (Thread->WaitNext) {
		Thread->WaitNext = FALSE;
	}
	else {
		Thread->WaitIrql = KeRaiseIrqlToDpcLevel();
	}

	NTSTATUS Status;
	KWAIT_BLOCK StackWaitBlocks[MAXIMUM_WAIT_OBJECTS];
	PKWAIT_BLOCK WaitBlocks = WaitBlockArray ? WaitBlockArray : StackWaitBlocks;
	LARGE_INTEGER DueTime, DummyTime;
	PLARGE_INTEGER CapturedTimeout = Timeout;
	BOOLEAN HasWaited = FALSE;

	while (true) {
		Thread->WaitStatus = STATUS_SUCCESS;
		BOOLEAN AllSatisfied = TRUE;

		// Check if objects are already signaled
		for (ULONG i = 0; i < Count; i++) {
			PKMUTANT Mutant = (PKMUTANT)Object[i];

			if (Mutant->Header.Type == MutantObject) {
				if ((Mutant->Header.SignalState > 0) || (Thread == Mutant->OwnerThread)) {
					if (Mutant->Header.SignalState == MUTANT_LIMIT) {
						KiUnlockDispatcherDatabase(Thread->WaitIrql);
						ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);
					}
					if (WaitType == WaitAny) {
						KiWaitSatisfyMutant(Mutant, Thread);
						Status = (NTSTATUS)i;
						goto Done;
					}
				}
				else {
					AllSatisfied = FALSE;
				}
			}
			else if (Mutant->Header.SignalState > 0) {
				if (WaitType == WaitAny) {
					if ((Mutant->Header.Type & SYNCHRONIZATION_OBJECT_TYPE_MASK) == EventSynchronizationObject) {
						Mutant->Header.SignalState = 0;
					}
					else if (Mutant->Header.Type == SemaphoreObject) {
						Mutant->Header.SignalState -= 1;
					}
					Status = (NTSTATUS)i;
					goto Done;
				}
			}
			else {
				AllSatisfied = FALSE;
			}
		}

		if (AllSatisfied && (WaitType == WaitAll)) {
			// All objects are signaled, satisfy them all
			for (ULONG i = 0; i < Count; i++) {
				PKMUTANT Mutant = (PKMUTANT)Object[i];
				if (Mutant->Header.Type == MutantObject) {
					KiWaitSatisfyMutant(Mutant, Thread);
				}
				else if ((Mutant->Header.Type & SYNCHRONIZATION_OBJECT_TYPE_MASK) == EventSynchronizationObject) {
					Mutant->Header.SignalState = 0;
				}
				else if (Mutant->Header.Type == SemaphoreObject) {
					Mutant->Header.SignalState -= 1;
				}
			}
			Status = STATUS_SUCCESS;
			goto Done;
		}

		// Setup wait blocks
		for (ULONG i = 0; i < Count; i++) {
			WaitBlocks[i].Object = Object[i];
			WaitBlocks[i].WaitKey = (USHORT)i;
			WaitBlocks[i].WaitType = WaitType;
			WaitBlocks[i].Thread = Thread;
			WaitBlocks[i].NextWaitBlock = &WaitBlocks[i + 1];
		}

		Thread->WaitBlockList = &WaitBlocks[0];

		if (Alertable) {
			RIP_API_MSG("Thread alerts are not supported");
		}
		else if ((WaitMode == UserMode) && Thread->ApcState.UserApcPending) {
			Status = STATUS_USER_APC;
			break;
		}

		if (Timeout) {
			if (Timeout->QuadPart == 0) {
				Status = STATUS_TIMEOUT;
				break;
			}

			PKTIMER Timer = &Thread->Timer;
			PKWAIT_BLOCK WaitTimer = &Thread->TimerWaitBlock;
			WaitBlocks[Count - 1].NextWaitBlock = WaitTimer;
			Timer->Header.WaitListHead.Flink = &WaitTimer->WaitListEntry;
			Timer->Header.WaitListHead.Blink = &WaitTimer->WaitListEntry;
			WaitTimer->NextWaitBlock = &WaitBlocks[0];
			if (KiInsertTimer(Timer, *Timeout) == FALSE) {
				Status = STATUS_TIMEOUT;
				break;
			}
			DueTime.QuadPart = Timer->DueTime.QuadPart;
		}
		else {
			WaitBlocks[Count - 1].NextWaitBlock = &WaitBlocks[0];
		}

		// Insert wait blocks into object wait lists
		for (ULONG i = 0; i < Count; i++) {
			PKMUTANT Mutant = (PKMUTANT)Object[i];
			InsertTailList(&Mutant->Header.WaitListHead, &WaitBlocks[i].WaitListEntry);
		}

		if (Thread->Queue) {
			RIP_API_MSG("Thread queues are not supported");
		}

		Thread->Alertable = Alertable;
		Thread->WaitMode = WaitMode;
		Thread->WaitReason = (UCHAR)WaitReason;
		Thread->WaitTime = KeTickCount;
		Thread->State = Waiting;
		InsertTailList(&KiWaitInListHead, &Thread->WaitListEntry);

		Status = KiSwapThread();
		HasWaited = TRUE;

		if (Status == STATUS_USER_APC) {
			RIP_API_MSG("User APCs are not supported");
		}

		if (Status != STATUS_KERNEL_APC) {
			return Status;
		}

		if (Timeout) {
			Timeout = KiRecalculateTimerDueTime(CapturedTimeout, &DueTime, &DummyTime);
		}

		Thread->WaitIrql = KeRaiseIrqlToDpcLevel();
	}

Done:
	if (HasWaited == FALSE) {
		KiAdjustQuantumThread();
	}

	KiUnlockDispatcherDatabase(Thread->WaitIrql);
	if (Status == STATUS_USER_APC) {
		RIP_API_MSG("User APCs are not supported");
	}

	return Status;
}
