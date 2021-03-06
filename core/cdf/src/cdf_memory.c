/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC:  cdf_memory
 *
 * Connectivity driver framework (CDF) memory management APIs
 */

/* Include Files */
#include "cdf_memory.h"
#include "cdf_nbuf.h"
#include "cdf_trace.h"
#include "cdf_lock.h"

#if defined(CONFIG_CNSS)
#include <net/cnss.h>
#endif

#ifdef CONFIG_WCNSS_MEM_PRE_ALLOC
#include <net/cnss_prealloc.h>
#endif

#ifdef MEMORY_DEBUG
#include <cdf_list.h>

cdf_list_t cdf_mem_list;
cdf_spinlock_t cdf_mem_list_lock;

static uint8_t WLAN_MEM_HEADER[] = { 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
					0x67, 0x68 };
static uint8_t WLAN_MEM_TAIL[] = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85,
					0x86, 0x87 };

struct s_cdf_mem_struct {
	cdf_list_node_t pNode;
	char *fileName;
	unsigned int lineNum;
	unsigned int size;
	uint8_t header[8];
};
#endif

/* Preprocessor Definitions and Constants */

/* Type Declarations */

/* Data definitions */

/* External Function implementation */
#ifdef MEMORY_DEBUG

/**
 * cdf_mem_init() - initialize cdf memory debug functionality
 *
 * Return: none
 */
void cdf_mem_init(void)
{
	/* Initalizing the list with maximum size of 60000 */
	cdf_list_init(&cdf_mem_list, 60000);
	cdf_spinlock_init(&cdf_mem_list_lock);
	cdf_net_buf_debug_init();
	return;
}

/**
 * cdf_mem_clean() - display memory leak debug info and free leaked pointers
 *
 * Return: none
 */
void cdf_mem_clean(void)
{
	uint32_t listSize;
	cdf_list_size(&cdf_mem_list, &listSize);

	cdf_net_buf_debug_clean();

	if (listSize) {
		cdf_list_node_t *pNode;
		CDF_STATUS cdf_status;

		struct s_cdf_mem_struct *memStruct;
		char *prev_mleak_file = "";
		unsigned int prev_mleak_lineNum = 0;
		unsigned int prev_mleak_sz = 0;
		unsigned int mleak_cnt = 0;

		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: List is not Empty. listSize %d ",
			  __func__, (int)listSize);

		do {
			cdf_spin_lock(&cdf_mem_list_lock);
			cdf_status =
				cdf_list_remove_front(&cdf_mem_list, &pNode);
			cdf_spin_unlock(&cdf_mem_list_lock);
			if (CDF_STATUS_SUCCESS == cdf_status) {
				memStruct = (struct s_cdf_mem_struct *)pNode;
				/* Take care to log only once multiple memory
				   leaks from the same place */
				if (strcmp(prev_mleak_file, memStruct->fileName)
				    || (prev_mleak_lineNum !=
					memStruct->lineNum)
				    || (prev_mleak_sz != memStruct->size)) {
					if (mleak_cnt != 0) {
						CDF_TRACE(CDF_MODULE_ID_CDF,
							  CDF_TRACE_LEVEL_FATAL,
							  "%d Time Memory Leak@ File %s, @Line %d, size %d",
							  mleak_cnt,
							  prev_mleak_file,
							  prev_mleak_lineNum,
							  prev_mleak_sz);
					}
					prev_mleak_file = memStruct->fileName;
					prev_mleak_lineNum = memStruct->lineNum;
					prev_mleak_sz = memStruct->size;
					mleak_cnt = 0;
				}
				mleak_cnt++;
				kfree((void *)memStruct);
			}
		} while (cdf_status == CDF_STATUS_SUCCESS);

		/* Print last memory leak from the module */
		if (mleak_cnt) {
			CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
				  "%d Time memory Leak@ File %s, @Line %d, size %d",
				  mleak_cnt, prev_mleak_file,
				  prev_mleak_lineNum, prev_mleak_sz);
		}
#ifdef CONFIG_HALT_KMEMLEAK
		BUG_ON(0);
#endif
	}
}

/**
 * cdf_mem_exit() - exit cdf memory debug functionality
 *
 * Return: none
 */
void cdf_mem_exit(void)
{
	cdf_net_buf_debug_exit();
	cdf_mem_clean();
	cdf_list_destroy(&cdf_mem_list);
}

