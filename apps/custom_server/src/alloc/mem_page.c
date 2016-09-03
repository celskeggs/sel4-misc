#include "../basic.h"
#include "mem_page.h"
#include "object.h"
#include "untyped.h"

static untyped_ref mem_page_tables[PAGE_TABLE_COUNT];
static uint16_t mem_page_counts[PAGE_TABLE_COUNT];

static inline uint16_t address_to_table_index(void *page_table) {
    uintptr_t page = (uintptr_t) page_table / PAGE_TABLE_SIZE;
    assert(page <= PAGE_TABLE_COUNT);
    return (uint16_t) page;
}

static seL4_Error map_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    assert(mem_page_tables[tid] == UNTYPED_NONE); // otherwise, someone unmapped us without permission
    assert(mem_page_counts[tid] == 0);
    untyped_ref ref = untyped_alloc(seL4_PageTableBits);
    if (ref == UNTYPED_NONE) {
        DEBUG("fail");
        return seL4_NotEnoughMemory;
    }
    seL4_IA32_PageTable table = untyped_retype(ref, seL4_IA32_PageTableObject, 0, 0);
    if (table == seL4_CapNull) {
        untyped_dealloc(seL4_PageTableBits, ref);
        DEBUG("fail");
        return seL4_NotEnoughMemory;
    }
    int err = seL4_IA32_PageTable_Map(table, seL4_CapInitThreadVSpace, tid * PAGE_TABLE_SIZE,
                                      seL4_IA32_Default_VMAttributes);
    if (err != seL4_NoError) {
        untyped_detype(ref);
        untyped_dealloc(seL4_PageTableBits, ref);
        DEBUG("fail");
        return (seL4_Error) err;
    }
    mem_page_tables[tid] = ref;
    mem_page_counts[tid] = 1;
    return seL4_NoError;
}

static void unmap_table_tid(uint16_t tid) {
    assert(mem_page_tables[tid] != UNTYPED_NONE);
    assert(mem_page_counts[tid] == 0);
    untyped_ref table = mem_page_tables[tid];
    mem_page_tables[tid] = UNTYPED_NONE;
    untyped_detype(table);
    untyped_dealloc(seL4_PageTableBits, table);
}

static bool ref_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    if (mem_page_tables[tid] == UNTYPED_NONE) {
        // must have been allocated by an outside source. don't bother.
        return true;
    } else {
        assert(mem_page_tables[tid] != UNTYPED_NONE);
        assert(mem_page_counts[tid] <= PAGE_COUNT_PER_TABLE);
        mem_page_counts[tid] += 1;
        return false;
    }
}

static void unref_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    assert(mem_page_tables[tid] != UNTYPED_NONE);
    assert(mem_page_counts[tid] > 0);
    if (--mem_page_counts[tid] == 0) {
        unmap_table_tid(tid);
    }
}

void mem_page_free(struct mem_page_cookie *data) {
    seL4_CNode_Delete(seL4_CapInitThreadVSpace, data->page, 32);
    if (data->unref_addr != NULL) {
        unref_table(data->unref_addr);
    }
}

seL4_Error mem_page_map(void *page, struct mem_page_cookie *cookie) {
    seL4_IA32_Page pent = object_alloc_page(); // TODO: register object with DEX
    if (pent == seL4_CapNull) {
        DEBUG("fail");
        return seL4_NotEnoughMemory;
    }
    seL4_Error err = (seL4_Error) seL4_IA32_Page_Map(pent, seL4_CapInitThreadVSpace, (uintptr_t) page, seL4_AllRights,
                                                     seL4_IA32_Default_VMAttributes);
    bool outside_table;
    if (err == seL4_NoError) {
        outside_table = ref_table(page);
    } else {
        if (err == seL4_FailedLookup) {
            err = map_table(page);
            if (err != seL4_NoError) {
                DEBUG("fail");
                return err;
            }
            err = (seL4_Error) seL4_IA32_Page_Map(pent, seL4_CapInitThreadVSpace, (uintptr_t) page, seL4_AllRights,
                                                  seL4_IA32_Default_VMAttributes);
            if (err != seL4_NoError) {
                DEBUG("fail");
                unref_table(page);
                return err;
            }
            outside_table = false;
        } else {
            return err;
        }
    }

    cookie->unref_addr = outside_table ? NULL : page;
    cookie->page = pent;
    return seL4_NoError;
}
