// cpu.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

#include "misc.h"

#include "mmu.h"
#include "exec.h"

namespace z86
{
	struct GeneralPurposeReg
	{
		union {
			uint64_t low_64;

			struct {
				uint32_t low_32;
				uint32_t high_32;
			};

			struct {
				uint16_t low_16;
				uint16_t high_16;

				uint32_t __low_32;
			};

			struct {
				uint8_t low_8;
				uint8_t high_8;

				uint16_t __mid_16;
				uint32_t __low_32_1;
			};
		};
	};


	struct FlagsReg
	{
		// why are these uppercase? because lowercase 'if' is a keyword, and it upsets me.
		ALWAYS_INLINE bool CF() const { return (this->m_rflags & 0x001); }
		ALWAYS_INLINE bool PF() const { return (this->m_rflags & 0x004); }
		ALWAYS_INLINE bool AF() const { return (this->m_rflags & 0x010); }
		ALWAYS_INLINE bool ZF() const { return (this->m_rflags & 0x040); }
		ALWAYS_INLINE bool SF() const { return (this->m_rflags & 0x080); }
		ALWAYS_INLINE bool TF() const { return (this->m_rflags & 0x100); }
		ALWAYS_INLINE bool IF() const { return (this->m_rflags & 0x200); }
		ALWAYS_INLINE bool DF() const { return (this->m_rflags & 0x400); }
		ALWAYS_INLINE bool OF() const { return (this->m_rflags & 0x800); }

		ALWAYS_INLINE void setCF(bool x = true)   { (x ? this->m_rflags |= 0x001 : this->m_rflags &= ~0x001); }
		ALWAYS_INLINE void setPF(bool x = true)   { (x ? this->m_rflags |= 0x004 : this->m_rflags &= ~0x004); }
		ALWAYS_INLINE void setAF(bool x = true)   { (x ? this->m_rflags |= 0x010 : this->m_rflags &= ~0x010); }
		ALWAYS_INLINE void setZF(bool x = true)   { (x ? this->m_rflags |= 0x040 : this->m_rflags &= ~0x040); }
		ALWAYS_INLINE void setSF(bool x = true)   { (x ? this->m_rflags |= 0x080 : this->m_rflags &= ~0x080); }
		ALWAYS_INLINE void setTF(bool x = true)   { (x ? this->m_rflags |= 0x100 : this->m_rflags &= ~0x100); }
		ALWAYS_INLINE void setIF(bool x = true)   { (x ? this->m_rflags |= 0x200 : this->m_rflags &= ~0x200); }
		ALWAYS_INLINE void setDF(bool x = true)   { (x ? this->m_rflags |= 0x400 : this->m_rflags &= ~0x400); }
		ALWAYS_INLINE void setOF(bool x = true)   { (x ? this->m_rflags |= 0x800 : this->m_rflags &= ~0x800); }

		ALWAYS_INLINE void clearCF()  { this->setCF(false); }
		ALWAYS_INLINE void clearPF()  { this->setPF(false); }
		ALWAYS_INLINE void clearAF()  { this->setAF(false); }
		ALWAYS_INLINE void clearZF()  { this->setZF(false); }
		ALWAYS_INLINE void clearSF()  { this->setSF(false); }
		ALWAYS_INLINE void clearTF()  { this->setTF(false); }
		ALWAYS_INLINE void clearIF()  { this->setIF(false); }
		ALWAYS_INLINE void clearDF()  { this->setDF(false); }
		ALWAYS_INLINE void clearOF()  { this->setOF(false); }

		ALWAYS_INLINE void clear()    { this->m_rflags = 0; }

		// bit 1 is supposed to be always 1.
		ALWAYS_INLINE uint16_t flags() const { return this->m_flags  | 0x2; }
		ALWAYS_INLINE uint32_t eflags() const { return this->m_eflags | 0x2; }
		ALWAYS_INLINE uint64_t rflags() const { return this->m_rflags | 0x2; }

	private:
		union {
			uint64_t m_rflags = 0;

			struct {
				uint32_t m_eflags;
				uint32_t __dummy_1;
			};

