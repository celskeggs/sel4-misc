#ifndef LIB_RESOURCE_MEM_FXLARGE_H
#define LIB_RESOURCE_MEM_FXLARGE_H

#include <bedrock/bedrock.h>
#include <bedrock/errx.h> // this module uses errx

// this is for allocating page-multiple chunks of memory

#define MEM_FXLARGE_MIN_PAGES 1
// note: nothing is hardcoded for MEM_FXLARGE_MAX_PAGES's value up until around 255 or so - it should probably be
// possible to raise it. it's more of an efficiency issue to avoid allocating too many vspace blocks.
#define MEM_FXLARGE_MAX_PAGES 2
#define MEM_FXLARGE_MAX_SIZE (MEM_FXLARGE_MAX_PAGES * PAGE_SIZE)

void *mem_fxlarge_alloc(uint8_t page_count);

void mem_fxlarge_free(void *data, uint8_t page_count);

#endif //LIB_RESOURCE_MEM_FXLARGE_H
