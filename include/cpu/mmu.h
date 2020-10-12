// mmu.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <cstdint>
#include <cstddef>

#include "cpu/mem.h"

namespace z86
{
	struct CPU;

	struct PagedMMU
	{
		PagedMMU(CPU& cpu, MemoryController& mem) : m_cpu(cpu), m_memcon(mem) { }

	private:
		bool m_enabled = false;

		CPU& m_cpu;
		MemoryController& m_memcon;

	public:
		PhysAddr resolve(VirtAddr addr);

		void enable();
		void disable();

		bool enabled();
	};

	struct SegmentedMMU
	{
		struct SystemDescriptor
		{
			uint16_t limit_low;
			uint16_t base_low;

			uint8_t  base_mid;
			uint8_t  access;
			uint8_t  flags;
			uint8_t  base_high;
		};

		SegmentedMMU(CPU& cpu, PagedMMU& pmmu) : m_cpu(cpu), m_pagedMMU(pmmu) { }

	private:

		CPU& m_cpu;
		PagedMMU& m_pagedMMU;

		uint64_t m_gdt_address = 0;
		uint16_t m_gdt_limit = 0;

		uint64_t m_ldt_address = 0;
		uint16_t m_ldt_limit = 0;

		SystemDescriptor m_cached_cs = { };
		SystemDescriptor m_cached_ds = { };
		SystemDescriptor m_cached_es = { };
		SystemDescriptor m_cached_fs = { };
		SystemDescriptor m_cached_gs = { };
		SystemDescriptor m_cached_ss = { };

	public:
		void loadCS(uint16_t sel);
		void loadDS(uint16_t sel);
		void loadES(uint16_t sel);
		void loadFS(uint16_t sel);
		void loadGS(uint16_t sel);
		void loadSS(uint16_t sel);

		VirtAddr resolve(SegmentedAddr addr);

		uint8_t read8(SegmentedAddr addr);
		uint16_t read16(SegmentedAddr addr);
		uint32_t read32(SegmentedAddr addr);
		uint64_t read64(SegmentedAddr addr);
		void write8(SegmentedAddr addr, uint8_t value);
		void write16(SegmentedAddr addr, uint16_t value);
		void write32(SegmentedAddr addr, uint32_t value);
		void write64(SegmentedAddr addr, uint64_t value);

		template <typename T> T read(SegmentedAddr addr);
		template <typename T> void write(SegmentedAddr addr, T value);
	};
}



/*
	a high-level overview of the memory system
	------------------------------------------
	regardless of the cpu mode, all memory accesses go through the SegmentedMMU, and all
	addresses are implicitly or explicitly tied to a segment register.

	the SegmentedMMU will first resolve the segments (if necessary), then forward the
	memory access request to the PagedMMU. again, this happens regardless of the cpu's mode. if
	paging is disabled, then there is a 1-to-1 mapping of virtual to physical addresses, and the
	PagedMMU will forward the request to the MainMemory.

	the MemoryController acts as the main memory controller of the CPU (the CPU has an onboard
	memory controller like it's 2003, Kapp). it is responsible for the overall memory mapping of
	the entire system -- eg. video memory to video devices, bios ROM areas, etc.

	internally, the MemoryController holds a list of MemoryRegions which are either backed by
	host OS RAM (via mmap or something), or mapped to various devices.

	Type safety (via SegmentedAddr, VirtAddr, and PhysAddr) is used to ensure that any and all
	memory accesses go through the appropriate MMU before being sent to the MemoryController.


	SegmentedMMU
	------------
	the main purpose of this is to resolve segmented accesses (well in actuality all accesses
	are segmented), and it is responsible for the GDT (global descriptor table) and the LDT (local
	descriptor table).
*/
