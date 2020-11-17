// region.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include <unistd.h>
#include <sys/mman.h>

#include "defs.h"
#include "cpu/mem.h"

namespace z86
{
	HostMmapMemoryRegion::HostMmapMemoryRegion(size_t size, bool writable) : MemoryRegion(size), m_writable(writable)
	{
		auto ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if(ptr == (void*) -1)
			lg::fatal("mem", "failed to mmap host region (size {} bytes)", size);

		m_ptr = reinterpret_cast<uint8_t*>(ptr);
	}

	HostMmapMemoryRegion::~HostMmapMemoryRegion()
	{
		munmap(m_ptr, m_size);
	}
}
