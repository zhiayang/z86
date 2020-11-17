// memory.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "defs.h"
#include "cpu/mem.h"

namespace z86
{
	MemoryController::MemoryController()
	{
		// the invariant on m_regions is that it is sorted.
		// for now, create 64k of memory.

		auto r = new HostMmapMemoryRegion(0x100000, /* writable: */ true);
		m_regions.push_back(RegionMapping {
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
		return region.start.addr <= addr.addr && addr.addr < region.start.addr + region.length;
	}

	static inline MemoryController::RegionMapping* find_region(std::vector<MemoryController::RegionMapping>& list, PhysAddr addr)
	{
		// for now, a linear search will suffice.
		for(auto& r : list)
		{
			if(contains(r, addr))
				return &r;
		}

		return nullptr;
	}





	void MemoryController::addRegion(PhysAddr start, MemoryRegion* region)
	{
		// check overlap with existing regions
		for(auto& reg : m_regions)
		{
			if(contains(reg, start))
				lg::fatal("mem", "overlapping regions");
		}

		m_regions.push_back(RegionMapping {
			.start  = start,
			.length = region->size(),
			.region = region
		});

		std::sort(m_regions.begin(), m_regions.end(), [](const auto& a, const auto& b) -> bool {
			return a.start.addr < b.start.addr;
		});
	}


	void MemoryController::read(PhysAddr addr, uint8_t* buf, size_t len)
	{
		// this is possible only because regions are sorted.
		for(auto& region : m_regions)
		{
			if(contains(region, addr))
			{
				auto done = std::min(region.length, len);
				region.region->read(addr.addr - region.start.addr, buf, done);

				len -= done;
				buf += done;
				addr.addr += done;
			}

			if(len == 0)
				break;
		}
	}

	void MemoryController::write(PhysAddr addr, const uint8_t* buf, size_t len)
	{
		// this is possible only because regions are sorted.
		for(auto& region : m_regions)
		{
			if(contains(region, addr))
			{
				auto done = std::min(region.length, len);
				region.region->write(addr.addr - region.start.addr, buf, done);

				len -= done;
				buf += done;
				addr.addr += done;
			}

			if(len == 0)
				break;
		}
	}

	uint8_t MemoryController::read8(PhysAddr addr)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		return r->region->read8(addr.addr - r->start.addr);
	}

	uint16_t MemoryController::read16(PhysAddr addr)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		return r->region->read16(addr.addr - r->start.addr);
	}

	uint32_t MemoryController::read32(PhysAddr addr)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		return r->region->read32(addr.addr - r->start.addr);
	}

	uint64_t MemoryController::read64(PhysAddr addr)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		return r->region->read64(addr.addr - r->start.addr);
	}

	void MemoryController::write8(PhysAddr addr, uint8_t value)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		r->region->write8(addr.addr - r->start.addr, value);
	}

	void MemoryController::write16(PhysAddr addr, uint16_t value)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		r->region->write16(addr.addr - r->start.addr, value);
	}

	void MemoryController::write32(PhysAddr addr, uint32_t value)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		r->region->write16(addr.addr - r->start.addr, value);
	}

	void MemoryController::write64(PhysAddr addr, uint64_t value)
	{
		auto r = find_region(m_regions, addr);
		assert(r && "out of bounds memory read");

		r->region->write64(addr.addr - r->start.addr, value);
	}
}
