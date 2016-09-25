#include <elfloader/elfloader.h>
#include <elfloader/elfparser.h>
#include <resource/mem_vspace.h>
#include <resource/mem_fx.h>
#include <resource/cslot.h>
#include <resource/mem_page.h>

#define PAGE_ACCESS_UNINIT 0xFF

struct pd_param {
    bool is_cptr_active;
    seL4_CPtr active_cptr;
    void *target_addr;
    struct mem_page_cookie cur_cookie;
    struct pagedir *pagedir;
};

static struct pagetable *get_pagetable(struct pagedir *pd, void *virtual_address) {
    uintptr_t page_table_id = ((uintptr_t) virtual_address) >> seL4_PageTableBits;
    assert(page_table_id < PAGE_TABLE_COUNT);
    struct pagetable *pt = pd->pts[page_table_id];
    if (pt != NULL) {
        return pt;
    }
    pt = (struct pagetable *) mem_fx_alloc(sizeof(struct pagetable));
    if (pt == NULL) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    pt->pt = untyped_allocate_4k();
    if (pt->pt == NULL) {
        mem_fx_free(pt, sizeof(struct pagetable));
        ERRX_TRACEPOINT;
        return NULL;
    }
    seL4_IA32_PageTable table = untyped_auxptr_4k(pt->pt);
    if (!cslot_retype(untyped_ptr_4k(pt->pt), seL4_IA32_PageTableObject, 0, 0, table, 1)) {
        untyped_free_4k(pt->pt);
        mem_fx_free(pt, sizeof(struct pagetable));
        ERRX_TRACEPOINT;
        return NULL;
    }
    int err = seL4_IA32_PageTable_Map(table, pd->pd, page_table_id * PAGE_TABLE_SIZE,
                                      seL4_IA32_Default_VMAttributes);
    if (err != seL4_NoError) {
        untyped_free_4k(pt->pt);
        mem_fx_free(pt, sizeof(struct pagetable));
        ERRX_RAISE_SEL4(err);
        return NULL;
    }
    for (uint32_t i = 0; i < PAGE_COUNT_PER_TABLE; i++) {
        pt->pages[i] = NULL;
        pt->page_accesses[i] = PAGE_ACCESS_UNINIT;
    }
    pd->pts[page_table_id] = pt;
    return pt;
}

seL4_IA32_Page elfloader_get_page(struct pagedir *pd, void *virtual_address, uint8_t access_flags, bool exclusive) {
    struct pagetable *pt = get_pagetable(pd, virtual_address);
    if (pt == NULL) {
        ERRX_TRACEPOINT;
        return seL4_CapNull;
    }
    uint32_t page_offset = (((uintptr_t) virtual_address) >> seL4_PageBits) & (PAGE_COUNT_PER_TABLE - 1);
    assert(page_offset < PAGE_COUNT_PER_TABLE);
    if (pt->pages[page_offset] == NULL) {
        // NEED TO ALLOCATE PAGE
        untyped_4k_ref ut = untyped_allocate_4k();
        if (ut == NULL) {
            ERRX_TRACEPOINT;
            return seL4_CapNull;
        }
        if (!cslot_retype(untyped_ptr_4k(ut), seL4_IA32_4K, 0, 0, untyped_auxptr_4k(ut), 1)) {
            untyped_free_4k(ut);
            ERRX_TRACEPOINT;
            return seL4_CapNull;
        }
        seL4_CapRights rights = (seL4_CapRights) (((access_flags & ELF_MEM_WRITABLE) ? seL4_CanWrite : 0) |
                                                  ((access_flags & (ELF_MEM_READABLE | ELF_MEM_EXECUTABLE))
                                                   ? seL4_CanRead : 0));
        int err = seL4_IA32_Page_Map(untyped_auxptr_4k(ut), pd->pd, (uintptr_t) virtual_address, rights,
                                     seL4_IA32_Default_VMAttributes);
        if (err != seL4_NoError) {
            untyped_free_4k(ut);
            ERRX_RAISE_SEL4(err);
            return seL4_CapNull;
        }
        pt->pages[page_offset] = ut;
        pt->page_accesses[page_offset] = access_flags;
    } else {
        // EXISTING PAGE
        if (exclusive) {
            ERRX_RAISE_GENERIC(GERR_UNSATISFIED_CONSTRAINT);
            return seL4_CapNull;
        } else if (pt->page_accesses[page_offset] != access_flags) {
            // won't let loader load two different access_flags
            ERRX_RAISE_GENERIC(GERR_ACCESS_VIOLATION);
            return seL4_CapNull;
        }
    }
    return untyped_auxptr_4k(pt->pages[page_offset]);
}

