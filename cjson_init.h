#ifndef CJON_WRAPPER_H
#define CJON_WRAPPER_H

#include <stdint.h>
#include <cJSON.h>
#include <hi_mem.h>

void *cJsonMalloc(size_t sz);
void cJsonFree(const char *p);
void cJsonInit(void);

#endif  // CJON_WRAPPER_H