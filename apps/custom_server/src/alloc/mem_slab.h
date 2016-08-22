#ifndef SEL4_MISC_MEM_SLAB_H
#define SEL4_MISC_MEM_SLAB_H

#define SLAB_BITS 4
#define SLAB_SIZE (1 << SLAB_BITS)

void *slab_allocate();
void slab_free(void *slab);

#endif //SEL4_MISC_MEM_SLAB_H
