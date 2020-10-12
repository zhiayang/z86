// region.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include <unistd.h>
#include <sys/mman.h>

#include "cpu/mem.h"

namespace z86
{
	HostMmapMemoryRegion::HostMmapMemoryRegion(size_t size)
	{
		this->m_ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		assert(this->m_ptr);
		this->m_size = size;
	}

	HostMmapMemoryRegion::~HostMmapMemoryRegion()
	{
		munmap(this->m_ptr, this->m_size);
	}
}
