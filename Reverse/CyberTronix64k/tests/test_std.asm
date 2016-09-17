ji start

import std.constants
import std.printhex
import std.println
import std.strpack

MESSAGE1: data "Blerghle\n"
MESSAGE2: data rep 9 0x0
equ MESSAGE_LEN 9

MESSAGE_PACKED:
M0: data 0x0
M1: data 0x0
M2: data 0x0
M3: data 0x0
M4: data 0x0

start:
	mi s00, MESSAGE_PACKED
	mi s01, MESSAGE1
	mi s02, MESSAGE_LEN
	call strpack

	mv s00, M0
	call printhex
	mi STDOUT, '\n'

	mv s00, M1
	call printhex
	mi STDOUT, '\n'

	mv s00, M2
	call printhex
	mi STDOUT, '\n'

	mv s00, M3
	call printhex
	mi STDOUT, '\n'

	mv s00, M4
	call printhex
	mi STDOUT, '\n'

	mi s00, MESSAGE2
	mi s01, MESSAGE_PACKED
	mi s02, MESSAGE_LEN
	call strunpack
	
	mi s00, MESSAGE2
	mi s01, MESSAGE_LEN
	call println

	hf
