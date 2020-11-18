// cpu.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "defs.h"
#include "cpu/cpu.h"

namespace z86
{
	static void dump(CPU& cpu)
	{
		if(cpu.mode() == z86::CPUMode::Real)
		{
			zpr::println("ax:    {4.2x}    bx:    {4.2x}", cpu.ax(), cpu.bx());
			zpr::println("cx:    {4.2x}    dx:    {4.2x}", cpu.cx(), cpu.dx());
			zpr::println("di:    {4.2x}    si:    {4.2x}", cpu.di(), cpu.si());
			zpr::println("bp:    {4.2x}    sp:    {4.2x}", cpu.bp(), cpu.sp());
			zpr::println("cs:    {4.2x}    ip:    {4.2x}", cpu.cs().get(), cpu.ip());
			zpr::println("ds:    {4.2x}    ss:    {4.2x}", cpu.ds().get(), cpu.ss().get());
			zpr::println("es:    {4.2x}    fs:    {4.2x}", cpu.es().get(), cpu.fs().get());
			zpr::println("gs:    {4.2x}", cpu.gs().get());
			zpr::println("flags: {016b}", cpu.flags().flags());
			zpr::println("           ODITSZ A P C");
			zpr::println("");
		}
	}


	CPU::CPU() : m_exec(*this), m_pmmu(*this, m_memory), m_smmu(*this, m_pmmu)
	{
	}

	void CPU::reset()
	{
		// AMD Manual, vol 2, 14.1.3
		// Processor Initialisation State

		// cr0 = 0x60000010
		// cr2-4 = 0

		m_mode = CPUMode::Real;

		// first we reset the smmu
		m_smmu.reset();

		// gdtr, idtr, ldtr, tr are base 0 with limit 0xFFFF

		// then, we set the selectors. note that this
		// doesn't change the cached base/limit, which is what we need
		// because there's some special stuff in there. (see segmentation.cpp)
		m_segment_regs[IDX_CS] = 0xF000;
		m_segment_regs[IDX_DS] = 0;
		m_segment_regs[IDX_ES] = 0;
		m_segment_regs[IDX_FS] = 0;
		m_segment_regs[IDX_GS] = 0;
		m_segment_regs[IDX_SS] = 0;

		// clear all the registers.
		for(size_t i = 0; i < 16; i++)
			m_gprs[i].low_64 = 0;

		// for some reason, EDX is special.
		// it's some cpuid stuff: edx[3:0] = stepping
		// edx[19:16] = extended model
		// edx[7:4] = model
		// edx[27:20] = extended family
		// edx[11:8] = family

		// for now just... set it to 0x30, which supposedly indicates
		// an 80386 with stepping 0, model 3.
		this->edx() = 0x30;

		// IP is set to 0xFFF0
		m_ip = 0xFFF0;



		// this is pretty important...
		auto test = GeneralPurposeReg {
			.low_64 = 0x0123'4567'89AB'CDEF
		};

		assert(test.low_8  == 0xEF);
		assert(test.high_8 == 0xCD);
		assert(test.low_16 == 0xCDEF);
		assert(test.low_32 == 0x89AB'CDEF);
		assert(test.low_64 == 0x0123'4567'89AB'CDEF);
	}

	void CPU::start()
	{
		this->reset();

		while(true)
		{
			if(!this->run(this->decode()))
				break;
		}
	}

	instrad::x86::Instruction CPU::decode()
	{
		auto buf = Buffer(*this);
		auto begin = buf.position();

		auto ret = instrad::x86::read(buf, instrad::x86::ExecMode::Legacy);

		m_ip += (buf.position() - begin);
		return ret;
	}

	bool CPU::run(instrad::x86::Instruction instr)
	{
		if(instr.op() == instrad::x86::ops::HLT)
			return false;

		m_exec.execute(instr);

		zpr::println("{}", print_att(instr, this->ip(), 0, 1));
		dump(*this);

		return true;
	}


	void CPU::jump(uint64_t ip)
	{
		m_ip = ip;
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

	static void segment_loader(CPU* cpu, short idx, uint16_t val)
	{
		cpu->smmu().load(static_cast<SegReg>(idx & 0x7), val);
	}

	RegWrapper<uint16_t> CPU::cs() { return RegWrapper<uint16_t>(this, IDX_CS, m_segment_regs[IDX_CS], &segment_loader); }
	RegWrapper<uint16_t> CPU::ds() { return RegWrapper<uint16_t>(this, IDX_DS, m_segment_regs[IDX_DS], &segment_loader); }
	RegWrapper<uint16_t> CPU::es() { return RegWrapper<uint16_t>(this, IDX_ES, m_segment_regs[IDX_ES], &segment_loader); }
	RegWrapper<uint16_t> CPU::fs() { return RegWrapper<uint16_t>(this, IDX_FS, m_segment_regs[IDX_FS], &segment_loader); }
	RegWrapper<uint16_t> CPU::gs() { return RegWrapper<uint16_t>(this, IDX_GS, m_segment_regs[IDX_GS], &segment_loader); }
	RegWrapper<uint16_t> CPU::ss() { return RegWrapper<uint16_t>(this, IDX_SS, m_segment_regs[IDX_SS], &segment_loader); }

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
			return RegWrapper<uint16_t>(this, idx, m_segment_regs[idx & 0x7], &segment_loader);
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
