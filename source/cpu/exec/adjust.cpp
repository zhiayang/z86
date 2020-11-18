// adjust.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "defs.h"
#include "cpu/cpu.h"

namespace z86
{
	using InstrMods = instrad::x86::InstrModifiers;

	static void set_pzs(CPU& cpu, uint8_t x)
	{
		cpu.flags().setSF(x & 0x80);
		cpu.flags().setZF(x == 0);
		cpu.flags().setPF(Value(x).parity());
	}


	void op_aaa(CPU& cpu)
	{
		if((cpu.al() & 0x0F) > 9 || cpu.flags().AF())
		{
			cpu.ax() += 0x106;

			cpu.flags().setAF();
			cpu.flags().setCF();
		}
		else
		{
			cpu.flags().clearAF();
			cpu.flags().clearCF();
		}

		cpu.al() &= 0x0F;
	}

	void op_aas(CPU& cpu)
	{
		if((cpu.al() & 0x0F) > 9 || cpu.flags().AF())
		{
			cpu.ax() -= 6;
			cpu.ah() -= 1;

			cpu.flags().setAF();
			cpu.flags().setCF();
		}
		else
		{
			cpu.flags().clearAF();
			cpu.flags().clearCF();
		}

		cpu.al() &= 0x0F;
	}

	void op_aad(CPU& cpu, uint8_t base)
	{
		auto al = cpu.al();
		auto ah = cpu.ah();

		uint8_t x = (al + (ah * base)) & 0xFF;

		cpu.al() = x;
		cpu.ah() = 0;

		set_pzs(cpu, x);
	}

	void op_aam(CPU& cpu, uint8_t base)
	{
		auto al = cpu.al();
		uint8_t x = al % base;

		cpu.ah() = al / base;
		cpu.al() = x;

		set_pzs(cpu, x);
	}

	void op_daa(CPU& cpu)
	{
		auto old_al = cpu.al();
		auto old_cf = cpu.flags().CF();
		cpu.flags().clearCF();

		if((old_al & 0x0F) > 9 || cpu.flags().AF())
		{
			cpu.al() += 6;
			cpu.flags().setCF(old_cf | ((old_al & 0x0F) > 9)); // i think?
			cpu.flags().setAF();
		}
		else
		{
			cpu.flags().clearAF();
		}

		if(old_al > 0x99 || old_cf)
		{
			cpu.al() += 0x60;
			cpu.flags().setCF();
		}
		else
		{
			cpu.flags().clearCF();
		}
	}

	void op_das(CPU& cpu)
	{
		auto old_al = cpu.al();
		auto old_cf = cpu.flags().CF();
		cpu.flags().clearCF();

		if((old_al & 0x0F) > 9 || cpu.flags().AF())
		{
			cpu.al() -= 6;
			cpu.flags().setCF(old_cf | ((old_al & 0x0F) < 6)); // i think?
			cpu.flags().setAF();
		}
		else
		{
			cpu.flags().clearAF();
		}

		if(old_al > 0x99 || old_cf)
		{
			cpu.al() -= 0x60;
			cpu.flags().setCF();
		}
	}
}
