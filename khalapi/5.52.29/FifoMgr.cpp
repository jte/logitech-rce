/**
FifoInit
FifoAdd
FifoGet
FifoGetCount
 */
 
typedef struct
{
    DWORD Pos;
    DWORD in;
    DWORD out;
    DWORD ulItemSize;
    DWORD ulStorageSize;
    DWORD FifoSize;
} Fifo;

void FifoInit(CMainShare *base, Fifo *pFifo, unsigned int ptr, int ulStorageSize, int ulItemSize)
{
    pFifo->Pos = PtrToPos(base, ptr);
    pFifo->ulStorageSize = ulStorageSize;
    pFifo->ulItemSize = ulItemSize;
    pFifo->FifoSize = ulStorageSize / pFifo->ulItemSize;
    pFifo->in = 0;
    pFifo->out = 0;
}

int FifoAdd(CMainShare *pBase, Fifo *pFifo, void* src, HANDLE hMutex)
{
    if(hMutex)
    {
        if(CriticalRegionEnter(hMutex, 0))
        {
            int
                fifoCount = pFifo->in - pFifo->out;
            if(fifoCount < 0)
            {
                fifoCount += pFifo->FifoSize;
            }
            if(fifoCount < pFifo->FifoSize - 1)
            {
                void *dest = PosToPtr(pBase, pFifo->Pos + pFifo->ulItemSize * pFifo->in);
                memcpy_fast(dest, src, pFifo->ulItemSize);
                pFifo->in = (pFifo->in + 1) % pFifo->FifoSize;
                return fifoCount + 1;
            }
            CriticalRegionLeave(hMutex);
        }
    }
    return -1;
}

int FifoGet(CMainShare *pMainShare, Fifo *pFifo, unsigned int dest, HANDLE hMutex)
{
    BOOL bGotMutex = FALSE;
    if(hMutex)
    {
        bGotMutex = CriticalRegionEnter(hMutex, 0);
    }
    int
        fifoCount = pFifo->in - pFifo->out;
    if(fifoCount < 0)
    {
        fifoCount += pFifo->FifoSize;
    }
    if(fifoCount > 0)
    {
        void *src = PosToPtr(pMainShare, pFifo->Pos + pFifo->ulItemSize * pFifo->out);
        memcpy_fast(dest, src, pFifo->ulItemSize);
        pFifo->out = (unsigned int)(pFifo->out + 1) % pFifo->FifoSize;
    }
    if(bGotMutex)
    {
        CriticalRegionLeave(hMutex);
    }
    return fifoCount;
}

int FifoGetCount(Fifo *pFifo)
{
    int fifoCount = pFifo->in - pFifo->out;
    
    if(fifoCount < 0)
    {
        fifoCount += pFifo->FifoSize;
    }
    return fifoCount;
}