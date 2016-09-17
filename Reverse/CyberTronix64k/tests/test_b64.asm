ji start

import std.constants
import std.base64
import std.println
import std.printhex

ENCODED: data rep 21 0x0
equ ENCODED_LEN 21
DECODED: data rep 7 0x0
equ DECODED_LEN 7

BINDATA: data 0x4239 0x3920 0x4921 0x8483 0x0012 0x0318 0x3829
equ BINDATA_LEN 7


start:
	mi s00, ENCODED
	mi s01, BINDATA
	mi s02, BINDATA_LEN
	call base64enc
	call println
	mi s00, DECODED
	mi s01, ENCODED
	mi s02, DECODED_LEN
	call base64dec

	mv r00, s00
	mi r01, 0
	mv r02, s01
	mi r03, 1

	loop_test:
	jq r01, r02, loop_end
		md s00, r00
		call printhex
		mi STDOUT, ' '
		ad r00, r03
		ad r01, r03
		ji loop_test
	loop_end:

	mi STDOUT, '\n'
	hf
