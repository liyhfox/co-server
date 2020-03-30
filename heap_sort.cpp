#include "heap_sort.h"

#define PARENT(i) (i - 1)/2
#define LEFT(i) i*2 + 1
#define RIGHT(i) i*2 + 2

void MaxHeapIFY(vector<uint32_t>& A, uint32_t i, uint32_t heap_size)
{
    uint32_t l = LEFT(i);
    uint32_t r = RIGHT(i);

    uint32_t largest = 0;
    if(l <= heap_size && A[l] >= A[i])
        largest = l;
    else
        largest = i;

    if(r <= heap_size && A[r] >= A[largest])
        largest = r;

    if(largest != i)
    {
        std::swap(A[i], A[largest]);
        MaxHeapIFY(A, largest, heap_size);
    }
}

void BuildMaxHeap(vector<uint32_t>& A, uint32_t heap_size)
{
    for(int32_t i = A.size() / 2 - 1; i >= 0; --i)
    {
        MaxHeapIFY(A, i, heap_size);
    }
}

void HeapSort(vector<uint32_t>& A, uint32_t heap_size)
{
    BuildMaxHeap(A, heap_size);
    for(uint32_t i = heap_size; i > 0; --i)
    {
        std::swap(A[0], A[i]);
        --heap_size;
        MaxHeapIFY(A, 0, heap_size);
    }
}
