// exec.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "defs.h"
#include "cpu/cpu.h"
#include "cpu/exec.h"

namespace z86
{
	using Operand = instrad::x86::Operand;
	using Register = instrad::x86::Register;
	using Instruction = instrad::x86::Instruction;
	using InstrMods = instrad::x86::InstrModifiers;

	// jump.cpp
	void op_jcxz(CPU& cpu, const InstrMods& mods, const Operand& dst);
	void op_jmp(CPU& cpu, const InstrMods& mods, const Operand& dst);
	void op_jo(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);
	void op_js(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);
	void op_ja(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);
	void op_jz(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);
	void op_jp(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);
	void op_jl(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);
	void op_jg(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);
	void op_jc(CPU& cpu, const InstrMods& mods, const Operand& dst, bool check);

	// arithmetic.cpp
	void op_inc_dec(CPU& cpu, const instrad::x86::Op& op, const InstrMods& mods, const Operand& dst);
	void op_arithmetic(CPU& cpu, const instrad::x86::Op& op, const InstrMods& mods, const Operand& dst, const Operand& src);

	// adjust.cpp
	void op_daa(CPU& cpu);
	void op_das(CPU& cpu);
	void op_aaa(CPU& cpu);
	void op_aas(CPU& cpu);
	void op_aad(CPU& cpu, uint8_t base);
	void op_aam(CPU& cpu, uint8_t base);

	static void op_xchg(CPU& cpu, const InstrMods& mods, const Operand& dst, const Operand& src);
	static void op_mov(CPU& cpu, const InstrMods& mods, const Operand& dst, const Operand& src);
	static void op_pop(CPU& cpu, const InstrMods& mods, const Operand& dst);
	static void op_push(CPU& cpu, const InstrMods& mods, const Operand& src);

