#ifndef __MEMERY_H__
#include "../config.h"

u8 mem_equal(const u8 *p, const u8 *q, u8 len);

void mem_copy(u8 *dst, u8 *src, u8 len);

void mem_fill(u8 *dst, u8 dat, u8 len);

#endif // !__MEMERY_H__