/**
 * cdf_mem_malloc_debug() - debug version of CDF memory allocation API
 * @size: Number of bytes of memory to allocate.
 * @fileName: File name from which memory allocation is called
 * @lineNum: Line number from which memory allocation is called
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory and ad it in cdf tracking list to check against memory leaks and
 * corruptions
 *
 *
 * Return:
 *      Upon successful allocate, returns a non-NULL pointer to the allocated
 *      memory.  If this function is unable to allocate the amount of memory
 *      specified (for any reason) it returns %NULL.
 *
 */
void *cdf_mem_malloc_debug(size_t size, char *fileName, uint32_t lineNum)
{
	struct s_cdf_mem_struct *memStruct;
	void *memPtr = NULL;
	uint32_t new_size;
	int flags = GFP_KERNEL;

	if (size > (1024 * 1024) || size == 0) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: called with invalid arg; passed in %zu !!!",
			  __func__, size);
		return NULL;
	}

#if defined(CONFIG_CNSS) && defined(CONFIG_WCNSS_MEM_PRE_ALLOC)
	if (size > WCNSS_PRE_ALLOC_GET_THRESHOLD) {
		void *pmem;
		pmem = wcnss_prealloc_get(size);
		if (NULL != pmem) {
			memset(pmem, 0, size);
			return pmem;
		}
	}
#endif

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	new_size = size + sizeof(struct s_cdf_mem_struct) + 8;

	memStruct = (struct s_cdf_mem_struct *)kzalloc(new_size, flags);

	if (memStruct != NULL) {
		CDF_STATUS cdf_status;

		memStruct->fileName = fileName;
		memStruct->lineNum = lineNum;
		memStruct->size = size;

		cdf_mem_copy(&memStruct->header[0],
			     &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER));

		cdf_mem_copy((uint8_t *) (memStruct + 1) + size,
			     &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL));

		cdf_spin_lock_irqsave(&cdf_mem_list_lock);
		cdf_status = cdf_list_insert_front(&cdf_mem_list,
						   &memStruct->pNode);
		cdf_spin_unlock_irqrestore(&cdf_mem_list_lock);
		if (CDF_STATUS_SUCCESS != cdf_status) {
			CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				  "%s: Unable to insert node into List cdf_status %d",
				  __func__, cdf_status);
		}

		memPtr = (void *)(memStruct + 1);
	}
	return memPtr;
}

/**
 *  cdf_mem_free() - debug version of CDF memory free API
 *  @ptr: Pointer to the starting address of the memory to be free'd.
 *
 *  This function will free the memory pointed to by 'ptr'. It also checks
 *  is memory is corrupted or getting double freed and panic.
 *
 *  Return:
 *       Nothing
 */
void cdf_mem_free(void *ptr)
{
	if (ptr != NULL) {
		CDF_STATUS cdf_status;
		struct s_cdf_mem_struct *memStruct =
			((struct s_cdf_mem_struct *)ptr) - 1;

#if defined(CONFIG_CNSS) && defined(CONFIG_WCNSS_MEM_PRE_ALLOC)
		if (wcnss_prealloc_put(ptr))
			return;
#endif

		cdf_spin_lock_irqsave(&cdf_mem_list_lock);
		cdf_status =
			cdf_list_remove_node(&cdf_mem_list, &memStruct->pNode);
		cdf_spin_unlock_irqrestore(&cdf_mem_list_lock);

		if (CDF_STATUS_SUCCESS == cdf_status) {
			if (0 == cdf_mem_compare(memStruct->header,
						 &WLAN_MEM_HEADER[0],
						 sizeof(WLAN_MEM_HEADER))) {
				CDF_TRACE(CDF_MODULE_ID_CDF,
					  CDF_TRACE_LEVEL_FATAL,
					  "Memory Header is corrupted. MemInfo: Filename %s, LineNum %d",
					  memStruct->fileName,
					  (int)memStruct->lineNum);
				CDF_BUG(0);
			}
			if (0 ==
			    cdf_mem_compare((uint8_t *) ptr + memStruct->size,
					    &WLAN_MEM_TAIL[0],
					    sizeof(WLAN_MEM_TAIL))) {
				CDF_TRACE(CDF_MODULE_ID_CDF,
					  CDF_TRACE_LEVEL_FATAL,
					  "Memory Trailer is corrupted. MemInfo: Filename %s, LineNum %d",
					  memStruct->fileName,
					  (int)memStruct->lineNum);
				CDF_BUG(0);
			}
			kfree((void *)memStruct);
		} else {
			CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
				  "%s: Unallocated memory (double free?)",
				  __func__);
			CDF_BUG(0);
		}
	}
}
#else
/**
 * cdf_mem_malloc() - allocation CDF memory
 * @size: Number of bytes of memory to allocate.
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory.
 *
 *
 * Return:
 *	Upon successful allocate, returns a non-NULL pointer to the allocated
 *	memory.  If this function is unable to allocate the amount of memory
 *	specified (for any reason) it returns %NULL.
 *
 */
