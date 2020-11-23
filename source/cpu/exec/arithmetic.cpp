// arithmetic.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/cpu.h"
#include "cpu/exec.h"

namespace z86
{
	using Operand = instrad::x86::Operand;
	using Instruction = instrad::x86::Instruction;
	using InstrMods = instrad::x86::InstrModifiers;

	static inline Value op_add(CPU& cpu, Value a, Value b);
	static inline Value op_sub(CPU& cpu, Value a, Value b);
	static inline Value op_adc(CPU& cpu, Value a, Value b);
	static inline Value op_sbb(CPU& cpu, Value a, Value b);
	static inline Value op_xor(CPU& cpu, Value a, Value b);
	static inline Value op_and(CPU& cpu, Value a, Value b);
	static inline Value op_or(CPU& cpu, Value a, Value b);

	static inline void set_OPZSA(CPU& cpu, Value& a, Value& b, Value& ret, bool sign, bool same_sign)
	{
		cpu.flags().setOF((ret.sign() ^ sign) & same_sign);
		cpu.flags().setPF(ret.parity());
		cpu.flags().setZF(ret.zero());
		cpu.flags().setSF(ret.sign());
		cpu.flags().setAF((a.u8() & 0xF) + (b.u8() & 0xF) > 0xF);
	}

	static inline void set_PZS(CPU& cpu, Value& ret)
	{
		cpu.flags().setPF(ret.parity());
		cpu.flags().setZF(ret.zero());
		cpu.flags().setSF(ret.sign());
	}


	void op_arithmetic(CPU& cpu, const instrad::x86::Op& op, const InstrMods& mods, const Operand& dst, const Operand& src)
	{
		using namespace instrad::x86;

		auto dst_val = get_operand(cpu, mods, dst);
		auto src_val = get_operand(cpu, mods, src);

		bool dont_set_result = false;
		Value result = static_cast<uint64_t>(0);

		switch(op.id())
		{
			using namespace ops;

			case ADD.id():  result = op_add(cpu, dst_val, src_val); break;
			case ADC.id():  result = op_adc(cpu, dst_val, src_val); break;
			case SUB.id():  result = op_sub(cpu, dst_val, src_val); break;
			case SBB.id():  result = op_sbb(cpu, dst_val, src_val); break;
			case XOR.id():  result = op_xor(cpu, dst_val, src_val); break;
			case AND.id():  result = op_and(cpu, dst_val, src_val); break;
			case OR.id():   result = op_or(cpu, dst_val, src_val); break;
			case CMP.id():  result = op_sub(cpu, dst_val, src_val); dont_set_result = true; break;
			case TEST.id(): result = op_and(cpu, dst_val, src_val); dont_set_result = true; break;
		}

		if(!dont_set_result)
			set_operand(cpu, mods, dst, result);
	}

	void op_inc_dec(CPU& cpu, const instrad::x86::Op& op, const InstrMods& mods, const Operand& dst)
	{
		using namespace instrad::x86;

		auto a = get_operand(cpu, mods, dst);
		auto b = Value(a.bits(), (op == ops::INC ? 1 : -1));

		auto same_sign  = !(a.sign() ^ b.sign());
		auto sign       = same_sign & a.sign();
		auto ret        = Value(a.bits(), a.u64() + b.u64());

		set_OPZSA(cpu, a, b, ret, sign, same_sign);
		set_operand(cpu, mods, dst, ret);
	}




	static inline Value op_add(CPU& cpu, Value a, Value b)
	{
		assert(a.bits() == b.bits());

		auto same_sign  = !(a.sign() ^ b.sign());
		auto sign       = same_sign & a.sign();
		auto ret        = Value(a.bits(), a.u64() + b.u64());

		set_OPZSA(cpu, a, b, ret, sign, same_sign);
		cpu.flags().setCF(ret.get() < a.get());
		return ret;
	}

	static inline Value op_sub(CPU& cpu, Value a, Value b)
	{
		assert(a.bits() == b.bits());

		auto same_sign  = !(a.sign() ^ b.sign());
		auto sign       = same_sign & a.sign();
		auto ret        = Value(a.bits(), a.u64() - b.u64());

		set_OPZSA(cpu, a, b, ret, sign, same_sign);
		cpu.flags().setCF(ret.get() > a.get());
		return ret;
	}

	static inline Value op_adc(CPU& cpu, Value a, Value b)
	{
		assert(a.bits() == b.bits());

		auto same_sign  = !(a.sign() ^ b.sign());
		auto sign       = same_sign & a.sign();
		auto ret        = Value(a.bits(), a.u64() + b.u64() + (cpu.flags().CF() ? 1 : 0));

		set_OPZSA(cpu, a, b, ret, sign, same_sign);
		cpu.flags().setCF((cpu.flags().CF() && (ret.get() == a.get())) || (ret.get() < a.get()));
		return ret;
	}

	static inline Value op_sbb(CPU& cpu, Value a, Value b)
	{
		assert(a.bits() == b.bits());

		auto same_sign  = !(a.sign() ^ b.sign());
		auto sign       = same_sign & a.sign();
		auto ret        = Value(a.bits(), a.u64() - b.u64() - (cpu.flags().CF() ? 1 : 0));

		set_OPZSA(cpu, a, b, ret, sign, same_sign);
		cpu.flags().setCF((cpu.flags().CF() && (ret.get() == a.get())) || (-ret.get() < -a.get()));
		return ret;
	}

	static inline Value op_and(CPU& cpu, Value a, Value b)
	{
		assert(a.bits() == b.bits());

		auto ret = Value(a.bits(), a.u64() & b.u64());

		set_PZS(cpu, ret);
		cpu.flags().clearCF();
		cpu.flags().clearOF();
		return ret;
	}

	static inline Value op_xor(CPU& cpu, Value a, Value b)
	{
		assert(a.bits() == b.bits());

		auto ret = Value(a.bits(), a.u64() ^ b.u64());

		set_PZS(cpu, ret);
		cpu.flags().clearCF();
		cpu.flags().clearOF();
		return ret;
	}

	static inline Value op_or(CPU& cpu, Value a, Value b)
	{
		assert(a.bits() == b.bits());

		auto ret = Value(a.bits(), a.u64() | b.u64());

		set_PZS(cpu, ret);
		cpu.flags().clearCF();
		cpu.flags().clearOF();
		return ret;
	}
}
