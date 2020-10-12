// main.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include <stdio.h>

#include "defs.h"
#include "instrad/x86/decode.h"

#include "cpu/cpu.h"


static void dump(z86::CPU& cpu)
{
	if(cpu.mode() == z86::CPUMode::Real)
	{
		zpr::println("ax:    {4.2x}    bx:    {4.2x}", cpu.ax(), cpu.bx());
		zpr::println("cx:    {4.2x}    dx:    {4.2x}", cpu.cx(), cpu.dx());
		zpr::println("di:    {4.2x}    si:    {4.2x}", cpu.di(), cpu.si());
		zpr::println("bp:    {4.2x}    sp:    {4.2x}", cpu.bp(), cpu.sp());
		zpr::println("ip:    {4.2x}    cs:    {4.2x}", cpu.ip(), cpu.cs());
		zpr::println("ds:    {4.2x}    ss:    {4.2x}", cpu.ds(), cpu.ss());
		zpr::println("es:    {4.2x}    fs:    {4.2x}", cpu.es(), cpu.fs());
		zpr::println("gs:    {4.2x}", cpu.gs());
		zpr::println("flags: {016b}", cpu.flags().flags());
		zpr::println("           ODITSZ A P C", cpu.flags().flags());
		zpr::println("");
	}
}

// constexpr uint8_t instrs[] = { 0x66, 0x31, 0xC0, 0x66, 0x31, 0xDB, 0x66, 0xBB, 0x10, 0x27, 0x66, 0xB8, 0x60, 0xEA, 0x66, 0x01, 0xD8 };

int main(int argc, char** argv)
{
	using namespace z86;

	if(argc < 2)
		lg::fatal("z86", "no input files");

	auto [ ptr, len ] = util::readEntireFile(argv[1]);
	if(!ptr || len == 0)
		lg::fatal("z86", "invalid input");

	auto buf = instrad::Buffer(ptr, len);
	// auto buf = instrad::Buffer(instrs, sizeof(instrs));

	auto cpu = z86::CPU();
	while(buf.remaining() > 0)
	{
		auto instr = instrad::x86::read(buf, instrad::x86::ExecMode::Legacy);
		cpu.m_exec.execute(instr);
		dump(cpu);
	}
}
