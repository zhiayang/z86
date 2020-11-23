// jump.cpp
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

	static void do_jump(CPU& cpu, const Operand& dst)
	{
		// check for far offsets
		if(dst.isFarOffset())
		{
			uint16_t seg = 0;
			uint64_t ofs = 0;

			auto& far = dst.far();
			if(far.isMemory())
			{
				auto& mem = far.memory();
				auto [ segreg, ptr ] = resolve_memory_access(cpu, mem);

				// the ptr comes first
				switch(mem.bits())
				{
					case 16: ofs = cpu.read16(segreg, ptr); break;
					case 32: ofs = cpu.read32(segreg, ptr); break;
					case 64: ofs = cpu.read64(segreg, ptr); break;
					default: assert(false && "invalid operand size");
				}

				// then the segment offset.
				seg = cpu.read16(segreg, ptr + (mem.bits() / 8));
			}
			else
			{
				seg = far.segment();
				ofs = far.offset();
			}

			if(cpu.mode() == CPUMode::Real)
			{
				// load CS, then jump.
				cpu.cs() = seg;
				cpu.jump(static_cast<uint16_t>(ofs));
			}
			else if(cpu.mode() == CPUMode::Prot)
			{
				// owo
				assert(false && "unsupported");
			}
			else if(cpu.mode() == CPUMode::Long)
			{
				// owo
				assert(false && "unsupported");
			}
			else
			{
				assert(false && "invalid cpu mode");
			}
		}
		else
		{
			assert(dst.isRelativeOffset());

			auto ofs = static_cast<int64_t>(dst.ofs().offset());
			cpu.jump(cpu.ip() + ofs);
		}
	}

	void op_call(CPU& cpu, const InstrMods& mods, const Operand& dst)
	{
		if(dst.isFarOffset())
		{
			assert(cpu.mode() == CPUMode::Real);

			uint16_t seg = 0;
			uint64_t ofs = 0;


			switch(get_operand_size(cpu, mods))
			{
				case 16: cpu.push16(cpu.cs()); cpu.push16(cpu.ip()); break;
				case 32: cpu.push32(cpu.cs()); cpu.push32(cpu.ip()); break;
				case 64: cpu.push64(cpu.cs()); cpu.push64(cpu.ip()); break;
			}

			auto& far = dst.far();
			if(far.isMemory())
			{
				auto [ segreg, ptr ] = resolve_memory_access(cpu, far.memory());
				switch(get_operand_size(cpu, mods))
				{
					case 16: {
						ofs = cpu.read16(segreg, ptr);
						seg = cpu.read16(segreg, ptr + 2);
					} break;

					case 32: {
						ofs = cpu.read32(segreg, ptr);
						seg = cpu.read16(segreg, ptr + 4);

						assert((ofs & 0xFFFF0000) == 0);
					}
				}
			}
			else
			{
				seg = far.segment();
				ofs = far.offset();
			}

			if(cpu.mode() == CPUMode::Real)
			{
				// load CS, then jump.
				cpu.cs() = seg;
				cpu.jump(static_cast<uint16_t>(ofs));
			}
			else if(cpu.mode() == CPUMode::Prot)
			{
				// owo
				assert(false && "unsupported");
			}
			else if(cpu.mode() == CPUMode::Long)
			{
				// owo
				assert(false && "unsupported");
			}
			else
			{
				assert(false && "invalid cpu mode");
			}
		}
		else
		{
			auto old_ip = cpu.ip();
			if(dst.isRelativeOffset())
			{
				auto ofs = static_cast<int64_t>(dst.ofs().offset());
				cpu.jump(cpu.ip() + ofs);
			}
			else
			{
				// absolute address.
				auto ip = get_operand(cpu, mods, dst);
				cpu.jump(ip.u64());
			}

			// now push the old IP, depending on size.
			switch(get_operand_size(cpu, mods))
			{
				case 16: cpu.push16(static_cast<uint16_t>(old_ip)); break;
				case 32: cpu.push32(static_cast<uint32_t>(old_ip)); break;
				case 64: cpu.push64(static_cast<uint64_t>(old_ip)); break;
			}
		}
	}

	void op_retf(CPU& cpu, const Instruction& instr)
	{
		assert(false);
	}

	void op_ret(CPU& cpu, const Instruction& instr)
	{
		switch(get_operand_size(cpu, instr.mods()))
		{
			case 16: cpu.jump(cpu.pop16()); break;
			case 32: cpu.jump(cpu.pop32()); break;
			case 64: cpu.jump(cpu.pop64()); break;
			default: assert(false);
		}
	}













	void op_jmp(CPU& cpu, const Operand& dst)
	{
		do_jump(cpu, dst);
	}

	void op_jo(CPU& cpu, const Operand& dst, bool check)
	{
		if(cpu.flags().OF() == check)
			do_jump(cpu, dst);
	}

	void op_js(CPU& cpu, const Operand& dst, bool check)
	{
		if(cpu.flags().SF() == check)
			do_jump(cpu, dst);
	}

	void op_jz(CPU& cpu, const Operand& dst, bool check)
	{
		if(cpu.flags().ZF() == check)
			do_jump(cpu, dst);
	}

	void op_jc(CPU& cpu, const Operand& dst, bool check)
	{
		if(cpu.flags().CF() == check)
			do_jump(cpu, dst);
	}

	void op_jp(CPU& cpu, const Operand& dst, bool check)
	{
		if(cpu.flags().PF() == check)
			do_jump(cpu, dst);
	}

	void op_ja(CPU& cpu, const Operand& dst, bool check)
	{
		if(check && !(cpu.flags().CF() | cpu.flags().ZF()))
			do_jump(cpu, dst);

		else if(!check && (cpu.flags().CF() | cpu.flags().ZF()))
			do_jump(cpu, dst);
	}

	void op_jl(CPU& cpu, const Operand& dst, bool check)
	{
		if(check && cpu.flags().SF() != cpu.flags().OF())
			do_jump(cpu, dst);

		else if(!check && cpu.flags().SF() == cpu.flags().OF())
			do_jump(cpu, dst);
	}

	void op_jg(CPU& cpu, const Operand& dst, bool check)
	{
		if(check && !cpu.flags().ZF() && cpu.flags().SF() == cpu.flags().OF())
			do_jump(cpu, dst);

		else if(!check && cpu.flags().ZF() && cpu.flags().SF() != cpu.flags().OF())
			do_jump(cpu, dst);
	}

	void op_jcxz(CPU& cpu, const InstrMods& mods, const Operand& dst)
	{
		switch(get_address_size(cpu, mods))
		{
			case 16: if(cpu.cx() == 0)  do_jump(cpu, dst); break;
			case 32: if(cpu.ecx() == 0) do_jump(cpu, dst); break;
			case 64: if(cpu.rcx() == 0) do_jump(cpu, dst); break;
		}
	}
}
