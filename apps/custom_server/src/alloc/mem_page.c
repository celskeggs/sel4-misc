#include "../basic.h"
#include "mem_page.h"
#include "object.h"
#include "../errno.h"
#include "mem_ao.h"

static seL4_IA32_PageTable mem_page_tables[PAGE_TABLE_COUNT];
static uint16_t mem_page_counts[PAGE_TABLE_COUNT];

static inline uint16_t address_to_table_index(void *page_table) {
    uintptr_t page = (uintptr_t) page_table / PAGE_TABLE_SIZE;
    assert(page <= PAGE_TABLE_COUNT);
    return (uint16_t) page;
}

// avoid needing to return memory immediately - 4K is a reasonable amount of extra memory to hold onto
static seL4_IA32_PageTable cached_allocated_table = seL4_CapNull;

static seL4_Error map_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    assert(mem_page_tables[tid] == seL4_CapNull); // otherwise, someone unmapped us without permission
    assert(mem_page_counts[tid] == 0);
    seL4_IA32_PageTable table = cached_allocated_table;
    if (table == seL4_CapNull) {
        table = object_alloc_page_table();
        if (table == seL4_CapNull) {
            return seL4_NotEnoughMemory;
        }
    } else {
        cached_allocated_table = seL4_CapNull;
    }
    int err = seL4_IA32_PageTable_Map(table, seL4_CapInitThreadVSpace, tid * PAGE_TABLE_SIZE,
                                      seL4_IA32_Default_VMAttributes);
    if (err == seL4_NoError) {
        mem_page_tables[tid] = table;
        mem_page_counts[tid] = 1;
    } else {
        cached_allocated_table = table;
    }
    return (seL4_Error) err;
}

static void unmap_table_tid(uint16_t tid) {
    assert(mem_page_tables[tid] != seL4_CapNull);
    assert(mem_page_counts[tid] == 0);
    seL4_IA32_PageTable table = mem_page_tables[tid];
    mem_page_tables[tid] = seL4_CapNull;
    assert(seL4_IA32_PageTable_Unmap(table) == seL4_NoError);
    if (cached_allocated_table == seL4_CapNull) {
        cached_allocated_table = table;
    } else {
        // TODO: object_dealloc_page_table(table);
    }
}

static bool ref_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    if (mem_page_tables[tid] == seL4_CapNull) {
        // must have been allocated by an outside source. don't bother.
        return true;
    } else {
        assert(mem_page_tables[tid] != seL4_CapNull);
        assert(mem_page_counts[tid] <= PAGE_COUNT_PER_TABLE);
        mem_page_counts[tid] += 1;
        return false;
    }
}

static void unref_table(void *page) {
    uint16_t tid = address_to_table_index(page);
    assert(mem_page_tables[tid] != seL4_CapNull);
    assert(mem_page_counts[tid] > 0);
    if (--mem_page_counts[tid] == 0) {
        unmap_table_tid(tid);
    }
}

struct mem_page_free_param {
    seL4_IA32_Page pent;
    void *address;
    bool outside_table;
};

void mem_page_free(void *data) {
    struct mem_page_free_param *param = (struct mem_page_free_param *) data;
    seL4_CNode_Delete(seL4_CapInitThreadVSpace, param->pent, 32);
    if (!param->outside_table) {
        unref_table(param->address);
    }
    // TODO: free param
}

seL4_Error mem_page_map(void *page, DEX dex) {
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
                return err;
            }
            err = (seL4_Error) seL4_IA32_Page_Map(pent, seL4_CapInitThreadVSpace, (uintptr_t) page, seL4_AllRights,
                                     seL4_IA32_Default_VMAttributes);
            if (err != seL4_NoError) {
                unref_table(page);
                return err;
            }
            outside_table = false;
        } else {
            return err;
        }
    }

    struct mem_page_free_param *param = mem_ao_alloc(sizeof(struct mem_page_free_param));
    param->address = page;
    param->pent = pent;
    param->outside_table = outside_table;
    if (!destructor_insert(dex, mem_page_free, param)) {
        return seL4_NotEnoughMemory;
    }
    return seL4_NoError;
}
