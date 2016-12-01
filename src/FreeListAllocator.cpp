#include "FreeListAllocator.h"
#include "Utils.h"		/* CalculatePaddingWithHeader */
#include <stdlib.h>     /* malloc, free */
#include <cassert> 		/* assert		*/
#include <limits>		/* limits_max */
#ifdef _DEBUG
	#include <iostream>
#endif

FreeListAllocator::FreeListAllocator(const std::size_t totalSize, enum PlacementPolicy policy);
	: Allocator(totalSize) {
	m_pPolicy = policy;
}

void FreeListAllocator::Init() {
	if (m_freeList != nullptr){
		delete m_freeList;
	}
	if (m_start_ptr != nullptr){
		free(m_start_ptr);
		m_start_ptr = nullptr;
	}
	// Init linkedlist with only one element
	m_start_ptr = malloc(m_totalSize);

	m_freeList = (FreeBlock *) m_start_ptr;
	m_freeList.m_head->size = m_totalSize;
	m_freeList.m_head->previous = NULL;
	m_freeList.m_head->next = NULL;
}

FreeListAllocator::~FreeListAllocator(){
	free(m_start_ptr);
	m_start_ptr = nullptr;
}

void* FreeListAllocator::Allocate(const std::size_t size, const std::size_t alignment){
	// Search through the free list for a free block that has enough space to allocate our data
	FreeBlock * affectedBlock = this->Find(size);
	const int rest = affectedBlock->size - size;
	if (rest == 0){
		// We have to split the block into the data block and a free block of size 'rest'
		FreeBlock * newFreeBlock = (FreeBlock *)((std::size_t) affectedBlock + size + 1);

		affectedBlock->previous->next = newFreeBlock;
		newFreeBlock->previous = affectedBlock->previous;
		newFreeBlock->next = affectedBlock->next;
		newFreeBlock->size = rest;
	}else {
		// Delete block from free list
		affectedBlock->previous->next = affectedBlock->next;
	}

#ifdef _DEBUG
	std::cout << "A" << "\t@C " << (void*) affectedBlock << std::endl;
#endif
	
	return (void*) affectedBlock;
}

FreeBlock * FreeListAllocator::Find(const std::size_t size){
	switch(m_pPolicy){
		case FIND_FIRST:
			return FindFirst(size);
		case FIND_BEST:
			return FindBest(size);
		default:
			return nullptr;
	}
}

FreeBlock * FreeListAllocator::FindFirst(const std::size_t size){
	//Iterate list and return the first free block with a size >= than given size
	FreeBlock * it = m_freeList;
	while(it != nullptr){
		if (it->size >= size){
			return it;
		}
		it = it->next;
	}
	return nullptr;
}

FreeBlock * FreeListAllocator::FindBest(const std::size_t size){
	// Iterate WHOLE list keeping a pointer to the best fit
	std::size_t smallestDiff = 	std::numeric_limits<T>::max();
	FreeBlock * bestBlock = nullptr;
	FreeBlock * it = m_freeList;
	while(it != nullptr){
		if (it->size >= size && (it->size - size < smallestDiff)){
			bestBlock = it;
		}
		it = it->next;
	}
	return bestBlock;
}
void FreeListAllocator::Free(void* ptr){
	//TODO
}

void FreeListAllocator::Reset() {
	m_freeList = (FreeBlock *) m_start_ptr;
	m_freeList.m_head->size = m_totalSize;
	m_freeList.m_head->previous = NULL;
	m_freeList.m_head->next = NULL;
}
