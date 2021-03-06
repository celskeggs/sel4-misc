#include <elfloader/elfparser.h>
#include <bedrock/bedrock.h>

#define PHDR_LOAD 1

#define ELF_VERSION 0x01
#define ELF_32 0x01
#define ELF_LITTLE 0x01
#define ELF_SYSTEMV 0x00
#define ELF_EXECUTABLE 0x02
#define ELF_X86 0x03

// 32 bytes minimum for each program header table entry. anything less and we'll reject it for security reasons.
#define MIN_PHDR_SIZE 32

// currently hard-coded as 32-bit. noticeably different code is necessary for 64-bit.
seL4_CompileTimeAssert(sizeof(void *) == sizeof(uint32_t));

// select just the address-within-page bits
#define PAGE_MASK (PAGE_SIZE - 1)
// select just the page-base-address bits
#define PAGE_UNMASK (~PAGE_MASK)

static bool copy_via_remapper(void *source, size_t length, void *virt_target, elfparser_remap_cb remapper, void *cookie,
                              void *page_buffer, uint8_t access_flags) {
    if (length == 0) {
        return true;
    }
    while (length > 0) {
        uint16_t page_offset = (uint16_t) (((uintptr_t) virt_target) & PAGE_MASK);
        void *page_base = (void *) ((uintptr_t) virt_target & PAGE_UNMASK);
        if (!remapper(cookie, page_base, access_flags)) {
            ERRX_TRACEPOINT;
            return false;
        }
        size_t copy_len = PAGE_SIZE - page_offset;
        if (copy_len > length) {
            copy_len = length;
        }
        assert(copy_len > 0);
        memcpy(page_buffer + page_offset, source, copy_len);
        length -= copy_len;
        source += copy_len;
        virt_target += copy_len;
        assert(length == 0 || ((uintptr_t) virt_target & PAGE_MASK) == 0);
    }
    return true;
}

// TODO: this code needs VERY CLOSE SCRUTINY. it is a clear attack surface, and must be protected well.
bool elfparser_load(void *elf, size_t file_size, elfparser_remap_cb remapper, void *cookie, void *page_buffer,
                    uintptr_t *entry_position_out) {
    assert(elf != NULL && remapper != NULL && cookie != NULL && page_buffer != NULL && entry_position_out != NULL);
    uint8_t *head = (uint8_t *) elf;
    uint8_t *end = elf + file_size;
    assert(end > head);
#define H16(n) (*(uint16_t *) (head + n))
#define H32(n) (*(uint32_t *) (head + n))
    if (file_size < 32) {
        ERRX_RAISE_GENERIC(GERR_MALFORMED_DATA);
        return false;
    }
    if (head[0] != 0x7F || head[1] != 'E' || head[2] != 'L' || head[3] != 'F') {
        ERRX_RAISE_GENERIC(GERR_INVALID_MAGIC);
        return false;
    }
    if (head[4] != ELF_32 || head[5] != ELF_LITTLE || head[6] != ELF_VERSION || head[7] != ELF_SYSTEMV) {
        ERRX_RAISE_GENERIC(GERR_UNSUPPORTED_OPTION);
        return false;
    }
    if (H16(16) != ELF_EXECUTABLE || H16(18) != ELF_X86 || H32(20) != ELF_VERSION) {
        ERRX_RAISE_GENERIC(GERR_UNSUPPORTED_OPTION);
        return false;
    }
    *entry_position_out = H32(24);
    uint8_t *table_pointer = elf + H32(28);
    if (table_pointer <= head || table_pointer > end) {
        ERRX_RAISE_GENERIC(GERR_MALFORMED_DATA);
        return false;
    }
    uint16_t entry_size = H16(42);
    uint16_t entry_count = H16(44);
    uint8_t *table_end = table_pointer + entry_size * (uint32_t) entry_count;
    if (table_end <= table_pointer || entry_size < MIN_PHDR_SIZE || table_end > end) {
        // detect overflow conditions; detect overread conditions
        ERRX_RAISE_GENERIC(GERR_MALFORMED_DATA);
        return false;
    }
    for (uint16_t i = 0; i < entry_count; i++) {
        uint32_t *entry_addr = (uint32_t *) ((entry_size * i) + table_pointer);
        assert((uint8_t *) entry_addr >= head && (uint8_t *) entry_addr <= end - MIN_PHDR_SIZE); // avoid overreading
        uint32_t phdr_type = entry_addr[0], file_offset = entry_addr[1], virtual_address = entry_addr[2];
        uint32_t stored_size = entry_addr[4], memory_size = entry_addr[5], flags = entry_addr[6];
        // TODO: should we look at alignment?
        if (phdr_type != PHDR_LOAD) {
            continue; // ignore this entry
        }
        assert(stored_size <= memory_size);
        if (stored_size > 0) {
            uint8_t *read_source = file_offset + elf;
            uint8_t *read_end = read_source + stored_size;
            assert(read_end > read_source);
            assert(read_source >= head && read_end <= end);
            if (!copy_via_remapper(read_source, stored_size, (void *) virtual_address, remapper, cookie, page_buffer,
                                   (uint8_t) (flags & ELF_MEM_FLAGS))) {
                ERRX_TRACEPOINT;
                return false;
            }
        }
        uint32_t alignment_correction = virtual_address & (PAGE_SIZE - 1);
        // make sure that we've loaded enough pages for the rest, as well
        for (uint32_t offset = 0; offset < memory_size + alignment_correction; offset += PAGE_SIZE) {
            if (!remapper(cookie, (void *) virtual_address + offset - alignment_correction,
                          (uint8_t) (flags & ELF_MEM_FLAGS))) {
                ERRX_TRACEPOINT;
                return false;
            }
        }
    }
    return true;
}
