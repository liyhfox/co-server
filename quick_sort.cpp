#include <stdlib.h>
#include <time.h>

#include "quick_sort.h"

uint32_t Partition(vector<uint32_t>& array, uint32_t p, uint32_t q)
{
    srand(time(0));
    uint32_t k = rand() % (q - p) + p;
    std::swap(array[p], array[k]);

    uint32_t val = array[p];
    uint32_t i = p;
    for(uint32_t j = p + 1; j <= q; ++j)
    {
        if(array[j] <= val)
        {
           ++i;
           std::swap(array[j], array[i]);
        }
    }

    std::swap(array[p], array[i]);
    return i;
}

void QuickSort(vector<uint32_t>& array, uint32_t p, uint32_t q)
{
    if(p < q)
    {
        uint32_t pivot = Partition(array, p, q);
        QuickSort(array, p, pivot-1);
        QuickSort(array, pivot+1, q);
    }
}
