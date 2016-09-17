import constants
import print

public base64enc
public base64dec

equ __base64_OFFSET 48

# NOTE: number + 48
# +--------------------------------------
# |     | 0 1 2 3 4 5 6 7 8 9 A B C D E F
# |-----+--------------------------------
# | 0x0 | 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
# | 0x1 | @ A B C D E F G H I J K L M N O
# | 0x2 | P Q R S T U V W X Y Z [ \ ] ^ _
# | 0x3 | ` a b c d e f g h i j k l m n o

# NOTE: length of to_encode must be length
# NOTE: length of to_write must be three times length
# str base64enc(char* to_write, uint16_t* to_encode, size_t length)
equ __a_base64enc_to_write s00
equ __a_base64enc_to_encode s01
equ __a_base64enc_length s02

equ __r_base64enc_ptr s00
equ __r_base64enc_length s01

equ __c_base64enc_1 s03
equ __c_base64enc_offset s04
equ __c_base64enc_63 s05
equ __c_base64enc_15 s06
equ __c_base64enc_6 s07
equ __c_base64enc_12 s08

equ __v_base64enc_to_write s09
equ __v_base64enc_start s10
equ __v_base64enc_end s11
equ __v_base64enc_curr s12
equ __v_base64enc_out s13

base64enc:
	mi __c_base64enc_1, 1
	mi __c_base64enc_offset, __base64_OFFSET
	mi __c_base64enc_63, 63
	mi __c_base64enc_15, 15
	mi __c_base64enc_6, 6
	mi __c_base64enc_12, 12

	# to_write_start = to_write;
	mv __v_base64enc_to_write, __a_base64enc_to_write
	# start = to_encode
	mv __v_base64enc_start, __a_base64enc_to_encode
  # end = to_encode + length;
	mv __v_base64enc_end, __a_base64enc_to_encode
	ad __v_base64enc_end, __a_base64enc_length
	# while (start != end) {
	__l_base64enc_loop_test:
	jq __v_base64enc_start, __v_base64enc_end, __l_base64enc_loop_end
		__l_base64enc_loop_start:
	#   curr = *start;
			md __v_base64enc_curr, __v_base64enc_start
	#   goto enc_num
			ji __l_base64enc_enc_num
			__l_base64enc_enc_num_end:
	#   start++;
			ad __v_base64enc_start, __c_base64enc_1
	# }
			ji __l_base64enc_loop_test
		__l_base64enc_loop_end:
	# return (to_write, to_write_start - to_write);
	mv __r_base64enc_ptr, __a_base64enc_to_write
	mv __r_base64enc_length, __v_base64enc_to_write
	sb __r_base64enc_length, __r_base64enc_ptr
	ret

	__l_base64enc_enc_num:
	  # out = (curr & 63) + __base64_OFFSET;
		mv __v_base64enc_out, __v_base64enc_curr
		nd __v_base64enc_out, __c_base64enc_63
		ad __v_base64enc_out, __c_base64enc_offset
		# *to_write++ = out;
		ld __v_base64enc_to_write, __v_base64enc_out
		ad __v_base64enc_to_write, __c_base64enc_1

		# out = ((curr >> 6) & 63) + __base64_OFFSET;
		mv __v_base64enc_out, __v_base64enc_curr
		sr __v_base64enc_out, __c_base64enc_6
		nd __v_base64enc_out, __c_base64enc_63
		ad __v_base64enc_out, __c_base64enc_offset
		# *to_write++ = out;
		ld __v_base64enc_to_write, __v_base64enc_out
		ad __v_base64enc_to_write, __c_base64enc_1

		# out = ((curr >> 12) & 15) + __base64_OFFSET;
		mv __v_base64enc_out, __v_base64enc_curr
		sr __v_base64enc_out, __c_base64enc_12
		nd __v_base64enc_out, __c_base64enc_15
		ad __v_base64enc_out, __c_base64enc_offset
		# *to_write++ = out;
		ld __v_base64enc_to_write, __v_base64enc_out
		ad __v_base64enc_to_write, __c_base64enc_1

		# goto enc_num_end
		ji __l_base64enc_enc_num_end


__base64dec_ERROR_MSG: data "DECODE ERROR: "
equ __base64dec_ERROR_MSG_LEN 14

# NOTE: length of to_write must be length
# NOTE: length of to_decode must be three times length
# str base64enc(char* to_write, uint16_t* to_decode, size_t length)
equ __a_base64dec_to_write s00
equ __a_base64dec_to_decode s01
equ __a_base64dec_length s02

