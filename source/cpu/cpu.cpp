// cpu.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "cpu/cpu.h"

namespace z86
{
	CPU::CPU() : m_exec(*this), m_pmmu(*this, this->m_memory), m_smmu(*this, this->m_pmmu)
	{
	}






	uint8_t& CPU::reg8(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		if(reg == regs::AL)   return this->al();
		if(reg == regs::CL)   return this->cl();
		if(reg == regs::DL)   return this->dl();
		if(reg == regs::BL)   return this->bl();
		if(reg == regs::AH)   return this->ah();
		if(reg == regs::CH)   return this->ch();
		if(reg == regs::DH)   return this->dh();
		if(reg == regs::BH)   return this->bh();

		if(reg == regs::SPL)  return this->spl();
		if(reg == regs::BPL)  return this->bpl();
		if(reg == regs::SIL)  return this->sil();
		if(reg == regs::DIL)  return this->dil();
		if(reg == regs::R8B)  return this->r8b();
		if(reg == regs::R9B)  return this->r9b();
		if(reg == regs::R10B) return this->r10b();
		if(reg == regs::R11B) return this->r11b();
		if(reg == regs::R12B) return this->r12b();
		if(reg == regs::R13B) return this->r13b();
		if(reg == regs::R14B) return this->r14b();
		if(reg == regs::R15B) return this->r15b();

		assert(false && "invalid register");
		return this->al();
	}

	uint16_t& CPU::reg16(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		if(reg == regs::AX)   return this->ax();
		if(reg == regs::CX)   return this->cx();
		if(reg == regs::DX)   return this->dx();
		if(reg == regs::BX)   return this->bx();
		if(reg == regs::SP)   return this->sp();
		if(reg == regs::BP)   return this->bp();
		if(reg == regs::SI)   return this->si();
		if(reg == regs::DI)   return this->di();
		if(reg == regs::R8W)  return this->r8w();
		if(reg == regs::R9W)  return this->r9w();
		if(reg == regs::R10W) return this->r10w();
		if(reg == regs::R11W) return this->r11w();
		if(reg == regs::R12W) return this->r12w();
		if(reg == regs::R13W) return this->r13w();
		if(reg == regs::R14W) return this->r14w();
		if(reg == regs::R15W) return this->r15w();

		assert(false && "invalid register");
		return this->ax();
	}

	uint32_t& CPU::reg32(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		if(reg == regs::EAX)  return this->eax();
		if(reg == regs::ECX)  return this->ecx();
		if(reg == regs::EDX)  return this->edx();
		if(reg == regs::EBX)  return this->ebx();
		if(reg == regs::ESP)  return this->esp();
		if(reg == regs::EBP)  return this->ebp();
		if(reg == regs::ESI)  return this->esi();
		if(reg == regs::EDI)  return this->edi();
		if(reg == regs::R8D)  return this->r8d();
		if(reg == regs::R9D)  return this->r9d();
		if(reg == regs::R10D) return this->r10d();
		if(reg == regs::R11D) return this->r11d();
		if(reg == regs::R12D) return this->r12d();
		if(reg == regs::R13D) return this->r13d();
		if(reg == regs::R14D) return this->r14d();
		if(reg == regs::R15D) return this->r15d();

		assert(false && "invalid register");
		return this->eax();
	}

	uint64_t& CPU::reg64(const instrad::x86::Register& reg)
	{
		using namespace instrad::x86;

		if(reg == regs::RAX)  return this->rax();
		if(reg == regs::RCX)  return this->rcx();
		if(reg == regs::RDX)  return this->rdx();
		if(reg == regs::RBX)  return this->rbx();
		if(reg == regs::RSP)  return this->rsp();
		if(reg == regs::RBP)  return this->rbp();
		if(reg == regs::RSI)  return this->rsi();
		if(reg == regs::RDI)  return this->rdi();
		if(reg == regs::R8)   return this->r8();
		if(reg == regs::R9)   return this->r9();
		if(reg == regs::R10)  return this->r10();
		if(reg == regs::R11)  return this->r11();
		if(reg == regs::R12)  return this->r12();
		if(reg == regs::R13)  return this->r13();
		if(reg == regs::R14)  return this->r14();
		if(reg == regs::R15)  return this->r15();

		assert(false && "invalid register");
		return this->rax();
	}
}
