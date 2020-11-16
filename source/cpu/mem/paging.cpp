// paging.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/mmu.h"

namespace z86
{
	PhysAddr PagedMMU::resolve(VirtAddr addr)
	{
		assert(!m_enabled);
		return PhysAddr(addr.addr);
	}

	void PagedMMU::enable()
	{
		m_enabled = true;
	}

	void PagedMMU::disable()
	{
		m_enabled = false;
	}

	bool PagedMMU::enabled()
	{
		return m_enabled;
	}

	uint8_t  PagedMMU::read8(VirtAddr addr)  { return m_memcon.read8(this->resolve(addr)); }
	uint16_t PagedMMU::read16(VirtAddr addr) { return m_memcon.read16(this->resolve(addr)); }
	uint32_t PagedMMU::read32(VirtAddr addr) { return m_memcon.read32(this->resolve(addr)); }
	uint64_t PagedMMU::read64(VirtAddr addr) { return m_memcon.read64(this->resolve(addr)); }

	void PagedMMU::write8(VirtAddr addr, uint8_t value)     { m_memcon.write8(this->resolve(addr), value); }
	void PagedMMU::write16(VirtAddr addr, uint16_t value)   { m_memcon.write16(this->resolve(addr), value); }
	void PagedMMU::write32(VirtAddr addr, uint32_t value)   { m_memcon.write32(this->resolve(addr), value); }
	void PagedMMU::write64(VirtAddr addr, uint64_t value)   { m_memcon.write64(this->resolve(addr), value); }
}
