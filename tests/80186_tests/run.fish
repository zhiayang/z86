#!/usr/bin/env fish

for asm in tests/80186_tests/*.asm
	set rom (noext $asm)".bin"
	set res (dirname $asm)"/res_"(basename (noext $rom))".out"

	nasm -f bin -o $rom $asm

	set sz (wc -c $res | awk '{print $1}')

	printf "testing "(basename $rom)
	build/z86 --rom $rom --program $rom
	if test $status -ne 0
		printf ":\tERROR\n"
		break
	end

	head -c $sz mem.bin > .mem.bin
	diff .mem.bin $res > /dev/null
	if test $status -ne 0
		printf ":\t\033[1m\033[31mFAILED\033[0m\n"
		break
	else
		printf ":\t\033[1m\033[32mPASSED\033[0m\n"
		rm .mem.bin
	end
end
