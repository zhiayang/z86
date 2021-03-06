// immediates.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../buffer.h"

namespace instrad::x86
{
	template <typename Buffer>
	constexpr int8_t readSignedImm8(Buffer& buf)
	{
		return (int8_t) buf.pop();
	}

	template <typename Buffer>
	constexpr int16_t readSignedImm16(Buffer& buf)
	{
		uint16_t a = buf.pop();
		uint16_t b = buf.pop();

		return (int16_t) ((a << 0) | (b << 8));
	}

	template <typename Buffer>
	constexpr int32_t readSignedImm32(Buffer& buf)
	{
		uint32_t a = buf.pop();
		uint32_t b = buf.pop();
		uint32_t c = buf.pop();
		uint32_t d = buf.pop();
		return (int32_t) ((a << 0) | (b << 8) | (c << 16) | (d << 24));
	}

	template <typename Buffer>
	constexpr int64_t readSignedImm64(Buffer& buf)
	{
		uint64_t a = buf.pop();
		uint64_t b = buf.pop();
		uint64_t c = buf.pop();
		uint64_t d = buf.pop();
		uint64_t e = buf.pop();
		uint64_t f = buf.pop();
		uint64_t g = buf.pop();
		uint64_t h = buf.pop();
		return (int64_t) ((a << 0) | (b << 8) | (c << 16) | (d << 24) | (e << 32) | (f << 40) | (g << 48) | (h << 56));
	}

	template <typename Buffer>
	constexpr uint8_t readUnsignedImm8(Buffer& buf)
	{
		return static_cast<uint8_t>(readSignedImm8(buf));
	}

	template <typename Buffer>
	constexpr uint16_t readUnsignedImm16(Buffer& buf)
	{
		return static_cast<uint16_t>(readSignedImm16(buf));
	}

	template <typename Buffer>
	constexpr uint32_t readUnsignedImm32(Buffer& buf)
	{
		return static_cast<uint32_t>(readSignedImm32(buf));
	}

	template <typename Buffer>
	constexpr uint64_t readUnsignedImm64(Buffer& buf)
	{
		return static_cast<uint64_t>(readSignedImm64(buf));
	}
}
