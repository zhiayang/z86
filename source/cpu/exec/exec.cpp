// exec.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/cpu.h"
#include "cpu/exec.h"

namespace z86
{
	using Operand = instrad::x86::Operand;
	using Instruction = instrad::x86::Instruction;
	using InstrMods = instrad::x86::InstrModifiers;

	void op_arithmetic(CPU& cpu, const instrad::x86::Op& op, const InstrMods& mods, const Operand& dst, const Operand& src);

	static void op_mov(CPU& cpu, const InstrMods& mods, const Operand& dst, const Operand& src)
	{
		auto src_val = get_operand(cpu, mods, src);
		set_operand(cpu, mods, dst, src_val);
	}

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



			default:
				assert(false && "invalid opcode");
		}
	}















	Value get_operand(CPU& cpu, const InstrMods& mods, const Operand& op)
	{
		if(op.isRegister())
		{
			if(cpu.mode() == CPUMode::Real)
			{
				if(mods.operandSizeOverride)
					return cpu.reg32(op.reg());
				else
					return cpu.reg16(op.reg());
			}
			else if(cpu.mode() == CPUMode::Prot || cpu.mode() == CPUMode::Long)
			{
				if(mods.operandSizeOverride)
					return cpu.reg16(op.reg());
				else if(mods.rex.W())
					return cpu.reg64(op.reg());
				else
					return cpu.reg32(op.reg());
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
		}

		assert(false);
		return static_cast<uint64_t>(0);
	}

	void set_operand(CPU& cpu, const InstrMods& mods, const Operand& op, Value value)
	{
		if(op.isRegister())
		{
			if(cpu.mode() == CPUMode::Real)
			{
				if(mods.operandSizeOverride)
					cpu.reg32(op.reg()) = value.u32();
				else
					cpu.reg16(op.reg()) = value.u16();

				return;
			}
			else if(cpu.mode() == CPUMode::Prot || cpu.mode() == CPUMode::Long)
			{
				if(mods.operandSizeOverride)
					cpu.reg16(op.reg()) = value.u16();
				else if(mods.rex.W())
					cpu.reg64(op.reg()) = value.u64();
				else
					cpu.reg32(op.reg()) = value.u32();

				return;
			}
		}
		else if(op.isMemory())
		{
		}

		assert(false && "invalid destination operand kind");
	}
}
