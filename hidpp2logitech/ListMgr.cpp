#include <Windows.h>
#include "ListMgr.h"

ListItem* PosToPtr(int base, int pos)
{
	if(pos != LD_INVALID_POS)
	{
		return reinterpret_cast<ListItem*>(base + (pos & POS_OFFSET_MASK));
	}
	return NULL;
}

int** ListGetNext(int pBase, int *pPosition)
{
	if(pBase && pPosition)
	{
		ListItem
			*pItem = (ListItem *)PosToPtr(pBase, *pPosition);
		if(!pItem)
		{
			return NULL;
		}
		*pPosition = pItem->Next;
		if(*pPosition != LD_INVALID_POS)
		{
			pItem = PosToPtr(pBase, *pPosition);
			return &pItem->ppData;
		}
	}
	return NULL;
}

BOOL ListSwap(int pBase, ListItem *pList, int posA, int posB)
{
	if(pBase && posA != LD_INVALID_POS && posB != LD_INVALID_POS)
	{
		ListItem
			*pItemA = reinterpret_cast<ListItem *>(PosToPtr(pBase, posA)),
			*pItemB = reinterpret_cast<ListItem *>(PosToPtr(pBase, posB));
		if(pItemA && pItemB)
		{
			ListItem
				*pItemAPrev = (ListItem *)PosToPtr(pBase, pItemA->Previous),
				*pItemBPrev = (ListItem *)PosToPtr(pBase, pItemB->Previous),
				*pItemANext = (ListItem *)PosToPtr(pBase, pItemA->Next),
				*pItemBNext = (ListItem *)PosToPtr(pBase, pItemB->Next);
			if(pItemAPrev)
				pItemAPrev->Next = posB;
			else
				pList->Next = posB;
			if(pItemANext)
				pItemANext->Previous = posB;
			else
				pList->This = posB;
			if(pItemBPrev)
				pItemBPrev->Next = posA;
			else
				pList->Next = posA;
			if(pItemBNext)
				pItemBNext->Previous = posA;
			else
				pList->This = posA;
			pItemA->Previous = pItemB->Previous;
			pItemB->Previous = pItemA->Previous;
			pItemA->Next = pItemB->Next;
			pItemB->Next = pItemA->Next;
			return true;
		}
	}
	return false;
}

bool ListRemoveAt(int pBase, List *pList, int position)
{
	if(pBase && pList)
	{
		ListItem
			*pItem = reinterpret_cast<ListItem*>(PosToPtr(pBase, position));
		if(pItem)
		{
			ListItem
				*pPrevious = reinterpret_cast<ListItem*>(PosToPtr(pBase, pItem->Previous)),
				*pNext = reinterpret_cast<ListItem*>(PosToPtr(pBase, pItem->Next));
			if(pPrevious)
			{
				pPrevious->Next = pItem->Next;
			}
			else
			{
				pList->First = pItem->Next;
			}
			if(pNext)
			{
				pNext->Previous = pItem->Previous;
			}
			else
			{
				pList->Last = pItem->Previous;
			}
			--pList->ItemCount;
			pItem->This = LD_INVALID_POS;
			return true;
		}
	}
	return false;
}

int ListGetCount(List *pList)
{
	return (pList) ? pList->ItemCount : 0;
}

int** ListGetAtMoveNext(int pBase, int a2, int *pPosition)
{
	if ( pBase && pPosition )
	{
		ListItem
			*pItem = reinterpret_cast<ListItem*>(PosToPtr(pBase, *pPosition));
		if(pItem)
		{
			*pPosition = pItem->Next;
			return &pItem->ppData;
		}
	}
	return NULL;
}

int PtrToPos(struct CMainShare *pBase, ListItem* pItem)
{
	if((int)pBase <= (int)pItem)
	{
		int offset = (int)pItem - (int)pBase;
		if(offset <= pBase->shareHeader.lSize)
		{
			return (InterlockedIncrement((volatile LONG *)(base + 4)) << 20) & 0xFFF00000 | offset & 0xFFFFF;
		}
	}
	return LD_INVALID_POS;
}