void *cdf_mem_malloc(size_t size)
{
	int flags = GFP_KERNEL;
#ifdef CONFIG_WCNSS_MEM_PRE_ALLOC
	void *pmem;
#endif
	if (size > (1024 * 1024) || size == 0) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: called with invalid arg; passed in %zu !!",
			  __func__, size);
		return NULL;
	}

#if defined(CONFIG_CNSS) && defined(CONFIG_WCNSS_MEM_PRE_ALLOC)
	if (size > WCNSS_PRE_ALLOC_GET_THRESHOLD) {
		pmem = wcnss_prealloc_get(size);
		if (NULL != pmem) {
			memset(pmem, 0, size);
			return pmem;
		}
	}
#endif

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	return kzalloc(size, flags);
}

/**
 * cdf_mem_free() - free CDF memory
 * @ptr: Pointer to the starting address of the memory to be free'd.
 *
 * This function will free the memory pointed to by 'ptr'.
 *
 * Return:
 *	Nothing
 *
 */
void cdf_mem_free(void *ptr)
{
	if (ptr == NULL)
		return;

#if defined(CONFIG_CNSS) && defined(CONFIG_WCNSS_MEM_PRE_ALLOC)
	if (wcnss_prealloc_put(ptr))
		return;
#endif

	kfree(ptr);
}
#endif

/**
 * cdf_mem_multi_pages_alloc() - allocate large size of kernel memory
 * @osdev:		OS device handle pointer
 * @pages:		Multi page information storage
 * @element_size:	Each element size
 * @element_num:	Total number of elements should be allocated
 * @memctxt:		Memory context
 * @cacheable:		Coherent memory or cacheable memory
 *
 * This function will allocate large size of memory over multiple pages.
 * Large size of contiguous memory allocation will fail frequentely, then
 * instead of allocate large memory by one shot, allocate through multiple, non
 * contiguous memory and combine pages when actual usage
 *
 * Return: None
 */
void cdf_mem_multi_pages_alloc(cdf_device_t osdev,
				struct cdf_mem_multi_page_t *pages,
				size_t element_size,
				uint16_t element_num,
				cdf_dma_context_t memctxt,
				bool cacheable)
{
	uint16_t page_idx;
	struct cdf_mem_dma_page_t *dma_pages;
	void **cacheable_pages = NULL;
	uint16_t i;

	pages->num_element_per_page = PAGE_SIZE / element_size;
	if (!pages->num_element_per_page) {
		cdf_print("Invalid page %d or element size %d",
			(int)PAGE_SIZE, (int)element_size);
		goto out_fail;
	}

	pages->num_pages = element_num / pages->num_element_per_page;
	if (element_num % pages->num_element_per_page)
		pages->num_pages++;

	if (cacheable) {
		/* Pages information storage */
		pages->cacheable_pages = cdf_mem_malloc(
			pages->num_pages * sizeof(pages->cacheable_pages));
		if (!pages->cacheable_pages) {
			cdf_print("Cacheable page storage alloc fail");
			goto out_fail;
		}

		cacheable_pages = pages->cacheable_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			cacheable_pages[page_idx] = cdf_mem_malloc(PAGE_SIZE);
			if (!cacheable_pages[page_idx]) {
				cdf_print("cacheable page alloc fail, pi %d",
					page_idx);
				goto page_alloc_fail;
			}
		}
		pages->dma_pages = NULL;
	} else {
		pages->dma_pages = cdf_mem_malloc(
			pages->num_pages * sizeof(struct cdf_mem_dma_page_t));
		if (!pages->dma_pages) {
			cdf_print("dmaable page storage alloc fail");
			goto out_fail;
		}

		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			dma_pages->page_v_addr_start =
				cdf_os_mem_alloc_consistent(osdev, PAGE_SIZE,
					&dma_pages->page_p_addr, memctxt);
			if (!dma_pages->page_v_addr_start) {
				cdf_print("dmaable page alloc fail pi %d",
					page_idx);
				goto page_alloc_fail;
			}
			dma_pages->page_v_addr_end =
				dma_pages->page_v_addr_start + PAGE_SIZE;
			dma_pages++;
		}
		pages->cacheable_pages = NULL;
	}
	return;

