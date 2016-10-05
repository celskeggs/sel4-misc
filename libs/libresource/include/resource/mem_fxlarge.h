#ifndef LIB_RESOURCE_MEM_FXLARGE_H
#define LIB_RESOURCE_MEM_FXLARGE_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx

// this is for allocating single-page chunks of memory

void *mem_fxlarge_alloc(void);
void mem_fxlarge_free(void *data);

#endif //LIB_RESOURCE_MEM_FXLARGE_H
