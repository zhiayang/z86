// disasm.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include "defs.h"
#include "instrad/x86/decode.h"

namespace z86
{
	std::string print_att(const instrad::x86::Instruction& instr, uint64_t ip, size_t marginSz, size_t maxBytes)
	{
		std::string instr_prefix = "";
		std::string instr_suffix = "";

		auto print_operand = [&](const instrad::x86::Operand& op) -> std::string {

			auto print_memory = [&instr, &instr_suffix](const instrad::x86::MemoryRef& mem) -> std::string {

				auto& base = mem.base();
				auto& idx = mem.index();

				if(instr.op().has_suffix() && instr_suffix.empty())
				{
					switch(mem.bits())
					{
						case 8:     instr_suffix = "b"; break;
						case 16:    instr_suffix = "w"; break;
						case 32:    instr_suffix = "l"; break;
						case 64:    instr_suffix = "q"; break;

						// there should be no ambiguity in these cases,
						// so in theory we should not need a suffix.
						// (either way, idk what the suffixes would be,
						// and there don't seem to be any defined)
						default:
							break;
					}
				}

				if(mem.isDisplacement64Bits())
					instr_suffix = "abs" + instr_suffix;

				std::string segment = mem.segment().present()
					? zpr::sprint("%{}:", mem.segment().name())
					: "";

				// you can't scale a displacement, so we're fine here.
				if(!base.present() && !idx.present())
					return segment + zpr::sprint("{#x}", mem.displacement());

				std::string tmp = segment;
				if(mem.displacement() != 0)
					tmp += zpr::sprint("{#x}", mem.displacement());

				tmp += "(";

				if(base.present())
					tmp += zpr::sprint("%{}", base.name());

				if(idx.present())
					tmp += zpr::sprint(", %{}", idx.name());

				if(mem.scale() != 1)
					tmp += zpr::sprint(", {}", mem.scale());

				tmp += ")";
				return tmp;
			};

			if(op.isRegister())
			{
				return zpr::sprint("%{}", op.reg().name());
			}
			else if(op.isImmediate())
			{
				int64_t value = op.imm();
				if(op.immediateSize() == 8)
					value = (uint8_t) value;

				if(op.immediateSize() == 16)
					value = (uint16_t) value;

				if(op.immediateSize() == 32)
					value = (uint32_t) value;

				if(op.immediateSize() == 64)
					value = (uint64_t) value;

				// printf("[%#lx]", op.imm());
				return zpr::sprint("${#x}", value);
			}
			else if(op.isRelativeOffset())
			{
				return zpr::sprint("{#x}", ip + instr.length() + op.ofs().offset());
			}
			else if(op.isFarOffset())
			{
				// ljmp, lcall
				instr_prefix = "l";

				auto& far = op.far();
				if(far.isMemory())
				{
					return zpr::sprint("{}", print_memory(far.memory()));
				}
				else
				{
					return zpr::sprint("{#4.2x}:{#x}", far.segment(), far.offset());
				}
			}
			else if(op.isMemory())
			{
				return print_memory(op.mem());
			}

			return "??";
		};

		auto margin = std::string(marginSz, ' ');

		// print the bytes
		size_t col = 0;
		// for(size_t i = 0; i < instr.length(); i++)
		// {
		// 	margin += zpr::sprint("{02x} ", instr.bytes()[i]);
		// 	col += 3;
		// }

		auto max = 2 * maxBytes + marginSz;

		if(col < max)
			margin += zpr::sprint("{}", zpr::w(max - col)(""));

		std::string prefix = "";
		if(instr.lockPrefix())  prefix = "lock ";
		if(instr.repPrefix())   prefix = "rep ";
		if(instr.repnzPrefix()) prefix = "repnz ";


		// only print the instruction last, because we need to parse the operand to know the suffix.

		std::string operands;
		if(instr.operandCount() == 0)
		{
		}
		else if(instr.operandCount() == 1)
		{
			operands += print_operand(instr.dst());
		}
		else if(instr.operandCount() == 2)
		{
			operands += print_operand(instr.src()); operands += ", ";
			operands += print_operand(instr.dst());
		}
		else if(instr.operandCount() == 3)
		{
			operands += print_operand(instr.ext()); operands += ", ";
			operands += print_operand(instr.src()); operands += ", ";
			operands += print_operand(instr.dst());
		}
		else if(instr.operandCount() == 4)
		{
			operands += print_operand(instr.op4()); operands += ", ";
			operands += print_operand(instr.ext()); operands += ", ";
			operands += print_operand(instr.src()); operands += ", ";
			operands += print_operand(instr.dst());
		}

		auto istr = prefix + instr_prefix + std::string(instr.op().mnemonic()) + instr_suffix;

		return zpr::sprint("{}{-10} {}", margin, istr, operands);
	}




