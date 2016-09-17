; swizzle data of len 25
; shuffles half-nibbles by this indexing (leftmost is 0): [7, 0, 3, 1, 2, 6, 4, 5]
; uint16_t* swizzle(uint16_t* data) {
;     uint16_t * end = data+25;
;     while (data != end) {
;         uint16_t dp = *data;
;         uint16_t val = 0;
;         val |= (dp & 0b1100000000000000) >> 14; // 0 -> 7
;         val |= (dp & 0b11000000000000) << 2; // 1 -> 0
;         val |= (dp & 0b110000000000) >> 2; // 2 -> 3
;         val |= (dp & 0b1100000000) << 4; // 3 -> 1
;         val |= (dp & 0b11000000) << 4; // 4 -> 2
;         val |= (dp & 0b110000) >> 2; // 5 -> 6 
;         val |= (dp & 0b1100) << 4; // 6 -> 4
;         val |= (dp & 0b11) << 4; // 7 -> 5
;         *data = val;
;         data++;
;     }
;     return data-25;
; }

; in: s00 = data 
; out: s00 = data
; clobbers: TODO
equ __swizzle_data s00
equ __swizzle_end s01
equ __swizzle_dp s02
equ __swizzle_tmp s04
equ __swizzle_val s05
equ __swizzle_user_size 25
swizzle:
    mv __swizzle_end, __swizzle_data
    adi __swizzle_end, 25 ; end = data+25
__swizzle_loop:
    jq __swizzle_data, __swizzle_end, __swizzle_done ; while (data != end) {
    md __swizzle_dp, __swizzle_data ; dp = *data
    mi __swizzle_val, 0 ; val = 0
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b1100000000000000
    nd __swizzle_tmp, sc0
    mi sc0, 14
    sr __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b1100000000000000) >> 14
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b11000000000000
    nd __swizzle_tmp, sc0
    mi sc0, 2
    sl __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b11000000000000) << 2
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b110000000000
    nd __swizzle_tmp, sc0
    mi sc0, 2
    sr __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b110000000000) >> 2
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b1100000000
    nd __swizzle_tmp, sc0
    mi sc0, 4
    sl __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b1100000000) << 4
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b11000000
    nd __swizzle_tmp, sc0
    mi sc0, 4
    sl __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b11000000) << 4
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b110000
    nd __swizzle_tmp, sc0
    mi sc0, 2
    sr __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b110000) >> 2
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b1100
    nd __swizzle_tmp, sc0
    mi sc0, 4
    sl __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b1100) << 4
    
    mv __swizzle_tmp, __swizzle_dp
    mi sc0, 0b11
    nd __swizzle_tmp, sc0
    mi sc0, 4
    sl __swizzle_tmp, sc0
    or __swizzle_val, __swizzle_tmp ; val |= (dp & 0b11) << 4

    st __swizzle_val, __swizzle_data ; *data = val
    inc __swizzle_data ; data++
    ji __swizzle_loop
__swizzle_done:
    sbi __swizzle_data, __swizzle_user_size
    ret ; return data-25
