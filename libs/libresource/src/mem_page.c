#include <resource/mem_page.h>

extern seL4_CPtr current_vspace;

static object_token mem_page_tables[PAGE_TABLE_COUNT];
static uint16_t mem_page_counts[PAGE_TABLE_COUNT];

static inline uint16_t address_to_table_index(void *page_table) {
    uintptr_t page = (uintptr_t) page_table / PAGE_TABLE_SIZE;
    assert(page <= PAGE_TABLE_COUNT);
    return (uint16_t) page;
}

static bool map_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    assert(mem_page_tables[tid] == NULL); // otherwise, someone unmapped us without permission
    assert(mem_page_counts[tid] == 0);
    seL4_CompileTimeAssert(seL4_PageTableBits == BITS_4KIB);
    object_token ref = object_alloc(seL4_IA32_PageTableObject);
    if (ref == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    int err = seL4_IA32_PageTable_Map(object_cap(ref), current_vspace, tid * PAGE_TABLE_SIZE,
                                      seL4_IA32_Default_VMAttributes);
    if (err != seL4_NoError) {
        object_free_token(ref);
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
    object_token table = mem_page_tables[tid];
    mem_page_tables[tid] = NULL;
    object_free_token(table);
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

bool mem_page_shared_map(void *page, seL4_IA32_Page pent, struct mem_page_cookie *cookie) {
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
    cookie->ref = NULL;
    cookie->mapped = pent;
    return true;
}

void mem_page_free(struct mem_page_cookie *cookie) {
    assert(cookie != NULL);
    if (cookie->ref != NULL) {
        object_free_token(cookie->ref);
        cookie->ref = NULL;
    }
    if (cookie->mapped != seL4_CapNull) {
        seL4_IA32_Page_Unmap(cookie->mapped);
        cookie->mapped = seL4_CapNull;
    }
    if (cookie->unref_addr != NULL) {
        unref_table(cookie->unref_addr);
        cookie->unref_addr = NULL;
    }
}

bool mem_page_valid(struct mem_page_cookie *cookie) {
    return cookie->ref != NULL || cookie->mapped != seL4_CapNull;
}

bool mem_page_map(void *page, struct mem_page_cookie *cookie) {
    seL4_CompileTimeAssert(seL4_PageBits == BITS_4KIB);
    assert(!mem_page_valid(cookie));
    object_token ref = object_alloc(seL4_IA32_4K);
    if (ref == NULL) {
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_IA32_Page pent = object_cap(ref);
    int err = seL4_IA32_Page_Map(pent, current_vspace, (uintptr_t) page, seL4_AllRights,
                                 seL4_IA32_Default_VMAttributes);
    bool outside_table;
    if (err == seL4_NoError) {
        outside_table = ref_table(page);
    } else {
        if (err == seL4_FailedLookup) {
            if (!map_table(page)) {
                object_free_token(ref);
                ERRX_TRACEPOINT;
                return false;
            }
            err = seL4_IA32_Page_Map(pent, current_vspace, (uintptr_t) page, seL4_AllRights,
                                     seL4_IA32_Default_VMAttributes);
            if (err != seL4_NoError) {
                object_free_token(ref);
                unref_table(page);
                ERRX_RAISE_SEL4(err);
                return false;
            }
            outside_table = false;
        } else {
            object_free_token(ref);
            ERRX_RAISE_SEL4(err);
            return false;
        }
    }

    cookie->unref_addr = outside_table ? NULL : page;
    cookie->ref = ref;
    cookie->mapped = seL4_CapNull;
    return true;
}