	std::string print_intel(const instrad::x86::Instruction& instr, uint64_t ip, size_t marginSz, size_t maxBytes)
	{
		auto print_operand = [&](const instrad::x86::Operand& op) -> std::string {

			auto print_memory = [](const instrad::x86::MemoryRef& mem) -> std::string {

				auto& base = mem.base();
				auto& idx = mem.index();

				std::string sz;

				switch(mem.bits())
				{
					case 8:     sz = "byte"; break;
					case 16:    sz = "word"; break;
					case 32:    sz = "dword"; break;
					case 64:    sz = "qword"; break;

					// there should be no ambiguity in these cases,
					// so in theory we should not need a suffix.
					// (either way, idk what the suffixes would be,
					// and there don't seem to be any defined)
					default:
						break;
				}

				auto segment = sz + (mem.segment().present()
					? zpr::sprint("{}:", mem.segment().name())
					: "");

				// you can't scale a displacement, so we're fine here.
				if(!base.present() && !idx.present())
					return segment + zpr::sprint("[{#x}]", mem.displacement());

				auto tmp = segment + "[";

				if(base.present())
					tmp += zpr::sprint("{}", base.name());

				if(idx.present())
					tmp += zpr::sprint(" + {}", idx.name());

				if(mem.scale() != 1)
					tmp += zpr::sprint(" * {}", mem.scale());

				if(mem.displacement() != 0)
					tmp += zpr::sprint(" + {#x}", mem.displacement());

				tmp += "]";
				return tmp;
			};

			if(op.isRegister())
			{
				return zpr::sprint("{}", op.reg().name());
			}
			else if(op.isImmediate())
			{
				int64_t value = op.imm();
				if(op.immediateSize() == 8)
					value = (uint8_t) value;

				if(op.immediateSize() == 16)
					value = (uint16_t) value;

				if(op.immediateSize() == 32)
					value = (uint32_t) value;

				if(op.immediateSize() == 64)
					value = (uint64_t) value;

				// printf("[%#lx]", op.imm());
				return zpr::sprint("{#x}", value);
			}
			else if(op.isRelativeOffset())
			{
				return zpr::sprint("{#x}", ip + instr.length() + op.ofs().offset());
			}
			else if(op.isFarOffset())
			{
				auto& far = op.far();
				if(far.isMemory())
				{
					return zpr::sprint("far {}", print_memory(far.memory()));
				}
				else
				{
					return zpr::sprint("far {#4.2x}:{#x}", far.segment(), far.offset());
				}
			}
			else if(op.isMemory())
			{
				return print_memory(op.mem());
			}

			return "??";
		};

		auto margin = std::string(marginSz, ' ');

		// print the bytes
		size_t col = 0;
		// for(size_t i = 0; i < instr.length(); i++)
		// {
		// 	margin += zpr::sprint("{02x} ", instr.bytes()[i]);
		// 	col += 3;
		// }

		auto max = 2 * maxBytes + marginSz;

		if(col < max)
			margin += zpr::sprint("{}", zpr::w(max - col)(""));

		std::string prefix = "";
		if(instr.lockPrefix())  prefix = "lock ";
		if(instr.repPrefix())   prefix = "rep ";
		if(instr.repnzPrefix()) prefix = "repnz ";


		// only print the instruction last, because we need to parse the operand to know the suffix.

		std::string operands;
		if(instr.operandCount() == 0)
		{
		}
		else if(instr.operandCount() == 1)
		{
			operands += print_operand(instr.dst());
		}
		else if(instr.operandCount() == 2)
		{
			operands += print_operand(instr.dst()); operands += ", ";
			operands += print_operand(instr.src());
		}
		else if(instr.operandCount() == 3)
		{
			operands += print_operand(instr.dst()); operands += ", ";
			operands += print_operand(instr.src()); operands += ", ";
			operands += print_operand(instr.ext());
		}
		else if(instr.operandCount() == 4)
		{
			operands += print_operand(instr.dst()); operands += ", ";
			operands += print_operand(instr.src()); operands += ", ";
			operands += print_operand(instr.ext()); operands += ", ";
			operands += print_operand(instr.op4());
		}

		auto istr = prefix + std::string(instr.op().mnemonic());

		return zpr::sprint("{}{-10} {}", margin, istr, operands);
	}
}