			struct {
				uint16_t m_flags;
				uint16_t __dummy_2;
				uint32_t __dummy_3;
			};
		};
	};

	static_assert(sizeof(FlagsReg) == 8);
	static_assert(sizeof(GeneralPurposeReg) == 8);

	enum class CPUMode
	{
		Real,
		Prot,
		Long,
	};

	template <typename T>
	struct RegWrapper
	{
		ALWAYS_INLINE RegWrapper(T& x) : cpu(nullptr), idx(0), x(x), fn([](CPU*, short, T){}) { }
		ALWAYS_INLINE RegWrapper(CPU* cpu, short idx, T& x, void (*fn)(CPU*, short, T))
			: cpu(cpu), idx(idx), x(x), fn(fn) { }

		ALWAYS_INLINE T& operator=(T v) { x = v; fn(cpu, idx, v); return x; }
		ALWAYS_INLINE operator T() { return x; }
		ALWAYS_INLINE T get() { return x; }
		ALWAYS_INLINE operator Value() { return Value(x); }

	private:
		CPU* cpu;
		short idx;
		T& x;
		void (*fn)(CPU*, short, T);
	};

	struct CPU
	{
		CPU();

	private:
		// order is in terms of "standard" indices:
		// A,  C,  D,   B,   SP,  BP,  SI,  DI
		// R8, R9, R10, R11, R12, R13, R14, R15
		GeneralPurposeReg m_gprs[16] = { };

		// CS, DS, ES, FS, GS, SS
		uint16_t m_segment_regs[6] = { };

		// instruction pointer
		uint64_t m_ip = 0;

		// flags register
		FlagsReg m_flags = { };

		CPUMode m_mode = CPUMode::Real;

		// internal helpers
	public:
		Executor m_exec;
	private:
		MemoryController m_memory;
		PagedMMU m_pmmu;
		SegmentedMMU m_smmu;

		static constexpr size_t IDX_A   = 0;
		static constexpr size_t IDX_C   = 1;
		static constexpr size_t IDX_D   = 2;
		static constexpr size_t IDX_B   = 3;
		static constexpr size_t IDX_SP  = 4;
		static constexpr size_t IDX_BP  = 5;
		static constexpr size_t IDX_SI  = 6;
		static constexpr size_t IDX_DI  = 7;
		static constexpr size_t IDX_R8  = 8;
		static constexpr size_t IDX_R9  = 9;
		static constexpr size_t IDX_R10 = 10;
		static constexpr size_t IDX_R11 = 11;
		static constexpr size_t IDX_R12 = 12;
		static constexpr size_t IDX_R13 = 13;
		static constexpr size_t IDX_R14 = 14;
		static constexpr size_t IDX_R15 = 15;

		static constexpr size_t IDX_CS = 0;
		static constexpr size_t IDX_DS = 1;
		static constexpr size_t IDX_ES = 2;
		static constexpr size_t IDX_FS = 3;
		static constexpr size_t IDX_GS = 4;
		static constexpr size_t IDX_SS = 5;

		instrad::x86::Instruction decode();
		bool run(instrad::x86::Instruction instr);

	public:
		void memLock();
		void memUnlock();

		uint8_t read8(uint64_t address);
		uint16_t read16(uint64_t address);
		uint32_t read32(uint64_t address);
		uint64_t read64(uint64_t address);

		uint8_t read8(SegReg seg, uint64_t address);
		uint16_t read16(SegReg seg, uint64_t address);
		uint32_t read32(SegReg seg, uint64_t address);
		uint64_t read64(SegReg seg, uint64_t address);

		void write8(SegReg seg, uint64_t address, uint8_t value);
		void write16(SegReg seg, uint64_t address, uint16_t value);
		void write32(SegReg seg, uint64_t address, uint32_t value);
		void write64(SegReg seg, uint64_t address, uint64_t value);

		CPUMode mode() const { return this->m_mode; }
		bool isProtected() const { return this->m_mode >= CPUMode::Prot; }

