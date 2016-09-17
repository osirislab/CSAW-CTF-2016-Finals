import constants

public getline

# struct str {
#	  char* ptr;
#   size_t len;
# };
#
# str getline(char* ptr, size_t capacity) {
#   char* start = ptr;
#   char* end = ptr;
#   end += capacity;
#   while (ptr != end) {
#     char c = read(STDIN);
#     if (c == '\n') {
#       break;
#     }
#     *ptr = c;
#     ptr++;
#   }
#   size_t len = end - ptr;
#   return { ptr, len };
# }

equ __ar_getline_ptr s00
equ __a_getline_capacity s01
equ __r_getline_len s01

equ __c_getline_lf s02
equ __c_getline_1 s03

equ __v_getline_start s04
equ __v_getline_end s05
equ __v_getline_c s06

getline:
	mi __c_getline_lf, '\n'
	mi __c_getline_1, 1

  mv __v_getline_start, __ar_getline_ptr
  mv __v_getline_end, __ar_getline_ptr
  ad __v_getline_end, __a_getline_capacity

	__v_getline_loop_test:
  jq __v_getline_start, __v_getline_end, __l_getline_loop_end
		__v_getline_loop_start:
			mv __v_getline_c, STDIN
			jq __v_getline_c, __c_getline_lf, __l_getline_loop_end
			ld __v_getline_start, __v_getline_c
			ad __v_getline_start, __c_getline_1
			ji __v_getline_loop_test
		__l_getline_loop_end:

  mv __r_getline_len, __v_getline_start
  sb __r_getline_len, __ar_getline_ptr
  ret
