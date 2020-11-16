// segmentation.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/cpu.h"
#include "cpu/mmu.h"

namespace z86
{
	static void load_segment(CPU* cpu, SegmentedMMU::SystemDescriptor* desc, uint16_t sel)
	{
		assert(cpu->mode() == CPUMode::Real);
		desc->base = sel * 0x10;
		desc->limit = 0xFFFFFFFF;
	}

	static uint64_t resolve_segment(CPU* cpu, SegmentedMMU::SystemDescriptor* desc)
	{
		assert(cpu->mode() == CPUMode::Real);
		return desc->base;
	}

	void SegmentedMMU::load(SegReg sr, uint16_t sel)
	{
		switch(sr)
		{
			case SegReg::CS: return load_segment(&m_cpu, &m_cached_cs, sel);
			case SegReg::DS: return load_segment(&m_cpu, &m_cached_ds, sel);
			case SegReg::ES: return load_segment(&m_cpu, &m_cached_es, sel);
			case SegReg::FS: return load_segment(&m_cpu, &m_cached_fs, sel);
			case SegReg::GS: return load_segment(&m_cpu, &m_cached_gs, sel);
			case SegReg::SS: return load_segment(&m_cpu, &m_cached_ss, sel);
		}

		assert(false && "invalid segment reg");
	}

	void SegmentedMMU::loadCS(uint16_t sel) { this->load(SegReg::CS, sel); }
	void SegmentedMMU::loadDS(uint16_t sel) { this->load(SegReg::DS, sel); }
	void SegmentedMMU::loadES(uint16_t sel) { this->load(SegReg::ES, sel); }
	void SegmentedMMU::loadFS(uint16_t sel) { this->load(SegReg::FS, sel); }
	void SegmentedMMU::loadGS(uint16_t sel) { this->load(SegReg::GS, sel); }
	void SegmentedMMU::loadSS(uint16_t sel) { this->load(SegReg::SS, sel); }

	VirtAddr SegmentedMMU::resolve(SegmentedAddr addr)
	{
		switch(addr.reg)
		{
			case SegReg::CS: return VirtAddr(resolve_segment(&m_cpu, &m_cached_cs) + addr.offset);
			case SegReg::DS: return VirtAddr(resolve_segment(&m_cpu, &m_cached_ds) + addr.offset);
			case SegReg::ES: return VirtAddr(resolve_segment(&m_cpu, &m_cached_es) + addr.offset);
			case SegReg::FS: return VirtAddr(resolve_segment(&m_cpu, &m_cached_fs) + addr.offset);
			case SegReg::GS: return VirtAddr(resolve_segment(&m_cpu, &m_cached_gs) + addr.offset);
			case SegReg::SS: return VirtAddr(resolve_segment(&m_cpu, &m_cached_ss) + addr.offset);
		}

		assert(false && "invalid segment reg");
	}

	uint8_t  SegmentedMMU::read8(SegmentedAddr addr)  { return m_pagedMMU.read8(this->resolve(addr)); }
	uint16_t SegmentedMMU::read16(SegmentedAddr addr) { return m_pagedMMU.read16(this->resolve(addr)); }
	uint32_t SegmentedMMU::read32(SegmentedAddr addr) { return m_pagedMMU.read32(this->resolve(addr)); }
	uint64_t SegmentedMMU::read64(SegmentedAddr addr) { return m_pagedMMU.read64(this->resolve(addr)); }

	void SegmentedMMU::write8(SegmentedAddr addr, uint8_t value)    { m_pagedMMU.write8(this->resolve(addr), value); }
	void SegmentedMMU::write16(SegmentedAddr addr, uint16_t value)  { m_pagedMMU.write16(this->resolve(addr), value); }
	void SegmentedMMU::write32(SegmentedAddr addr, uint32_t value)  { m_pagedMMU.write32(this->resolve(addr), value); }
	void SegmentedMMU::write64(SegmentedAddr addr, uint64_t value)  { m_pagedMMU.write64(this->resolve(addr), value); }
}