		// A, C, D, B, SP, BP, etc.
		RegWrapper<uint8_t> reg8(const instrad::x86::Register& reg);
		RegWrapper<uint16_t> reg16(const instrad::x86::Register& reg);
		RegWrapper<uint32_t> reg32(const instrad::x86::Register& reg);
		RegWrapper<uint64_t> reg64(const instrad::x86::Register& reg);

		void start();
		void reset();
		void jump(uint64_t ip);

		MemoryController& memory() { return m_memory; }
		SegmentedMMU& smmu() { return m_smmu; }

		// accessor spam.
		// flags register
		inline FlagsReg flags() const   { return this->m_flags; }
		inline FlagsReg& flags()        { return this->m_flags; }

		// special
		inline uint64_t ip() const
		{
			if(m_mode == CPUMode::Real) return static_cast<uint16_t>(this->m_ip);
			if(m_mode == CPUMode::Prot) return static_cast<uint32_t>(this->m_ip);
			if(m_mode == CPUMode::Long) return static_cast<uint64_t>(this->m_ip);

			return 0;
		}

		// segment registers
		RegWrapper<uint16_t> cs();
		RegWrapper<uint16_t> ds();
		RegWrapper<uint16_t> es();
		RegWrapper<uint16_t> fs();
		RegWrapper<uint16_t> gs();
		RegWrapper<uint16_t> ss();

		// GPRS
		// 8-bit registers (high)
		inline uint8_t& ah()            { return this->m_gprs[IDX_A].high_8; }
		inline uint8_t& ch()            { return this->m_gprs[IDX_C].high_8; }
		inline uint8_t& dh()            { return this->m_gprs[IDX_D].high_8; }
		inline uint8_t& bh()            { return this->m_gprs[IDX_B].high_8; }

		// 8-bit registers (low)
		inline uint8_t& al()            { return this->m_gprs[IDX_A].low_8; }
		inline uint8_t& cl()            { return this->m_gprs[IDX_C].low_8; }
		inline uint8_t& dl()            { return this->m_gprs[IDX_D].low_8; }
		inline uint8_t& bl()            { return this->m_gprs[IDX_B].low_8; }
		inline uint8_t& spl()           { return this->m_gprs[IDX_SP].low_8; }
		inline uint8_t& bpl()           { return this->m_gprs[IDX_BP].low_8; }
		inline uint8_t& sil()           { return this->m_gprs[IDX_SI].low_8; }
		inline uint8_t& dil()           { return this->m_gprs[IDX_DI].low_8; }
		inline uint8_t& r8b()           { return this->m_gprs[IDX_R8].low_8; }
		inline uint8_t& r9b()           { return this->m_gprs[IDX_R9].low_8; }
		inline uint8_t& r10b()          { return this->m_gprs[IDX_R10].low_8; }
		inline uint8_t& r11b()          { return this->m_gprs[IDX_R11].low_8; }
		inline uint8_t& r12b()          { return this->m_gprs[IDX_R12].low_8; }
		inline uint8_t& r13b()          { return this->m_gprs[IDX_R13].low_8; }
		inline uint8_t& r14b()          { return this->m_gprs[IDX_R14].low_8; }
		inline uint8_t& r15b()          { return this->m_gprs[IDX_R15].low_8; }

		// 16-bit registers
		inline uint16_t& ax()           { return this->m_gprs[IDX_A].low_16; }
		inline uint16_t& cx()           { return this->m_gprs[IDX_C].low_16; }
		inline uint16_t& dx()           { return this->m_gprs[IDX_D].low_16; }
		inline uint16_t& bx()           { return this->m_gprs[IDX_B].low_16; }
		inline uint16_t& sp()           { return this->m_gprs[IDX_SP].low_16; }
		inline uint16_t& bp()           { return this->m_gprs[IDX_BP].low_16; }
		inline uint16_t& si()           { return this->m_gprs[IDX_SI].low_16; }
		inline uint16_t& di()           { return this->m_gprs[IDX_DI].low_16; }
		inline uint16_t& r8w()          { return this->m_gprs[IDX_R8].low_16; }
		inline uint16_t& r9w()          { return this->m_gprs[IDX_R9].low_16; }
		inline uint16_t& r10w()         { return this->m_gprs[IDX_R10].low_16; }
		inline uint16_t& r11w()         { return this->m_gprs[IDX_R11].low_16; }
		inline uint16_t& r12w()         { return this->m_gprs[IDX_R12].low_16; }
		inline uint16_t& r13w()         { return this->m_gprs[IDX_R13].low_16; }
		inline uint16_t& r14w()         { return this->m_gprs[IDX_R14].low_16; }
		inline uint16_t& r15w()         { return this->m_gprs[IDX_R15].low_16; }

