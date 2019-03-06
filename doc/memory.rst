************************
Memory API documentation
************************

0. Overview and constants
#########################

The memory manager in Yggdrasil kernel provides functions for management of memory mappings for both
kernel and userspace functions by utilizing platform MMU, additionally providing features like
physical page allocation and memory space object management. The API is written to be as detached
from the target platform as possible, while still providing full control over virtual memory.

Defined global variables:
   * `mm_space_t mm_kernel` - kernel memory space

Defined constants/flags are:
   Memory mapping/allocation options:
      * `MM_FLG_WR`           - write flag
      * `MM_FLG_US`           - userspace flag
      * `MM_FLG_PS`           - alternate page size flag
      * `MM_FLG_MERGE`        - if used with mapping functions, doesn't fail in case of already existing
        mappings, only adding missing ones
      * `MM_FLG_NOPHYS`       - if used with unmapping functions, doesn't de-allocate physical pages
        associated with virtual ranges
      * `MM_FLG_DMA`          - the mapping is intended for hardware DMA usage

   Address space clone/fork flags:
      * `MM_FLG_CLONE_KERNEL` - copy kernel virtual memory mappings to destination space
      * `MM_FLG_CLONE_USER`   - copy/fork userspace memory mappings/regions
      * `MM_FLG_CLONE_HW`     - copy hardware memory mappings

1. Memory mapping interface
###########################

The section defines a platform-independent interface for target device's MMU and memory
protection schemes.


``int mm_map_range(mm_space_t pd, uintptr_t start, size_t count, uint32_t flags)``
   Changes address space `pd` to create a mapping of contiguous range of `count` pages starting at
   address `start` to some region of physical memory, automatically allocating needed pages. `flags`
   may specify desired write/user/etc. permissions. The function will also differentiate between
   kernel, user and hardware mapping spaces (e.g. this function will disallow mappings in hardware
   space).

   `flags`:
      * `MM_FLG_WR`  - allows write access
      * `MM_FLG_US`  - allows access from userspace applications
      * `MM_FLG_DMA` - tells the function that the range is intended for DMA usage,
        which may differ from regular mappings in some platforms (like x86)
      * `MM_FLG_PS`  - use alternate page size

   Returned values:
      * 0 in case of success
      * Non-zero values otherwise

``int mm_map_range_linear(mm_space_t pd, uintptr_t vstart, uintptr_t pstart, size_t count, uint32_t flags)``
   Creates a mapping of contiguous range [`vstart` .. `vstart` + `count` * `PAGE_SIZE`) to physical
   memory range [`pstart` .. `pstart` + `count` * `PAGE_SIZE`). The size for pages used (`PAGE_SIZE`)
   is determined by `flags`. The default `PAGE_SIZE` is 4KiB unless specified by `MM_FLG_PS` flag.

   `flags`:
      * `MM_FLG_WR`  - allows write access
      * `MM_FLG_US`  - allows access from userspace applications
      * `MM_FLG_DMA` - tells the function that the range is intended for DMA usage,
        which may differ from regular mappings in some platforms (like x86)
      * `MM_FLG_PS`  - use alternate page size

   Returned values:
      * 0 in case of success
      * Non-zero values otherwise

``int mm_map_range_pages(mm_space_t pd, uintptr_t start, uintptr_t *pages, size_t count, uint32_t flags)``
   Creates a mapping of `count` pages for a virtual address range [`start` .. `start` + `count` *
   `PAGE_SIZE`) in address space `pd` to physical addresses in `pages`. The size for pages used
   (`PAGE_SIZE`) is determined by `flags`. The default `PAGE_SIZE` is 4KiB unless specified by
   `MM_FLG_PS` flag.

   `flags`:
      * `MM_FLG_WR`  - allows write access
      * `MM_FLG_US`  - allows access from userspace applications
      * `MM_FLG_DMA` - the physical pages specified are addresses of memory-mapped DMA hardware
      * `MM_FLG_PS`  - use alternate page size

   Returned values:
      * 0 in case of success
      * Non-zero values otherwise

``int mm_umap_range(mm_space_t pd, uintptr_t start, size_t count, uint32_t flags)``
   Unmaps the contiguous range [`start` .. `start` + `count`) from `pd` address space, if the
   mapping is present, optionally freeing the mapped-to physical memory pages.

   `flags`:
      * `MM_FLG_NOPHYS`   - do not free the physical pages the region is mapped to

   Returned values:
      * 0 in case the region was unmapped successfully
      * Non-zero values otherwise

``uintptr_t mm_translate(mm_space_t pd, uintptr_t vaddr, uint32_t *rflags)``
   Translates the virtual address `vaddr` in `pd` memory space into its physical counterpart, if
   possible. Additionally, the function returns the mapping attributes in `rflags` (if the latter is
   a non-null pointer).

   Returned values:
      * A physical address corresponding to `vaddr` in `pd` space
      * `MM_NADDR` in case the mapping for the address does not exist in `pd`

   Values stored in `rflags`:
      * `MM_FLG_WR`  - page has write access
      * `MM_FLG_US`  - page has userspace access
      * `MM_FLG_PS`  - page is not a 4KiB-page