static bool remapper(void *cookie, void *virtual_address, uint8_t access_flags) {
    struct pd_param *param = (struct pd_param *) cookie;
    assert(param != NULL);
    seL4_IA32_Page page = elfloader_get_page(param->pagedir, virtual_address, access_flags, false);
    if (page == seL4_CapNull) {
        ERRX_TRACEPOINT;
        return false;
    }
    seL4_IA32_Page alt = param->active_cptr;
    if (param->is_cptr_active) {
        mem_page_free(&param->cur_cookie);
    }
    assert(cslot_delete(alt) == seL4_NoError);
    assert(cslot_copy(page, alt) == seL4_NoError);
    if (!mem_page_shared_map(param->target_addr, alt, &param->cur_cookie)) {
        param->is_cptr_active = false;
        ERRX_TRACEPOINT;
        return false;
    } else {
        param->is_cptr_active = true;
        return true;
    }
}

struct pagedir *elfloader_load(void *elf, size_t file_size, seL4_IA32_PageDirectory page_dir) {
    struct mem_vspace zone;
    size_t actual_size = mem_vspace_alloc_slice(&zone, PAGE_SIZE * 2);
    if (actual_size == 0) {
        ERRX_TRACEPOINT;
        return NULL;
    }
    assert(actual_size >= PAGE_SIZE);
    void *buffer = mem_vspace_ptr(&zone);
    assert(buffer != NULL);
    struct pagedir *pdir = mem_fx_alloc(sizeof(struct pagedir));
    if (pdir == NULL) {
        mem_vspace_dealloc_slice(&zone);
        ERRX_TRACEPOINT;
        return NULL;
    }
    seL4_CPtr spare_cptr = cslot_alloc();
    if (spare_cptr == seL4_CapNull) {
        mem_fx_free(pdir, sizeof(struct pagedir));
        mem_vspace_dealloc_slice(&zone);
        return NULL;
    }
    pdir->pd = page_dir;
    struct pd_param param = {.pagedir = pdir, .target_addr = buffer, .active_cptr = spare_cptr, .is_cptr_active = false};
    for (uint32_t i = 0; i < PAGE_TABLE_COUNT; i++) {
        pdir->pts[i] = NULL;
    }
    bool success = elfparser_load(elf, file_size, remapper, &param, buffer, &pdir->entry_position);
    if (param.is_cptr_active) {
        mem_page_free(&param.cur_cookie);
    }
    cslot_delete(spare_cptr);
    cslot_free(spare_cptr);
    mem_vspace_dealloc_slice(&zone);
    if (!success) {
        elfloader_unload(param.pagedir);
        ERRX_TRACEPOINT;
        return NULL;
    }
    return param.pagedir;
}

void elfloader_unload(struct pagedir *pd) {
    for (uint32_t i = 0; i < PAGE_TABLE_COUNT; i++) {
        struct pagetable *pt = pd->pts[i];
        if (pt != NULL) {
            for (uint32_t j = 0; j < PAGE_COUNT_PER_TABLE; j++) {
                if (pt->pages[j] != NULL) {
                    untyped_free_4k(pt->pages[j]);
                    pt->pages[j] = NULL;
                }
            }
            untyped_free_4k(pt->pt);
        }
        mem_fx_free(pt, sizeof(struct pagetable));
        pd->pts[i] = NULL;
    }
    mem_fx_free(pd, sizeof(struct pagedir));
}
