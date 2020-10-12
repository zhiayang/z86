// exec.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

#include "misc.h"
#include "instrad/x86/decode.h"

namespace z86
{
	struct CPU;

	struct Value
	{
		Value(uint8_t x)  : m_bits(8),  m_value(x) { }
		Value(uint16_t x) : m_bits(16), m_value(x) { }
		Value(uint32_t x) : m_bits(32), m_value(x) { }
		Value(uint64_t x) : m_bits(64), m_value(x) { }

		Value(int bits, uint64_t x) : m_bits(bits), m_value(x) { }

		ALWAYS_INLINE int bits()      { return this->m_bits; }

		ALWAYS_INLINE uint8_t  u8()   { return static_cast<uint8_t>(this->m_value); }
		ALWAYS_INLINE uint16_t u16()  { return static_cast<uint16_t>(this->m_value); }
		ALWAYS_INLINE uint32_t u32()  { return static_cast<uint32_t>(this->m_value); }
		ALWAYS_INLINE uint64_t u64()  { return static_cast<uint64_t>(this->m_value); }

		ALWAYS_INLINE int8_t  i8()    { return static_cast<int8_t>(this->m_value); }
		ALWAYS_INLINE int16_t i16()   { return static_cast<int16_t>(this->m_value); }
		ALWAYS_INLINE int32_t i32()   { return static_cast<int32_t>(this->m_value); }
		ALWAYS_INLINE int64_t i64()   { return static_cast<int64_t>(this->m_value); }

		ALWAYS_INLINE uint64_t get()
		{
			switch(this->m_bits)
			{
				case 8:  return this->m_value & 0xFF;
				case 16: return this->m_value & 0xFFFF;
				case 32: return this->m_value & 0xFFFF'FFFF;
				default: return this->m_value & 0xFFFF'FFFF'FFFF'FFFF;
			}
		}

		ALWAYS_INLINE bool sign()
		{
			switch(this->m_bits)
			{
				case 8:  return this->m_value & 0x80;
				case 16: return this->m_value & 0x8000;
				case 32: return this->m_value & 0x8000'0000;
				default: return this->m_value & 0x8000'0000'0000'0000;
			}
		}

		ALWAYS_INLINE bool parity()
		{
			// __builtin_parity returns 1 for odd parity, but parityflag is 1 for even parity.
			// so, invert it.
			switch(this->m_bits)
			{
				case 8:  return !__builtin_parity(this->u8());
				case 16: return !__builtin_parity(this->u16());
				case 32: return !__builtin_parity(this->u32());
				default: return !__builtin_parity(this->u64());
			}
		}

		ALWAYS_INLINE bool zero()
		{
			switch(this->m_bits)
			{
				case 8:  return this->u8() == 0;
				case 16: return this->u16() == 0;
				case 32: return this->u32() == 0;
				default: return this->u64() == 0;
			}
		}

	private:
		int m_bits = 0;
		uint64_t m_value = 0;
	};

	struct Executor
	{
		Executor(CPU& cpu) : m_cpu(cpu) { }

	private:
		CPU& m_cpu;

	public:
		void execute(const instrad::x86::Instruction& instr);
	};


	Value get_operand(CPU& cpu, const instrad::x86::InstrModifiers& mods, const instrad::x86::Operand& op);
	void set_operand(CPU& cpu, const instrad::x86::InstrModifiers& mods, const instrad::x86::Operand& op, Value value);
}
