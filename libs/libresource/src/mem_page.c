#include <resource/mem_page.h>
#include <resource/cslot.h>

extern seL4_CPtr current_vspace;

static untyped_4k_ref mem_page_tables[PAGE_TABLE_COUNT];
static uint16_t mem_page_counts[PAGE_TABLE_COUNT];

static inline uint16_t address_to_table_index(void *page_table) {
    uintptr_t page = (uintptr_t) page_table / PAGE_TABLE_SIZE;
    assert(page <= PAGE_TABLE_COUNT);
    return (uint16_t) page;
}

static bool untyped_retype_to(untyped_4k_ref ref, int type, int offset, int size_bits, seL4_CPtr ptr) {
    assert(ptr != seL4_CapNull);
    return cslot_retype(untyped_ptr_4k(ref), type, offset, size_bits, ptr, 1);
}

static bool map_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    assert(mem_page_tables[tid] == NULL); // otherwise, someone unmapped us without permission
    assert(mem_page_counts[tid] == 0);
    seL4_CompileTimeAssert(seL4_PageTableBits == BITS_4KIB);
    untyped_4k_ref ref = untyped_allocate_4k();
    if (ref == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_IA32_PageTable table = untyped_auxptr_4k(ref);
    if (!untyped_retype_to(ref, seL4_IA32_PageTableObject, 0, 0, table)) {
        untyped_free_4k(ref);
        ERRX_TRACEPOINT;
        return false;
    }
    int err = seL4_IA32_PageTable_Map(table, current_vspace, tid * PAGE_TABLE_SIZE,
                                      seL4_IA32_Default_VMAttributes);
    if (err != seL4_NoError) {
        untyped_free_4k(ref);
        ERRX_RAISE_SEL4(err);
        return false;
    }
    mem_page_tables[tid] = ref;
    mem_page_counts[tid] = 1;
    return true;
}

static void unmap_table_tid(uint16_t tid) {
    assert(mem_page_tables[tid] != NULL);
    assert(mem_page_counts[tid] == 0);
    untyped_4k_ref table = mem_page_tables[tid];
    mem_page_tables[tid] = NULL;
    untyped_free_4k(table);
}

static bool ref_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    if (mem_page_tables[tid] == NULL) {
        // must have been allocated by an outside source. don't bother.
        return true;
    } else {
        assert(mem_page_tables[tid] != NULL);
        assert(mem_page_counts[tid] <= PAGE_COUNT_PER_TABLE);
        mem_page_counts[tid] += 1;
        return false;
    }
}

static void unref_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    assert(mem_page_tables[tid] != NULL);
    assert(mem_page_counts[tid] > 0);
    if (--mem_page_counts[tid] == 0) {
        unmap_table_tid(tid);
    }
}

void mem_page_free(struct mem_page_cookie *data) {
    untyped_free_4k(data->ref);
    data->ref = NULL;
    if (data->unref_addr != NULL) {
        unref_table(data->unref_addr);
    }
}

bool mem_page_map(void *page, struct mem_page_cookie *cookie) {
    seL4_CompileTimeAssert(seL4_PageBits == BITS_4KIB);
    untyped_4k_ref ref = untyped_allocate_4k();
    if (ref == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_IA32_Page pent = untyped_auxptr_4k(ref);
    if (!untyped_retype_to(ref, seL4_IA32_4K, 0, 0, pent)) {
        untyped_free_4k(ref);
        ERRX_TRACEPOINT;
        return false;
    }
    int err = seL4_IA32_Page_Map(pent, current_vspace, (uintptr_t) page, seL4_AllRights,
                                 seL4_IA32_Default_VMAttributes);
    bool outside_table;
    if (err == seL4_NoError) {
        outside_table = ref_table(page);
    } else {
        if (err == seL4_FailedLookup) {
            if (!map_table(page)) {
                ERRX_TRACEPOINT;
                return false;
            }
            err = seL4_IA32_Page_Map(pent, current_vspace, (uintptr_t) page, seL4_AllRights,
                                     seL4_IA32_Default_VMAttributes);
            if (err != seL4_NoError) {
                unref_table(page);
                ERRX_RAISE_SEL4(err);
                return false;
            }
            outside_table = false;
        } else {
            ERRX_RAISE_SEL4(err);
            return false;
        }
    }

    cookie->unref_addr = outside_table ? NULL : page;
    cookie->ref = ref;
    return true;
}
