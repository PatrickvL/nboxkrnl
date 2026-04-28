/*
 * PatrickvL              Copyright (c) 2026
 */

#include "ex.hpp"


EXPORTNUM(30) OBJECT_TYPE ExSemaphoreObjectType = {
	ExAllocatePoolWithTag,
	ExFreePool,
	nullptr,
	nullptr,
	nullptr,
	(PVOID)offsetof(KSEMAPHORE, Header),
	'ameS'
};