	void Executor::execute(const Instruction& instr)
	{
		using namespace instrad::x86;

		if(instr.lockPrefix())
			m_cpu.memLock();

		auto& op = instr.op();
		switch(op.id())
		{
			case ops::ADD.id():
			case ops::ADC.id():
			case ops::SUB.id():
			case ops::SBB.id():
			case ops::AND.id():
			case ops::OR.id():
			case ops::XOR.id():
			case ops::CMP.id():
			case ops::TEST.id():
				op_arithmetic(m_cpu, op, instr.mods(), instr.dst(), instr.src());
				break;

			case ops::INC.id():
			case ops::DEC.id():
				op_inc_dec(m_cpu, op, instr.mods(), instr.dst());
				break;

			case ops::MOV.id():
				op_mov(m_cpu, instr.mods(), instr.dst(), instr.src());
				break;

			case ops::PUSH.id():
				op_push(m_cpu, instr.mods(), instr.dst());
				break;

			case ops::POP.id():
				op_pop(m_cpu, instr.mods(), instr.dst());
				break;

			case ops::XCHG.id():
				op_xchg(m_cpu, instr.mods(), instr.dst(), instr.src());
				break;



			case ops::DAA.id():
				op_daa(m_cpu);
				break;

			case ops::DAS.id():
				op_das(m_cpu);
				break;

			case ops::AAA.id():
				op_aaa(m_cpu);
				break;

			case ops::AAS.id():
				op_aas(m_cpu);
				break;

			case ops::AAM.id():
				op_aam(m_cpu, instr.dst().imm() & 0xFF);
				break;

			case ops::AAD.id():
				op_aad(m_cpu, instr.dst().imm() & 0xFF);
				break;



			// uwu
			case ops::JMP.id():
				op_jmp(m_cpu, instr.mods(), instr.dst());
				break;

			case ops::JO.id():
				op_jo(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JNO.id():
				op_jo(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JS.id():
				op_js(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JNS.id():
				op_js(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JZ.id():
				op_jz(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JNZ.id():
				op_jz(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JB.id():
				op_jc(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JNB.id():
				op_jc(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JA.id():
				op_ja(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JNA.id():
				op_ja(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JL.id():
				op_jl(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JGE.id():
				op_jl(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JG.id():
				op_jg(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JLE.id():
				op_jg(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JP.id():
				op_jp(m_cpu, instr.mods(), instr.dst(), true);
				break;

			case ops::JNP.id():
				op_jp(m_cpu, instr.mods(), instr.dst(), false);
				break;

			case ops::JCXZ.id():
				op_jcxz(m_cpu, instr.mods(), instr.dst());
				break;

			default:
				lg::fatal("exec", "invalid opcode: {}", print_att(instr, m_cpu.ip(), 0, 1));
				break;
		}

		if(instr.lockPrefix())
			m_cpu.memUnlock();
	}

	int get_operand_size(CPU& cpu, const InstrMods& mods)
	{
		if(cpu.mode() == CPUMode::Real)
		{
			return mods.operandSizeOverride
				? 32
				: 16;
		}
		else if(cpu.mode() == CPUMode::Prot || cpu.mode() == CPUMode::Long)
		{
			if(mods.operandSizeOverride)    return 16;
			else if(mods.rex.W())           return 64;
			else                            return 32;
		}
		else
		{
			assert(false && "invalid cpu mode");
		}

		assert(false && "invalid operand size");
		return 0;
	}

	int get_address_size(CPU& cpu, const InstrMods& mods)
	{
		if(cpu.mode() == CPUMode::Real)
		{
			return mods.addressSizeOverride
				? 32
				: 16;
		}
		else if(cpu.mode() == CPUMode::Prot || cpu.mode() == CPUMode::Long)
		{
			if(mods.addressSizeOverride)    return 16;
			else if(mods.rex.W())           return 64;
			else                            return 32;
		}
		else
		{
			assert(false && "invalid cpu mode");
		}

		assert(false && "invalid address size");
		return 0;
	}



	static SegReg convert_sreg(const Register& reg)
	{
		auto idx = reg.index();
		assert(idx & instrad::x86::regs::REG_FLAG_SEGMENT);

		return static_cast<SegReg>(idx & 0x7);
	}

	std::pair<SegReg, uint64_t> resolve_memory_access(CPU& cpu, const instrad::x86::MemoryRef& mem)
	{
		auto seg = SegReg::DS;
		uint64_t ofs = 0;
		uint64_t idx = 0;

		if(mem.segment().present())
			seg = convert_sreg(mem.segment());

		if(cpu.mode() == CPUMode::Real)
		{
			ofs += mem.base().present() ? cpu.reg16(mem.base()) : 0;
			idx = mem.index().present() ? cpu.reg16(mem.index()) : 0;
		}
		else if(cpu.mode() == CPUMode::Prot)
		{
			ofs += mem.base().present() ? cpu.reg32(mem.base()) : 0;
			idx = mem.index().present() ? cpu.reg32(mem.index()) : 0;
		}
		else if(cpu.mode() == CPUMode::Long)
		{
			ofs += mem.base().present() ? cpu.reg64(mem.base()) : 0;
			idx = mem.index().present() ? cpu.reg64(mem.index()) : 0;
		}
		else
		{
			assert(false && "invalid cpu mode");
		}

		ofs += mem.displacement();
		ofs += idx * mem.scale();

		// a little gross to repeat it here, but whatever...
		if(cpu.mode() == CPUMode::Real)         ofs &= 0xFFFF;
		else if(cpu.mode() == CPUMode::Prot)    ofs &= 0xFFFF'FFFF;
		else if(cpu.mode() == CPUMode::Long)    ofs &= 0xFFFF'FFFF'FFFF'FFFF;
		else                                    assert(false && "invalid cpu mode");

		return { seg, ofs };
	}

	Value get_operand(CPU& cpu, const InstrMods& mods, const Operand& op)
	{
		if(op.isRegister())
		{
			switch(get_operand_size(cpu, mods))
			{
				case 16: return cpu.reg16(op.reg());
				case 32: return cpu.reg32(op.reg());
				case 64: return cpu.reg64(op.reg());
			}
		}
		else if(op.isImmediate())
		{
			switch(op.immediateSize())
			{
				case 8:     return Value(static_cast<uint8_t>(op.imm()));
				case 16:    return Value(static_cast<uint16_t>(op.imm()));
				case 32:    return Value(static_cast<uint32_t>(op.imm()));
				case 64:    return Value(static_cast<uint64_t>(op.imm()));
			}
		}
		else if(op.isRelativeOffset())
		{
			return static_cast<uint64_t>(op.ofs().offset());
		}
		else if(op.isMemory())
		{
			auto [ seg, ofs ] = resolve_memory_access(cpu, op.mem());
			switch(get_operand_size(cpu, mods))
			{
				case 16: return cpu.read16(seg, ofs);
				case 32: return cpu.read32(seg, ofs);
				case 64: return cpu.read64(seg, ofs);
			}
		}

		// note: i'm deliberately *NOT* handling FarOffset here,
		// because it's a little complicated, and in particular we might need to
		// return 80-bit values, so it's better to just let the (small) handful of
		// instructions that need it to handle it themselves.

		assert(false && "invalid operand kind");
		return static_cast<uint64_t>(0);
	}

	void set_operand(CPU& cpu, const InstrMods& mods, const Operand& op, Value value)
	{
		if(op.isRegister())
		{
			switch(get_operand_size(cpu, mods))
			{
				case 16: cpu.reg16(op.reg()) = value.u16(); return;
				case 32: cpu.reg32(op.reg()) = value.u32(); return;
				case 64: cpu.reg64(op.reg()) = value.u64(); return;
			}
		}
		else if(op.isMemory())
		{
			auto [ seg, ofs ] = resolve_memory_access(cpu, op.mem());
			switch(get_operand_size(cpu, mods))
			{
				case 16: cpu.write16(seg, ofs, value.u16()); return;
				case 32: cpu.write32(seg, ofs, value.u32()); return;
				case 64: cpu.write64(seg, ofs, value.u64()); return;
			}
		}

		assert(false && "invalid destination operand kind");
	}













	static void op_xchg(CPU& cpu, const InstrMods& mods, const Operand& dst, const Operand& src)
	{
		// xchg always asserts the lock signal
		cpu.memLock();

		auto s = get_operand(cpu, mods, src);
		auto d = get_operand(cpu, mods, dst);

		set_operand(cpu, mods, dst, s);
		set_operand(cpu, mods, src, d);

		cpu.memUnlock();
	}

	static void op_push(CPU& cpu, const InstrMods& mods, const Operand& src)
	{
		auto src_val = get_operand(cpu, mods, src);
		auto decr = get_operand_size(cpu, mods) / 8;
		uint64_t ofs = 0;

		switch(cpu.mode())
		{
			case CPUMode::Real: ofs = (cpu.sp() -= decr); break;
			case CPUMode::Prot: ofs = (cpu.esp() -= decr); break;
			case CPUMode::Long: ofs = (cpu.rsp() -= decr); break;
		}

		switch(decr)
		{
			case 2: return cpu.write16(SegReg::SS, ofs, src_val.u16());
			case 4: return cpu.write32(SegReg::SS, ofs, src_val.u32());
			case 8: return cpu.write64(SegReg::SS, ofs, src_val.u64());
		}

		assert(false && "owo");
	}

	static void op_pop(CPU& cpu, const InstrMods& mods, const Operand& dst)
	{
		auto incr = get_operand_size(cpu, mods) / 8;
		uint64_t ofs = 0;

		switch(cpu.mode())
		{
			case CPUMode::Real: ofs = cpu.sp(), cpu.sp() += incr; break;
			case CPUMode::Prot: ofs = cpu.esp(), cpu.esp() += incr; break;
			case CPUMode::Long: ofs = cpu.rsp(), cpu.rsp() += incr; break;
		}

		switch(incr)
		{
			case 2: return set_operand(cpu, mods, dst, cpu.read16(SegReg::SS, ofs));
			case 4: return set_operand(cpu, mods, dst, cpu.read32(SegReg::SS, ofs));
			case 8: return set_operand(cpu, mods, dst, cpu.read64(SegReg::SS, ofs));
		}

		assert(false && "owo");
	}

	static void op_mov(CPU& cpu, const InstrMods& mods, const Operand& dst, const Operand& src)
	{
		auto src_val = get_operand(cpu, mods, src);
		set_operand(cpu, mods, dst, src_val);
	}
}
