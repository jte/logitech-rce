/**
 */
#pragma once

#include "MainProc.h"

#define LD_INVALID_POS		(-1)
#define POS_OFFSET_MASK		(0xFFFFF)

struct List
{
	DWORD pListMgr;
	DWORD First;
	DWORD Last;
	DWORD ulItemSize;
	DWORD pad1;
	DWORD ItemCount;
	DWORD ulStorageSize;
	BOOL bSharedStorage;
	DWORD ListSize;
	DWORD LastItemIndex;
};

struct ListItem
{
	int Previous;
	int Next;
	int This;
	int *ppData;
};