// main.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include <stdio.h>

#include "defs.h"
#include "instrad/x86/decode.h"

#include "cpu/cpu.h"
#include "cpu/mem.h"


static void print_usage()
{
	zpr::println("usage: ./z86 --rom <rom> --program <program>");
	zpr::println("    --rom <rom>           mandatory: specify a path to the ROM file");
	zpr::println("    --program <program>   mandatory: specify a path to program file");
}

int main(int argc, char** argv)
{
	using namespace z86;

	if(argc < 3)
	{
		print_usage();
		exit(1);
	}

	const char* rom_path = nullptr;
	const char* prog_path = nullptr;

	for(int i = 1; i < argc; i++)
	{
		auto get_path = [&](const char** path) {
			if(i + 1 == argc)
			{
				zpr::fprintln(stderr, "expected path after '{}'", argv[i]);
				exit(1);
			}

			*path = argv[++i];
		};

		if(strcmp(argv[i], "--rom") == 0)
		{
			get_path(&rom_path);
		}
		else if(strcmp(argv[i], "--program") == 0)
		{
			get_path(&prog_path);
		}
		else
		{
			zpr::fprintln(stderr, "unknown argument '{}'", argv[i]);
			exit(1);
		}
	}

	if(rom_path == nullptr)  lg::fatal("z86", "rom missing");
	if(prog_path == nullptr) lg::fatal("z86", "program missing");

	auto [ rom_ptr, rom_len ]   = util::readEntireFile(rom_path);
	auto [ prog_ptr, prog_len ] = util::readEntireFile(prog_path);

	if(!rom_ptr || rom_len == 0)
		lg::fatal("z86", "invalid rom");

	if(!prog_ptr || prog_len == 0)
		lg::fatal("z86", "invalid program");

	auto cpu = z86::CPU();
	auto rom = new HostMmapMemoryRegion(rom_len, /* readonly: */ true);
	rom->write(0, rom_ptr, rom_len);

	cpu.memory().addRegion(PhysAddr(0xFFFF0000), rom);
	cpu.memory().write(PhysAddr(0x7C00), prog_ptr, prog_len);

	delete[] rom_ptr;
	delete[] prog_ptr;

	cpu.start();
}
