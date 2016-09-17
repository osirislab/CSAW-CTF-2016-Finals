import constants

public print

# void print(char* ptr, size_t len) {
#   char* end = ptr;
#   end += len;
#   while (ptr != end) {
#     write(STDOUT, *ptr);
#     ptr++;
#   }
# }

equ __a_print_ptr s00
equ __a_print_len s01

equ __c_print_1 s02 ; instead of inc, make the temporary reg once

equ __v_print_end s03

print:
	mi __c_print_1, 1

  mv __v_print_end, __a_print_ptr
	ad __v_print_end, __a_print_len

	__l_print_loop_test:
	jq __a_print_ptr, __v_print_end, __l_print_loop_end
	  __l_print_loop_start:
		  md STDOUT, __a_print_ptr
			ad __a_print_ptr, __c_print_1
			ji __l_print_loop_test
	  __l_print_loop_end:

	ret
