// mem.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cassert>

namespace z86
{
	enum class SegReg { CS, DS, ES, FS, GS, SS };

	struct SegmentedAddr
	{
		SegmentedAddr() : reg(SegReg::DS), offset(0) { }
		SegmentedAddr(SegReg sr, uint64_t o) : reg(sr), offset(o) { }

		SegReg reg;
		uint64_t offset;

		static SegmentedAddr cs(uint64_t ofs) { return SegmentedAddr(SegReg::CS, ofs); }
		static SegmentedAddr ds(uint64_t ofs) { return SegmentedAddr(SegReg::DS, ofs); }
		static SegmentedAddr es(uint64_t ofs) { return SegmentedAddr(SegReg::ES, ofs); }
		static SegmentedAddr fs(uint64_t ofs) { return SegmentedAddr(SegReg::FS, ofs); }
		static SegmentedAddr gs(uint64_t ofs) { return SegmentedAddr(SegReg::GS, ofs); }
		static SegmentedAddr ss(uint64_t ofs) { return SegmentedAddr(SegReg::SS, ofs); }
	};

	struct VirtAddr
	{
		VirtAddr() : addr(0) { }
		VirtAddr(uint64_t x) : addr(x) { }

		uint64_t addr = 0;
	};

	struct PhysAddr
	{
		PhysAddr() : addr(0) { }
		PhysAddr(uint64_t x) : addr(x) { }

		uint64_t addr = 0;
	};


	struct MemoryRegion
	{
		virtual ~MemoryRegion() { }
		virtual uint8_t read8(uint64_t offset) = 0;
		virtual uint16_t read16(uint64_t offset) = 0;
		virtual uint32_t read32(uint64_t offset) = 0;
		virtual uint64_t read64(uint64_t offset) = 0;
		virtual void write8(uint64_t offset, uint8_t value) = 0;
		virtual void write16(uint64_t offset, uint16_t value) = 0;
		virtual void write32(uint64_t offset, uint32_t value) = 0;
		virtual void write64(uint64_t offset, uint64_t value) = 0;

		virtual size_t size() = 0;
	};

	struct HostMmapMemoryRegion : MemoryRegion
	{
		HostMmapMemoryRegion(size_t size);
		~HostMmapMemoryRegion();

	private:
		void* m_ptr = 0;
		size_t m_size = 0;

	public:
		virtual size_t size() override { return this->m_size; }

		virtual inline uint8_t read8(uint64_t offset) override
		{
			assert(offset < this->m_size);
			return *((uint8_t*) ((uintptr_t) this->m_ptr + offset));
		}

		virtual inline uint16_t read16(uint64_t offset) override
		{
			assert(offset + 1 < this->m_size);

			uint16_t ret = 0;
			memcpy(&ret, (uint16_t*) ((uintptr_t) this->m_ptr + offset), sizeof(uint16_t));
			return ret;
		}

		virtual inline uint32_t read32(uint64_t offset) override
		{
			assert(offset + 3 < this->m_size);

			uint32_t ret = 0;
			memcpy(&ret, (uint32_t*) ((uintptr_t) this->m_ptr + offset), sizeof(uint32_t));
			return ret;
		}

		virtual inline uint64_t read64(uint64_t offset) override
		{
			assert(offset + 7 < this->m_size);

			uint64_t ret = 0;
			memcpy(&ret, (uint64_t*) ((uintptr_t) this->m_ptr + offset), sizeof(uint64_t));
			return ret;
		}

		virtual inline void write8(uint64_t offset, uint8_t value) override
		{
			assert(offset < this->m_size);
			*((uint8_t*) ((uintptr_t) this->m_ptr + offset)) = value;
		}

		virtual inline void write16(uint64_t offset, uint16_t value) override
		{
			assert(offset + 1 < this->m_size);
			memcpy((uint16_t*) ((uintptr_t) this->m_ptr + offset), &value, sizeof(uint16_t));
		}

		virtual inline void write32(uint64_t offset, uint32_t value) override
		{
			assert(offset + 3 < this->m_size);
			memcpy((uint32_t*) ((uintptr_t) this->m_ptr + offset), &value, sizeof(uint32_t));
		}

		virtual inline void write64(uint64_t offset, uint64_t value) override
		{
			assert(offset + 7 < this->m_size);
			memcpy((uint64_t*) ((uintptr_t) this->m_ptr + offset), &value, sizeof(uint64_t));
		}
	};

	struct MemoryController
	{
		MemoryController();
		~MemoryController();

		struct RegionMapping
		{
			PhysAddr start;
			size_t length;

			MemoryRegion* region;
		};

	private:
		std::vector<RegionMapping> m_regions;

	public:
		void lock();
		void unlock();

		uint8_t read8(PhysAddr addr);
		uint16_t read16(PhysAddr addr);
		uint32_t read32(PhysAddr addr);
		uint64_t read64(PhysAddr addr);
		void write8(PhysAddr addr, uint8_t value);
		void write16(PhysAddr addr, uint16_t value);
		void write32(PhysAddr addr, uint32_t value);
		void write64(PhysAddr addr, uint64_t value);

		template <typename T> T read(SegmentedAddr addr);
		template <typename T> void write(SegmentedAddr addr, T value);
	};
}
