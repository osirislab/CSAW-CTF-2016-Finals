equ ptr 0x40
equ c_1 0x41

mi c_1, 1
mi ptr, 0x1000
loop:
	md 0x200, ptr
	ad ptr, c_1
	ji loop