equ __r_base64dec_ptr s00
equ __r_base64dec_len s01

equ __c_base64dec_1 s03
equ __c_base64dec_offset s04
equ __c_base64dec_63 s05
equ __c_base64dec_15 s06
equ __c_base64dec_6 s07
equ __c_base64dec_12 s08

equ __v_base64dec_start s09
equ __v_base64dec_end s10
equ __v_base64dec_to_decode s11
equ __v_base64dec_num s12
equ __v_base64dec_tmp s13

base64dec:
	mi __c_base64dec_1, 1
	mi __c_base64dec_offset, __base64_OFFSET
	mi __c_base64dec_63, 63
	mi __c_base64dec_15, 15
	mi __c_base64dec_6, 6
	mi __c_base64dec_12, 12

	# start = to_write;
	mv __v_base64dec_start, __a_base64dec_to_write
	# end = to_write + length;
	mv __v_base64dec_end, __a_base64dec_to_write
	ad __v_base64dec_end, __a_base64dec_length
	# to_decode_start = to_decode;
	mv __v_base64dec_to_decode, __a_base64dec_to_decode

	# while (start != end) {
	__l_base64dec_loop_test:
	jq __v_base64dec_start, __v_base64dec_end, __l_base64dec_loop_end
		__l_base64dec_loop_start:
			# goto dec_num;
			ji __l_base64dec_dec_num
			__l_base64dec_dec_num_end:
			# *start++ = num;
			ld __v_base64dec_start, __v_base64dec_num
			ad __v_base64dec_start, __c_base64dec_1
	# }
		ji __l_base64dec_loop_test
		__l_base64dec_loop_end:

	# ret.ptr = to_write;
	mv __r_base64dec_ptr, __a_base64dec_to_write
	# ret.len = start - ret.ptr;
	mv __r_base64dec_len, __v_base64dec_start
	sb __r_base64dec_len, __r_base64dec_ptr
	# return
	ret

  __l_base64dec_dec_num:
		# num = 0;
		mi __v_base64dec_num, 0
		# tmp = (*to_decode_start++) - __base64_OFFSET;
		md __v_base64dec_tmp, __v_base64dec_to_decode
		ad __v_base64dec_to_decode, __c_base64dec_1
		sb __v_base64dec_tmp, __c_base64dec_offset
		# if (tmp > 63) goto invalid_digit;
		jg __v_base64dec_tmp, __c_base64dec_63, __l_base64dec_invalid_digit
		# num |= tmp;
		or __v_base64dec_num, __v_base64dec_tmp

		# tmp = (*to_decode_start++) - __base64_OFFSET;
		md __v_base64dec_tmp, __v_base64dec_to_decode
		ad __v_base64dec_to_decode, __c_base64dec_1
		sb __v_base64dec_tmp, __c_base64dec_offset
		# if (tmp > 63) goto invalid_digit;
		jg __v_base64dec_tmp, __c_base64dec_63, __l_base64dec_invalid_digit
		# tmp <<= 6;
		sl __v_base64dec_tmp, __c_base64dec_6
		# num |= tmp;
		or __v_base64dec_num, __v_base64dec_tmp

		# tmp = (*to_decode_start++) - __base64_OFFSET;
		md __v_base64dec_tmp, __v_base64dec_to_decode
		ad __v_base64dec_to_decode, __c_base64dec_1
		sb __v_base64dec_tmp, __c_base64dec_offset
		# if (tmp > 15) goto invalid_digit;
		jg __v_base64dec_tmp, __c_base64dec_15, __l_base64dec_invalid_digit
		# tmp <<= 12;
		sl __v_base64dec_tmp, __c_base64dec_12
		# num |= tmp;
		or __v_base64dec_num, __v_base64dec_tmp

		# goto dec_num_end
	  ji __l_base64dec_dec_num_end
	
	__l_base64dec_invalid_digit:
		mv r00, __v_base64dec_tmp
		ad r00, __c_base64dec_offset
	  mi s00, __base64dec_ERROR_MSG
		mi s01, __base64dec_ERROR_MSG_LEN
		call print
		mv STDOUT, r00
		mi STDOUT, ' '
		mi STDOUT, '('
		mv s00, r00
		call printhex
		mi STDOUT, ')'
		mi STDOUT, '\n'
		hf
