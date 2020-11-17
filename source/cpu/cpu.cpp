// cpu.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/cpu.h"

namespace z86
{
	CPU::CPU() : m_exec(*this), m_pmmu(*this, m_memory), m_smmu(*this, m_pmmu)
	{
	}

	void CPU::memLock() { m_memory.lock(); }
	void CPU::memUnlock() { m_memory.unlock(); }

	uint8_t  CPU::read8(uint64_t address)  { return this->read8(SegReg::DS, address); }
	uint16_t CPU::read16(uint64_t address) { return this->read16(SegReg::DS, address); }
	uint32_t CPU::read32(uint64_t address) { return this->read32(SegReg::DS, address); }
	uint64_t CPU::read64(uint64_t address) { return this->read64(SegReg::DS, address); }

	uint8_t CPU::read8(SegReg seg, uint64_t address)    { return m_smmu.read8(SegmentedAddr(seg, address)); }
	uint16_t CPU::read16(SegReg seg, uint64_t address)  { return m_smmu.read16(SegmentedAddr(seg, address)); }
	uint32_t CPU::read32(SegReg seg, uint64_t address)  { return m_smmu.read32(SegmentedAddr(seg, address)); }
	uint64_t CPU::read64(SegReg seg, uint64_t address)  { return m_smmu.read64(SegmentedAddr(seg, address)); }

	void CPU::write8(SegReg seg, uint64_t address, uint8_t value)   { return m_smmu.write8(SegmentedAddr(seg, address), value); }
	void CPU::write16(SegReg seg, uint64_t address, uint16_t value) { return m_smmu.write16(SegmentedAddr(seg, address), value); }
	void CPU::write32(SegReg seg, uint64_t address, uint32_t value) { return m_smmu.write32(SegmentedAddr(seg, address), value); }
	void CPU::write64(SegReg seg, uint64_t address, uint64_t value) { return m_smmu.write64(SegmentedAddr(seg, address), value); }

	RegWrapper<uint8_t> CPU::reg8(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		auto idx = reg.index();
		if(idx & regs::REG_FLAG_HI_BYTE && (idx & ~regs::REG_FLAG_HI_BYTE) < 4)
		{
			return m_gprs[idx & 0x4].high_8;
		}
		else if(idx >= 0 && idx < 16)
		{
			return m_gprs[idx].low_8;
		}

		assert(false && "invalid register");
		return this->al();
	}

	RegWrapper<uint16_t> CPU::reg16(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		auto idx = reg.index();
		if(idx & regs::REG_FLAG_SEGMENT)
		{
			return RegWrapper<uint16_t>(this, idx, m_segment_regs[idx & 0x7], [](CPU* cpu, short idx, uint16_t val){
				cpu->m_smmu.load(static_cast<SegReg>(idx & 0x7), val);
			});
		}
		else if(idx >= 0 && idx < 16)
		{
			return this->m_gprs[idx].low_16;
		}

		assert(false && "invalid register");
		return this->ax();
	}

	RegWrapper<uint32_t> CPU::reg32(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		auto idx = reg.index();
		if(idx >= 0 && idx < 16)
		{
			return m_gprs[idx].low_32;
		}

		assert(false && "invalid register");
		return this->eax();
	}

	RegWrapper<uint64_t> CPU::reg64(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		auto idx = reg.index();
		if(idx >= 0 && idx < 16)
		{
			return m_gprs[idx].low_64;
		}

		assert(false && "invalid register");
		return this->rax();
	}
}
