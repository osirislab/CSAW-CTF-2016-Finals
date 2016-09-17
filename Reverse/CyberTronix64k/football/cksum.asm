import std.constants

public cksum

; uint16_t cksum(uint16_t* data, uint16_t nbytes) {
;     uint16_t cks1 = 0, cks2 = 0;
;     uint16_t* end = data + nbytes;
;     while (data != end) {
;         uint16_t dp = *data;
;         cks1 += dp;
;         cks2 += cks1 + ((dp & 0xFF00)>>8);
;         data++;
;     }
;     cks1 &= 0xFF;
;     cks2 &= 0xFF;
;     return (cks1 << 8) | cks2;
; }

; in:  s00 = data, s01 = nbytes
; out: s00 = checksum
; clobbers: s00, s01, s02, s03, s04

equ __cksum_data s00
equ __cksum_end s01
equ __cksum_cks1 s02
equ __cksum_cks2 s03
equ __cksum_dp s04

cksum:
    mi __cksum_cks1, 0x0 ; cks1 = 0
    mi __cksum_cks2, 0x0 ; cks2 = 0
    ad __cksum_end, __cksum_data ; end = data + nbytes
__cksum_loop: ; while (data != end) {
    jq __cksum_data, __cksum_end, __cksum_compl ; jmp if done
    md __cksum_dp, __cksum_data ; dp = *data
    ad __cksum_cks1, __cksum_dp ; cks1 += dp
    ad __cksum_cks2, __cksum_cks1 ; cks2 += cks1
    mi sc0, 0xFF00
    nd __cksum_dp, sc0 ; dp & 0xFF00
    mi sc0, 8
    sr __cksum_dp, sc0 ; (dp & 0xFF00) >> 8
    ad __cksum_cks2, __cksum_dp ; cks2 += (dp & 0xFF00) >> 8
    inc __cksum_data ; data++
    ji __cksum_loop ; }
__cksum_compl:
    mi sc0, 0xFF
    nd __cksum_cks1, sc0 ; cks1 &= 0xFF
    nd __cksum_cks2, sc0 ; cks2 &= 0xFF
    mi sc0, 8
    sl __cksum_cks1, sc0 ; cks1 <<= 8
    mv s00, __cksum_cks2 ; retval = cks2
    or s00, __cksum_cks1 ; retval |= cks1
    ret
