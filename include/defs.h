// defs.h
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#pragma once

#include <string>
#include <utility>

#include "zpr.h"

namespace instrad { namespace x86 { struct Instruction; } }
namespace z86
{
	std::string print_att(const instrad::x86::Instruction& instr, uint64_t ip, size_t margin = 4, size_t maxBytes = 13);

	namespace util
	{
		size_t getFileSize(const std::string& path);
		std::pair<uint8_t*, size_t> readEntireFile(const std::string& path);
	}

	namespace lg
	{
		bool isDebugEnabled();
		std::string getLogMessagePreambleString(int lvl, std::string_view sys);

		template <typename... Args>
		static inline void __generic_log(int lvl, std::string_view sys, const std::string& fmt, Args&&... args)
		{
			if(!isDebugEnabled() && lvl < 0)
				return;

			auto out = getLogMessagePreambleString(lvl, sys);

			out += zpr::sprint(fmt, static_cast<Args&&>(args)...);

			if(lvl >= 2)    fprintf(stderr, "%s\n", out.c_str());
			else            printf("%s\n", out.c_str());
		}

		template <typename... Args>
		static void log(std::string_view sys, const std::string& fmt, Args&&... args)
		{
			__generic_log(0, sys, fmt, static_cast<Args&&>(args)...);
		}

		template <typename... Args>
		static void warn(std::string_view sys, const std::string& fmt, Args&&... args)
		{
			__generic_log(1, sys, fmt, static_cast<Args&&>(args)...);
		}

		template <typename... Args>
		static void error(std::string_view sys, const std::string& fmt, Args&&... args)
		{
			__generic_log(2, sys, fmt, static_cast<Args&&>(args)...);
		}

		template <typename... Args>
		static void fatal(std::string_view sys, const std::string& fmt, Args&&... args)
		{
			__generic_log(3, sys, fmt, static_cast<Args&&>(args)...);
			abort();
		}

		template <typename... Args>
		static void dbglog(std::string_view sys, const std::string& fmt, Args&&... args)
		{
			__generic_log(-1, sys, fmt, static_cast<Args&&>(args)...);
		}
	}

	namespace colours
	{
		constexpr const char* COLOUR_RESET  = "\x1b[0m";
		constexpr const char* BLACK         = "\x1b[30m";
		constexpr const char* RED           = "\x1b[31m";
		constexpr const char* GREEN         = "\x1b[32m";
		constexpr const char* YELLOW        = "\x1b[33m";
		constexpr const char* BLUE          = "\x1b[34m";
		constexpr const char* MAGENTA       = "\x1b[35m";
		constexpr const char* CYAN          = "\x1b[36m";
		constexpr const char* WHITE         = "\x1b[37m";
		constexpr const char* BLACK_BOLD    = "\x1b[1m";
		constexpr const char* RED_BOLD      = "\x1b[1m\x1b[31m";
		constexpr const char* GREEN_BOLD    = "\x1b[1m\x1b[32m";
		constexpr const char* YELLOW_BOLD   = "\x1b[1m\x1b[33m";
		constexpr const char* BLUE_BOLD     = "\x1b[1m\x1b[34m";
		constexpr const char* MAGENTA_BOLD  = "\x1b[1m\x1b[35m";
		constexpr const char* CYAN_BOLD     = "\x1b[1m\x1b[36m";
		constexpr const char* WHITE_BOLD    = "\x1b[1m\x1b[37m";
		constexpr const char* GREY_BOLD     = "\x1b[30;1m";
	}
}