page_alloc_fail:
	if (cacheable) {
		for (i = 0; i < page_idx; i++)
			cdf_mem_free(pages->cacheable_pages[i]);
		cdf_mem_free(pages->cacheable_pages);
	} else {
		dma_pages = pages->dma_pages;
		for (i = 0; i < page_idx; i++) {
			cdf_os_mem_free_consistent(osdev, PAGE_SIZE,
				dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt);
			dma_pages++;
		}
		cdf_mem_free(pages->dma_pages);
	}

out_fail:
	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
	return;
}

/**
 * cdf_mem_multi_pages_free() - free large size of kernel memory
 * @osdev:	OS device handle pointer
 * @pages:	Multi page information storage
 * @memctxt:	Memory context
 * @cacheable:	Coherent memory or cacheable memory
 *
 * This function will free large size of memory over multiple pages.
 *
 * Return: None
 */
void cdf_mem_multi_pages_free(cdf_device_t osdev,
				struct cdf_mem_multi_page_t *pages,
				cdf_dma_context_t memctxt,
				bool cacheable)
{
	unsigned int page_idx;
	struct cdf_mem_dma_page_t *dma_pages;

	if (cacheable) {
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++)
			cdf_mem_free(pages->cacheable_pages[page_idx]);
		cdf_mem_free(pages->cacheable_pages);
	} else {
		dma_pages = pages->dma_pages;
		for (page_idx = 0; page_idx < pages->num_pages; page_idx++) {
			cdf_os_mem_free_consistent(osdev, PAGE_SIZE,
				dma_pages->page_v_addr_start,
				dma_pages->page_p_addr, memctxt);
			dma_pages++;
		}
		cdf_mem_free(pages->dma_pages);
	}

	pages->cacheable_pages = NULL;
	pages->dma_pages = NULL;
	pages->num_pages = 0;
	return;
}


/**
 * cdf_mem_set() - set (fill) memory with a specified byte value.
 * @pMemory:    Pointer to memory that will be set
 * @numBytes:   Number of bytes to be set
 * @value:      Byte set in memory
 *
 * Return:
 *    Nothing
 *
 */
void cdf_mem_set(void *ptr, uint32_t numBytes, uint32_t value)
{
	if (ptr == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter ptr", __func__);
		return;
	}
	memset(ptr, value, numBytes);
}

/**
 * cdf_mem_zero() - zero out memory
 * @pMemory:    pointer to memory that will be set to zero
 * @numBytes:   number of bytes zero
 * @value:      byte set in memory
 *
 *  This function sets the memory location to all zeros, essentially clearing
 *  the memory.
 *
 * Return:
 *      Nothing
 *
 */
void cdf_mem_zero(void *ptr, uint32_t numBytes)
{
	if (0 == numBytes) {
		/* special case where ptr can be NULL */
		return;
	}

	if (ptr == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter ptr", __func__);
		return;
	}
	memset(ptr, 0, numBytes);
}

/**
 * cdf_mem_copy() - copy memory
 * @pDst:       Pointer to destination memory location (to copy to)
 * @pSrc:       Pointer to source memory location (to copy from)
 * @numBytes:   Number of bytes to copy.
 *
 * Copy host memory from one location to another, similar to memcpy in
 * standard C.  Note this function does not specifically handle overlapping
 * source and destination memory locations.  Calling this function with
 * overlapping source and destination memory locations will result in
 * unpredictable results.  Use cdf_mem_move() if the memory locations
 * for the source and destination are overlapping (or could be overlapping!)
 *
 * Return:
 *    Nothing
 *
 */
void cdf_mem_copy(void *pDst, const void *pSrc, uint32_t numBytes)
{
	if (0 == numBytes) {
		/* special case where pDst or pSrc can be NULL */
		return;
	}

	if ((pDst == NULL) || (pSrc == NULL)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter, source:%p destination:%p",
			  __func__, pSrc, pDst);
		CDF_ASSERT(0);
		return;
	}
	memcpy(pDst, pSrc, numBytes);
}

/**
 * cdf_mem_move() - move memory
 * @pDst:       pointer to destination memory location (to move to)
 * @pSrc:       pointer to source memory location (to move from)
 * @numBytes:   number of bytes to move.
 *
 * Move host memory from one location to another, similar to memmove in
 * standard C.  Note this function *does* handle overlapping
 * source and destination memory locations.

 * Return:
 *      Nothing
 */
void cdf_mem_move(void *pDst, const void *pSrc, uint32_t numBytes)
{
	if (0 == numBytes) {
		/* special case where pDst or pSrc can be NULL */
		return;
	}

	if ((pDst == NULL) || (pSrc == NULL)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter, source:%p destination:%p",
			  __func__, pSrc, pDst);
		CDF_ASSERT(0);
		return;
	}
	memmove(pDst, pSrc, numBytes);
}

