// exec.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/cpu.h"
#include "cpu/exec.h"

namespace z86
{
	using Operand = instrad::x86::Operand;
	using Register = instrad::x86::Register;
	using Instruction = instrad::x86::Instruction;
	using InstrMods = instrad::x86::InstrModifiers;

	void op_arithmetic(CPU& cpu, const instrad::x86::Op& op, const InstrMods& mods, const Operand& dst, const Operand& src);

	static void op_mov(CPU& cpu, const InstrMods& mods, const Operand& dst, const Operand& src);
	static void op_push(CPU& cpu, const InstrMods& mods, const Operand& src);
	static void op_pop(CPU& cpu, const InstrMods& mods, const Operand& dst);

	void Executor::execute(const Instruction& instr)
	{
		using namespace instrad::x86;

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
				op_arithmetic(this->m_cpu, op, instr.mods(), instr.dst(), instr.src());
				break;

			case ops::MOV.id():
				op_mov(this->m_cpu, instr.mods(), instr.dst(), instr.src());
				break;

			case ops::PUSH.id():
				op_push(this->m_cpu, instr.mods(), instr.dst());
				break;

			case ops::POP.id():
				op_pop(this->m_cpu, instr.mods(), instr.dst());
				break;

			default:
				assert(false && "invalid opcode");
		}
	}

	static int get_operand_size(CPU& cpu, const InstrMods& mods)
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

		assert(false && "invalid operand size");
		return 0;
	}


	static SegReg convert_sreg(const Register& reg)
	{
		auto idx = reg.index();
		assert(idx & instrad::x86::regs::REG_FLAG_SEGMENT);

		return static_cast<SegReg>(idx & 0x7);
	}

	static std::pair<SegReg, uint64_t> resolve_mem(CPU& cpu, const InstrMods& mods, const Operand& op)
	{
		auto seg = SegReg::DS;
		uint64_t ofs = 0;
		uint64_t idx = 0;

		auto& mem = op.mem();

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

		ofs += mem.displacement();
		ofs += idx * mem.scale();

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
		}
		else if(op.isMemory())
		{
			auto [ seg, ofs ] = resolve_mem(cpu, mods, op);
			zpr::println("seg: {}, ofs: {x} ({x})", seg, ofs, cpu.es());

			switch(get_operand_size(cpu, mods))
			{
				case 16: return cpu.read16(seg, ofs);
				case 32: return cpu.read32(seg, ofs);
				case 64: return cpu.read64(seg, ofs);
			}
		}

		assert(false);
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
			auto [ seg, ofs ] = resolve_mem(cpu, mods, op);
			switch(get_operand_size(cpu, mods))
			{
				case 16: cpu.write16(seg, ofs, value.u16()); return;
				case 32: cpu.write32(seg, ofs, value.u32()); return;
				case 64: cpu.write64(seg, ofs, value.u64()); return;
			}
		}

		assert(false && "invalid destination operand kind");
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

		zpr::println("ofs = {x}, decr = {}, val = {x}", ofs, decr, src_val.u16());
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

		zpr::println("ofs = {x}, incr = {}", ofs, incr);
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
