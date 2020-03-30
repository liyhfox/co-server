#include "merge_sort.h"

void Merge(vector<uint32_t>& A, uint32_t p, uint32_t r, uint32_t q)
{
    vector<uint32_t> L;
    for(uint32_t i = p; i <= r; ++i)
    {
        L.push_back(A[i]);
    }
    L.push_back(static_cast<uint32_t>(-1));

    vector<uint32_t> R;
    for(uint32_t j = r + 1; j <= q; ++j)
    {
        R.push_back(A[j]);
    }
    R.push_back(static_cast<uint32_t>(-1));

    uint32_t i = 0;
    uint32_t j = 0;
    for(uint32_t k = p; k <= q; ++k)
    {
        if(L[i] <= R[j])
        {
            A[k] = L[i];
            ++i;
        }
        else
        {
            A[k] = R[j];
            ++j;
        }
    }
}

void MergeSort(vector<uint32_t>& A, uint32_t p, uint32_t q)
{
    if(p < q)
    {
        int r = (q + p) / 2;
        MergeSort(A, p, r);
        MergeSort(A, r + 1, q);
        Merge(A, p, r, q);
    }
}
