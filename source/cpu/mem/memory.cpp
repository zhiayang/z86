// memory.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/mem.h"

namespace z86
{
	MemoryController::MemoryController()
	{
		// for now, create 64k of memory.
		auto r = new HostMmapMemoryRegion(0x100000);
		this->m_regions.push_back(RegionMapping {
			.start  = PhysAddr(0),
			.length = 0x100000,
			.region = r
		});
	}

	MemoryController::~MemoryController()
	{
		for(auto& reg : this->m_regions)
			delete reg.region;
	}

	void MemoryController::lock() { }
	void MemoryController::unlock() { }

	static inline bool contains(MemoryController::RegionMapping& region, PhysAddr addr)
	{
		return region.start.addr <= addr.addr && addr.addr <= region.start.addr + region.length;
	}

	uint8_t MemoryController::read8(PhysAddr addr)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->read8(addr.addr);

		assert(false && "out of bounds memory read");
	}

	uint16_t MemoryController::read16(PhysAddr addr)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->read16(addr.addr);

		assert(false && "out of bounds memory read");
	}

	uint32_t MemoryController::read32(PhysAddr addr)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->read32(addr.addr);

		assert(false && "out of bounds memory read");
	}

	uint64_t MemoryController::read64(PhysAddr addr)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->read64(addr.addr);

		assert(false && "out of bounds memory read");
	}

	void MemoryController::write8(PhysAddr addr, uint8_t value)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->write8(addr.addr, value);

		assert(false && "out of bounds memory read");
	}

	void MemoryController::write16(PhysAddr addr, uint16_t value)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->write16(addr.addr, value);

		assert(false && "out of bounds memory read");
	}

	void MemoryController::write32(PhysAddr addr, uint32_t value)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->write32(addr.addr, value);

		assert(false && "out of bounds memory read");
	}

	void MemoryController::write64(PhysAddr addr, uint64_t value)
	{
		for(auto& region : this->m_regions)
			if(contains(region, addr))
				return region.region->write64(addr.addr, value);

		assert(false && "out of bounds memory read");
	}


}
