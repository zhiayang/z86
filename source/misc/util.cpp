// util.cpp
// Copyright (c) 2020, zhiayang
// Licensed under the Apache License Version 2.0.

#include <errno.h>
#include <sys/stat.h>

#include <fstream>

#include "defs.h"

namespace z86::util
{
	size_t getFileSize(const std::string& path)
	{
		struct stat st;
		if(stat(path.c_str(), &st) != 0)
		{
			char buf[128] = { 0 };
			strerror_r(errno, buf, 127);
			lg::error("misc", "failed to get filesize for '{}' (error code {} / {})", path, errno, buf);

			return -1;
		}

		return st.st_size;
	}

	std::pair<uint8_t*, size_t> readEntireFile(const std::string& path)
	{
		auto bad = std::pair(nullptr, 0);;

		auto sz = getFileSize(path);
		if(sz == static_cast<size_t>(-1)) return bad;

		// i'm lazy, so just use fstreams.
		auto fs = std::fstream(path);
		if(!fs.good()) return bad;


		uint8_t* buf = new uint8_t[sz + 1];
		fs.read(reinterpret_cast<char*>(buf), sz);
		fs.close();

		return std::pair(buf, sz);
	}
}


namespace z86::lg
{
	constexpr bool ENABLE_DEBUG = false;
	constexpr bool USE_COLOURS  = true;

	constexpr const char* WHITE_BOLD_RED_BG = "\x1b[1m\x1b[37m\x1b[48;5;9m";

	constexpr const char* DBG    = (USE_COLOURS ? colours::WHITE : "");
	constexpr const char* LOG    = (USE_COLOURS ? colours::GREY_BOLD : "");
	constexpr const char* WRN    = (USE_COLOURS ? colours::YELLOW_BOLD : "");
	constexpr const char* ERR    = (USE_COLOURS ? colours::RED_BOLD : "");
	constexpr const char* FTL    = (USE_COLOURS ? WHITE_BOLD_RED_BG : "");

	constexpr const char* RESET  = (USE_COLOURS ? colours::COLOUR_RESET : "");
	constexpr const char* SUBSYS = (USE_COLOURS ? colours::BLUE_BOLD : "");

	std::string getLogMessagePreambleString(int lvl, std::string_view sys)
	{
		const char* lvlcolour = 0;
		const char* str = 0;

		if(lvl == -1)       { lvlcolour = DBG;  str = "[dbg]"; }
		else if(lvl == 0)   { lvlcolour = LOG;  str = "[log]"; }
		else if(lvl == 1)   { lvlcolour = WRN;  str = "[wrn]"; }
		else if(lvl == 2)   { lvlcolour = ERR;  str = "[err]"; }
		else if(lvl == 3)   { lvlcolour = FTL;  str = "[ftl]"; }

		auto loglevel  = zpr::sprint("{}{}{}", lvlcolour, str, RESET);
		auto subsystem = zpr::sprint("{}{}{}", SUBSYS, sys, RESET);

		return zpr::sprint("{} {}: ", loglevel, subsystem);
	}

	bool isDebugEnabled() { return ENABLE_DEBUG; }
}