/**
 * cdf_mem_compare() - memory compare
 * @pMemory1:   pointer to one location in memory to compare.
 * @pMemory2:   pointer to second location in memory to compare.
 * @numBytes:   the number of bytes to compare.
 *
 * Function to compare two pieces of memory, similar to memcmp function
 * in standard C.
 *
 * Return:
 *      bool - returns a bool value that tells if the memory locations
 *      are equal or not equal.
 *
 */
bool cdf_mem_compare(const void *pMemory1, const void *pMemory2,
		     uint32_t numBytes)
{
	if (0 == numBytes) {
		/* special case where pMemory1 or pMemory2 can be NULL */
		return true;
	}

	if ((pMemory1 == NULL) || (pMemory2 == NULL)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s called with NULL parameter, p1:%p p2:%p",
			  __func__, pMemory1, pMemory2);
		CDF_ASSERT(0);
		return false;
	}
	return memcmp(pMemory1, pMemory2, numBytes) ? false : true;
}

/**
 * cdf_mem_compare2() - memory compare
 * @pMemory1: pointer to one location in memory to compare.
 * @pMemory2:   pointer to second location in memory to compare.
 * @numBytes:   the number of bytes to compare.
 *
 * Function to compare two pieces of memory, similar to memcmp function
 * in standard C.
 * Return:
 *       int32_t - returns a bool value that tells if the memory
 *       locations are equal or not equal.
 *       0 -- equal
 *       < 0 -- *pMemory1 is less than *pMemory2
 *       > 0 -- *pMemory1 is bigger than *pMemory2
 */
int32_t cdf_mem_compare2(const void *pMemory1, const void *pMemory2,
			 uint32_t numBytes)
{
	return (int32_t) memcmp(pMemory1, pMemory2, numBytes);
}

/**
 * cdf_os_mem_alloc_consistent() - allocates consistent cdf memory
 * @osdev: OS device handle
 * @size: Size to be allocated
 * @paddr: Physical address
 * @mctx: Pointer to DMA context
 *
 * Return: pointer of allocated memory or null if memory alloc fails
 */
inline void *cdf_os_mem_alloc_consistent(cdf_device_t osdev, cdf_size_t size,
					 cdf_dma_addr_t *paddr,
					 cdf_dma_context_t memctx)
{
#if defined(A_SIMOS_DEVHOST)
	static int first = 1;
	void *vaddr;

	if (first) {
		first = 0;
		pr_err("Warning: bypassing %s\n", __func__);
	}
	vaddr = cdf_mem_malloc(size);
	*paddr = ((cdf_dma_addr_t) vaddr);
	return vaddr;
#else
	int flags = GFP_KERNEL;
	void *alloc_mem = NULL;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	alloc_mem = dma_alloc_coherent(osdev->dev, size, paddr, flags);
	if (alloc_mem == NULL)
		pr_err("%s Warning: unable to alloc consistent memory of size %zu!\n",
			__func__, size);
	return alloc_mem;
#endif
}

/**
 * cdf_os_mem_free_consistent() - free consistent cdf memory
 * @osdev: OS device handle
 * @size: Size to be allocated
 * @paddr: Physical address
 * @mctx: Pointer to DMA context
 *
 * Return: none
 */
inline void
cdf_os_mem_free_consistent(cdf_device_t osdev,
			   cdf_size_t size,
			   void *vaddr,
			   cdf_dma_addr_t paddr, cdf_dma_context_t memctx)
{
#if defined(A_SIMOS_DEVHOST)
	static int first = 1;

	if (first) {
		first = 0;
		pr_err("Warning: bypassing %s\n", __func__);
	}
	cdf_mem_free(vaddr);
	return;
#else
	dma_free_coherent(osdev->dev, size, vaddr, paddr);
#endif
}


/**
 * cdf_os_mem_dma_sync_single_for_device() - assign memory to device
 * @osdev: OS device handle
 * @bus_addr: dma address to give to the device
 * @size: Size of the memory block
 * @direction: direction data will be dma'ed
 *
 * Assgin memory to the remote device.
 * The cache lines are flushed to ram or invalidated as needed.
 *
 * Return: none
 */

inline void
cdf_os_mem_dma_sync_single_for_device(cdf_device_t osdev,
				      cdf_dma_addr_t bus_addr,
				      cdf_size_t size,
				      enum dma_data_direction direction)
{
	dma_sync_single_for_device(osdev->dev, bus_addr,  size, direction);
}

