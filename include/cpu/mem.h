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
		MemoryRegion(size_t sz) : m_size(sz) { }
		virtual ~MemoryRegion() { }

		size_t size() { return m_size; }

		uint8_t read8(uint64_t offset)                  { return this->internal_read<uint8_t>(offset); }
		uint16_t read16(uint64_t offset)                { return this->internal_read<uint16_t>(offset); }
		uint32_t read32(uint64_t offset)                { return this->internal_read<uint32_t>(offset); }
		uint64_t read64(uint64_t offset)                { return this->internal_read<uint64_t>(offset); }

		void write8(uint64_t offset, uint8_t value)     { this->internal_write(offset, value); }
		void write16(uint64_t offset, uint16_t value)   { this->internal_write(offset, value); }
		void write32(uint64_t offset, uint32_t value)   { this->internal_write(offset, value); }
		void write64(uint64_t offset, uint64_t value)   { this->internal_write(offset, value); }

		virtual void read(uint64_t offset, void* buf, size_t len) = 0;
		virtual void write(uint64_t offset, const void* buf, size_t len) = 0;

	protected:
		size_t m_size;

	private:

		template <typename T>
		void internal_write(uint64_t offset, T value)
		{
			this->write(offset, &value, sizeof(value));
		}

		template <typename T>
		T internal_read(uint64_t offset)
		{
			T value = 0;
			this->read(offset, &value, sizeof(T));
			return value;
		}
	};

	struct HostMmapMemoryRegion : MemoryRegion
	{
		HostMmapMemoryRegion(size_t size, bool writable);
		~HostMmapMemoryRegion();

	private:
		uint8_t* m_ptr = 0;
		bool m_writable = false;

	public:
		virtual void read(uint64_t offset, void* buf, size_t len) override
		{
			assert(offset + len <= m_size);
			memcpy(buf, m_ptr + offset, len);
		}

		virtual void write(uint64_t offset, const void* buf, size_t len) override
		{
			assert(offset + len <= m_size);
			assert(m_writable);

			memcpy(m_ptr + offset, buf, len);
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
		void addRegion(PhysAddr start, MemoryRegion* region);

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

		void read(PhysAddr addr, uint8_t* buf, size_t len);
		void write(PhysAddr addr, const uint8_t* buf, size_t len);
	};
}