		// 32-bit registers
		inline uint32_t& eax()          { return this->m_gprs[IDX_A].low_32; }
		inline uint32_t& ecx()          { return this->m_gprs[IDX_C].low_32; }
		inline uint32_t& edx()          { return this->m_gprs[IDX_D].low_32; }
		inline uint32_t& ebx()          { return this->m_gprs[IDX_B].low_32; }
		inline uint32_t& esp()          { return this->m_gprs[IDX_SP].low_32; }
		inline uint32_t& ebp()          { return this->m_gprs[IDX_BP].low_32; }
		inline uint32_t& esi()          { return this->m_gprs[IDX_SI].low_32; }
		inline uint32_t& edi()          { return this->m_gprs[IDX_DI].low_32; }
		inline uint32_t& r8d()          { return this->m_gprs[IDX_R8].low_32; }
		inline uint32_t& r9d()          { return this->m_gprs[IDX_R9].low_32; }
		inline uint32_t& r10d()         { return this->m_gprs[IDX_R10].low_32; }
		inline uint32_t& r11d()         { return this->m_gprs[IDX_R11].low_32; }
		inline uint32_t& r12d()         { return this->m_gprs[IDX_R12].low_32; }
		inline uint32_t& r13d()         { return this->m_gprs[IDX_R13].low_32; }
		inline uint32_t& r14d()         { return this->m_gprs[IDX_R14].low_32; }
		inline uint32_t& r15d()         { return this->m_gprs[IDX_R15].low_32; }

		// 64-bit registers
		inline uint64_t& rax()          { return this->m_gprs[IDX_A].low_64; }
		inline uint64_t& rcx()          { return this->m_gprs[IDX_C].low_64; }
		inline uint64_t& rdx()          { return this->m_gprs[IDX_D].low_64; }
		inline uint64_t& rbx()          { return this->m_gprs[IDX_B].low_64; }
		inline uint64_t& rsp()          { return this->m_gprs[IDX_SP].low_64; }
		inline uint64_t& rbp()          { return this->m_gprs[IDX_BP].low_64; }
		inline uint64_t& rsi()          { return this->m_gprs[IDX_SI].low_64; }
		inline uint64_t& rdi()          { return this->m_gprs[IDX_DI].low_64; }
		inline uint64_t& r8()           { return this->m_gprs[IDX_R8].low_64; }
		inline uint64_t& r9()           { return this->m_gprs[IDX_R9].low_64; }
		inline uint64_t& r10()          { return this->m_gprs[IDX_R10].low_64; }
		inline uint64_t& r11()          { return this->m_gprs[IDX_R11].low_64; }
		inline uint64_t& r12()          { return this->m_gprs[IDX_R12].low_64; }
		inline uint64_t& r13()          { return this->m_gprs[IDX_R13].low_64; }
		inline uint64_t& r14()          { return this->m_gprs[IDX_R14].low_64; }
		inline uint64_t& r15()          { return this->m_gprs[IDX_R15].low_64; }
	};


	// implements the Buffer interface as specified in instrad/Buffer.h
	struct Buffer
	{
		Buffer(CPU& cpu) : m_idx(0), m_cpu(cpu) { }

		size_t position() const { return m_idx; }

		uint8_t peek() const
		{
			return m_cpu.read8(SegReg::CS, m_cpu.ip() + m_idx);
		}

		uint8_t pop()
		{
			auto ret = this->peek();
			m_idx++;
			return ret;
		}

		bool match(uint8_t b)
		{
			if(this->peek() == b)
			{
				m_idx++;
				return true;
			}

			return false;
		}

	private:
		size_t m_idx;
		CPU& m_cpu;
	};

}
