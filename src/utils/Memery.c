#include "Memery.h"

u8 mem_equal(const u8 *p, const u8 *q, u8 len)
{
    while (len > 0)
    {
        if (*p != *q)
        {
            return 0;
        }
        p++;
        q++;
        len--;
    }
    return 1;
}

void mem_copy(u8 *dst, u8 *src, u8 len)
{
    while (len > 0)
    {
        *dst = *src;
        dst++;
        src++;
        len--;
    }
}

void mem_fill(u8 *dst, u8 dat, u8 len)
{
    while (len > 0)
    {
        *dst = dat;
        dst++;
        len--;
    }
}
