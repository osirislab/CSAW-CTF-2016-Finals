import constants
import print

public println

# void println(char* ptr, size_t len) {
#   print(ptr, len);
#   write(STDOUT, '\n');
# }

equ __a_println_ptr s00
equ __a_println_len s01
println:
  call print
	mi STDOUT, '\n'
	ret