int** ListGetFirst(int pBase, List *pList, int *pPosition)
{
	if(pBase && pList && pPosition)
	{
		*pPosition = ListGetFirstPosition(pList);
		if(*pPosition != LD_INVALID_POS)
		{
			ListItem
				*pItem = (ListItem *)PosToPtr(pBase, *pPosition);
			return &pItem->ppData;
		}
	}
	return NULL;
}

bool ListAssertPosition(struct CMainShare *pBase, int position)
{
	if(pBase && position != LD_INVALID_POS)
	{
		if((position & POS_OFFSET_MASK) <= pBase->shareHeader.lSize)
		{
			ListItem
				*pItem = (ListItem *)PosToPtr((int)pBase, position);
			if(pItem)
			{
				return pItem->This == position;
			}
		}
	}
	return false;
}

int ListCreate(unsigned int pBase, List *pList)
{
	if(pBase && pList)
	{
		if(pList->bSharedStorage || pList->ItemCount < pList->ListSize)
		{
			for(size_t i = 0; i < pList->ListSize; ++i)
			{
				int LastItemIndex = (i + pList->LastItemIndex) % pList->ListSize;
				ListItem
					*pLast = (ListItem *)(LastItemIndex * pList->ulItemSize + PosToPtr(pBase, pList->pListMgr));
				if(pLast->This == LD_INVALID_POS)
				{
					int LastPos = PtrToPos(pBase, pLast);
					ListItem
						*pPrev = (ListItem *)PosToPtr(pBase, pList->Last);
					if(pPrev)
					{
						pPrev->This = pList->Last;
						pPrev->Next = LastPos;
					}
					pLast->Previous = pList->Last;
					pLast->This = LastPos;
					pLast->Next = -1;
					if(pList->First == LD_INVALID_POS)
						pList->First = LastPos;
					pList->Last = LastPos;
					++pList->ItemCount;
					pList->LastItemIndex = (unsigned int)(LastPos + 1) % pList->ListSize;
					return LastPos;
				}
			}
		}
	}
	return LD_INVALID_POS;
}

int ListGetPos(CMainShare *pBase, int a2, int pData)
{
	if(pBase && pData)
	{
		ListItem
			*ptr = (ListItem *)(pData - 12);
		return ptr->This;
	}
	return LD_INVALID_POS;
}

void ListInit(CMainShare *pBase, List *pList, ListItem *ptr, size_t ulStorageSize, int a5, bool bSharedStorage)
{
	pList->pListMgr = PtrToPos(pBase, ptr);
	pList->ulStorageSize = ulStorageSize;
	pList->field_10 = a5;
	pList->ulItemSize = a5 + 12;
	pList->LastItemIndex = 0;
	pList->bSharedStorage = bSharedStorage;
	pList->ListSize = ulStorageSize / pList->ulItemSize;
	pList->First = -1;
	pList->Last = -1;
	pList->ItemCount = 0;
	if(!bSharedStorage)
	{
		memset(ptr, 0, ulStorageSize);
		for(size_t i = 0; i < pList->ListSize; i++ )
		{
			ListItem
				*pItem = PosToPtr(pBase, pList->pListMgr + i * pList->ulItemSize);
			pItem->This = -1;
			pItem->Next = -1;
			pItem->Previous = -1;
		}
	}
}

int ListGetFirstPosition(List *pList)
{
	return (pList) ? pList->First : LD_INVALID_POS;
}

int** ListGetAt(int pBase, int position)
{
	if(pBase && position != LD_INVALID_POS)
	{
		ListItem
			*pItem = PosToPtr(pBase, position);
		if(pItem)
		{
			if ( pItem->This == position )
			{
				return &pItem->ppData;
			}
		}
	}
	return NULL;
}
