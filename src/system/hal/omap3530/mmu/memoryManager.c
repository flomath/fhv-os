/*
 * memoryManager.c
 *
 * Created on: 17.04.15
 *     Author: Nino Schoch
 */

#include <stdlib.h>
#include <string.h>
#include "memoryManager.h"
#include "../../common/mmu/memoryManager.h"

memoryRegion_t memoryRegions[MEMORY_REGIONS];

/**
 * Initialize the region with addresses
 * and based on the used page size
 *
 * Reserve directed mapped regions directly
 */
static void memoryManager_initRegion(memoryRegion_t* memRegion, uint32_t addressStart, uint32_t addressEnd, unsigned char directMapped);

/**
 * Reserve a page in a memory region
 */
static void memoryManager_reservePage(memoryRegion_t* memRegion, unsigned int pageNumber);

/**
 * Reserve a number of pages in a memory region
 *
 * Returns -1 for success - pages reserved
 * Returns >= 0 for failure - already a reserved page was found
 */
static int memoryManager_reservePages(memoryRegion_t* memRegion, unsigned int pageNumber, unsigned int numPagesReserve);

/**
 * Check if a given number of pages in a region
 * at a specific page number are free
 *
 * Returns -1 for success - free pages available
 * Returns >= 0 for failure - already a reserved page was found
 */
static int memoryManager_areFreePages(memoryRegion_t* memRegion, unsigned int pageNumber, unsigned int numPages);

/**
 * Get address of given page number
 * based on the region
 */
static uint32_t* memoryManager_getPageAddress(memoryRegion_t* memRegion, unsigned int pageNumber);

void memoryManager_init(void)
{
	// init regions
	memoryManager_initRegion(&memoryRegions[BOOT_ROM_REGION     ], BOOT_ROM_START_ADDRESS     , BOOT_ROM_END_ADDRESS     , 1);
	memoryManager_initRegion(&memoryRegions[INTERNAL_SRAM_REGION], INTERNAL_SRAM_START_ADDRESS, INTERNAL_SRAM_END_ADDRESS, 1);
	memoryManager_initRegion(&memoryRegions[MMIO_REGION         ], MMIO_START_ADDRESS         , MMIO_END_ADDRESS         , 1);
	memoryManager_initRegion(&memoryRegions[KERNEL_REGION       ], KERNEL_START_ADDRESS       , KERNEL_END_ADDRESS       , 1);
	memoryManager_initRegion(&memoryRegions[PAGE_TABLE_REGION   ], PAGE_TABLES_START_ADDRESS  , PAGE_TABLES_END_ADDRESS  , 1);
	memoryManager_initRegion(&memoryRegions[PROCESS_REGION      ], PROCESS_PAGES_START_ADDRESS, PROCESS_PAGES_END_ADDRESS, 0); //TODO: physical address of process region?
}

memoryRegion_t* memoryManger_getRegion(unsigned int memRegionNumber)
{
	return &memoryRegions[memRegionNumber];
}

uint32_t* memoryManager_getFreePages(unsigned int memRegionNumber, unsigned int numPagesReserve)
{
	memoryRegion_t* memRegion = memoryManger_getRegion(memRegionNumber);

	// check if pages are available
	if( numPagesReserve == 0 || numPagesReserve > (memRegion->numPages - memRegion->numPagesReserved) ) {
		return NULL;
	}

	// loop through all pages and look
	// for specified number of pages
	unsigned int pageNumber;
	int pageStatus;
	for(pageNumber = 0; pageNumber < memRegion->numPages; pageNumber++)
	{
		// if a page is found, check
		// if more are available
		if( (pageNumber + numPagesReserve) > memRegion->numPages ) {
			return NULL;
		}

		// if a page is reserved try to find next one
		// if not, check if number of reserved pages are free
		if( memRegion->pages[pageNumber].reserved == 1 ) {
			continue;
		} else {
			// try to reserve pages
			pageStatus = memoryManager_reservePages(memRegion, pageNumber, numPagesReserve);

			// if pageNumber is not -1 (free pages available)
			// search for new one
			// otherwise pages were reserved and return address of first page
			if( pageStatus != -1 ) {
				continue;
			} else {
				uint32_t* pageAddress = memoryManager_getPageAddress(memRegion, pageNumber);

				if( pageAddress == NULL ) {
					return NULL;
				}

				// fill all pages with page faults
				memset(pageAddress, FAULT_PAGE_HIT, SMALL_PAGE_SIZE_4KB * numPagesReserve);
				return pageAddress;
			}
		}
	}

	return NULL;
}

/**
 * Static functions
 */

static void memoryManager_initRegion(memoryRegion_t* memRegion, uint32_t addressStart, uint32_t addressEnd, unsigned char directMapped)
{
	memRegion->addressStart 		= addressStart;
	memRegion->addressEnd 			= addressEnd;
	memRegion->pageSize				= SMALL_PAGE_SIZE_4KB;
	memRegion->numPages				= (addressStart-addressEnd) / memRegion->pageSize;
	memRegion->numPagesReserved		= 0;

	// reserve all pages in a region
	// that is directly mapped vA-pA
	if( directMapped == 1 ) {
		memoryManager_reservePages(memRegion, 0, memRegion->numPages);
	}
}

static void memoryManager_reservePage(memoryRegion_t* memRegion, unsigned int pageNumber)
{
	// check if region can address
	// number of page
	if( pageNumber > memRegion->numPages ) {
		return;
	}

	memRegion->pages[pageNumber].reserved = 1;
	memRegion->numPagesReserved++;
}

static int memoryManager_reservePages(memoryRegion_t* memRegion, unsigned int pageNumber, unsigned int numPagesReserve)
{
	// check if free pages are available
	// if status is not -1 (free pages available)
	// return index of first reserved page found
	int status = memoryManager_areFreePages(memRegion, pageNumber, numPagesReserve);
	if( status != -1 ) {
		return status;
	}

	// reserve given pages
	for(pageNumber; pageNumber < numPagesReserve; pageNumber++) {
		memoryManager_reservePage(memRegion, pageNumber);
	}

	return status;
}

static int memoryManager_areFreePages(memoryRegion_t* memRegion, unsigned int pageNumber, unsigned int numPages)
{
	// if region has not enough pages,
	// don't reserve
	if( (pageNumber + numPages) > (memRegion->numPages - memRegion->numPagesReserved) ) {
		return pageNumber;
	}

	unsigned int i = pageNumber;
	for(i; i < (pageNumber + numPages); i++) {
		// if a page is found, which is reserved,
		// return index - better performance
		if(memRegion->pages[i].reserved == 1) {
			return i;
		}
	}

	return -1;
}

static uint32_t* memoryManager_getPageAddress(memoryRegion_t* memRegion, unsigned int pageNumber)
{
	// check if region can address
	// number of page
	if( pageNumber > memRegion->numPages ) {
		return NULL;
	}

	return (uint32_t*)(memRegion->addressStart + pageNumber * memRegion->pageSize);
}