2. Memory transfer interface
############################

The section defines an interface for various transfer options for kernel and userspace interaction.


``int mm_memcpy_kernel_to_user(mm_space_t dst_pd, userspace void *dst, const void *src, size_t count)``
   Performs a copy of memory range [`src` .. `src` + `count`) in kernel memory space into the range
   [`dst` .. `dst` + `count`) in `dst_pd` memory space. Requires that all the pages of destination
   range are mapped in advance to performing the transfer. Will fail if `dst` does not belong to
   userspace virtual range.

   Returned values:
      * 0 in case the transfer succceeds
      * Non-zero values otherwise

``int mm_memcpy_user_to_kernel(mm_space_t src_pd, void *dst, const userspace void *src, size_t count)``
   Performs a copy of memory range [`src` .. `src` + `count`) in `src_pd` memory space into the
   range [`dst` .. `dst` + `count`) in kernel memory space. Requires that all the pages of source
   range are mapped and that the source range belongs to userspace virtual range.

   Returned values:
      * 0 in case the transfer succeeds
      * Non-zero values otherwise

``ssize_t mm_strncpy_user_to_kernel(mm_space_t src_pd, void *dst, const userspace void *src, size_t count)``
   Performs a copy of a `count`-character limited NULL-terminated string from pointer `src` in
   `src_pd` memory space into the `dst` block in kernel memory space. The count of bytes copied does
   not exceed `count`. Requires that all the pages of source range before NULL or `count` bytes are
   mapped in advance to performing the transfer. Will fail if `src` does not belong to userspace
   virtual range.

   Returned values:
      * Number of bytes copied to `dst` (<= `count`) in case of success
      * -1 otherwise


3. Physical memory interface
############################

The section defines an interface for kernel to interact with physical memory pages.

``uintptr_t mm_alloc_physical_page(uint32_t flags)``
   Allocates a single physical page. If `flags` has `MM_FLG_PS` set, alternate page size will
   be used. The default size for the pages allocated is 4KiB.

   `flags`:
      * `MM_FLG_PS`  - use alternate page size (4MiB for x86)

   Returned values:
      * Physical address of the page allocated on success
      * `MM_NADDR` otherwise

``void mm_free_physical_page(uintptr_t page, uint32_t flags)``
   Frees a single physical page. If `flags` has `MM_FLG_PS` set, the `page` is considered to be an
   physical address to a page of alternate size, the address is considered to be a 4KiB-page physical
   address otherwise.

   `flags`:
      * `MM_FLG_PS`  - the page freed is of alternate size (4MiB for x86)


4. Address space interface
##########################

The section defines an interface for kernel to allocate, free and perform various operations on
address spaces and paging structures.

``mm_space_t mm_create_space(uintptr_t *phys)``
   Allocates an address space with underlying paging structures. Physical address of the paging
   structure allocated is stored in `phys` (if the latter is a non-NULL pointer)

   Returned values:
      * An address space object with no mappings present.
      * `NULL` if allocation fails

``void mm_destroy_space(mm_space_t pd)``
   Frees the resources used by `pd` address space and makes them available for next allocations.
   Additionally, the function either decrements reference counts for paging structures referred from
   `pd` or destroys them.

``void mm_space_clone(mm_space_t dst, const mm_space_t src, uint32_t flags)``
   Clones entries in address space `src` into the destination address space `dst`.

   `flags`:
      * `MM_FLG_CLONE_KERNEL` - clone kernel virtual address space (if present in `src`)
      * `MM_FLG_CLONE_USER`   - clone user virtual address space (if present in `src`)
      * `MM_FLG_CLONE_HW`     - clone hardware mappings (if present in `src`)

``int mm_space_fork(mm_space_t dst, const mm_space_t src, uint32_t flags)``
   Performs a "fork" of `src` address space, storing resulting mappings in `dst`. The kernel and
   hardware virtual spaces are copied verbatim into `dst` spaces if present in `src`. The userspace
   mappings are cloned either using Copy-On-Write (if implemented for target platform) or physical
   copy of the pages in `src`.

   `flags`:
      * `MM_FLG_CLONE_KERNEL` - clone kernel virtual pages (if present in `src`)
      * `MM_FLG_CLONE_USER`   - clone user virtual pages (if present in `src`)
      * `MM_FLG_CLONE_HW`     - clone hardware mappings (is present in `src`)

   Returned values:
      * 0 in case of success
      * Non-zero values otherwise.

``void mm_set(mm_space_t pd)``
   Switches the current address space to `pd`


5. Debugging interface
######################

The section defines an interface for memory space debugging.

``void mm_dump_stats(int level)``
   Prints detailed stats of the memory manager. `level` specifies debug output level.

   These may include:
      * Number of address spaces present
      * Stats of address space allocator:
         * How many spaces were allocated
         * How many spaces were freed
      * Physical memory stats
      * Virtual memory stats
      * Kernel memory mappings

``void mm_dump_map(int level, mm_space_t pd)``
   Prints details of memory mappings in `pd` address space. `level` specifies debug output level.
