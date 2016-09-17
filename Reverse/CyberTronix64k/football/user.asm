; --- Various stuff to deal with user data
;
; The structs we're dealing with, in C syntax:
;
; struct password {
;     uint16_t cksum;
;     struct user_data data;
; }
; 
; struct user_data {
;     uint16_t name[16];  // username
;     uint16_t secret[8]; // randomized password, must match username's entry or error occurs
;     uint16_t ac_index;   // index into authcode array (actually just 4 bits, but we need to be word-aligned anyways
; }

import std.constants

public user_data_get_name
public user_data_get_secret
public user_data_get_ac_index
public equ user_data_len 25


; uint16_t[16]* user_data_get_name(struct user_data* ud) {
;    return &ud->name;
; }
user_data_get_name:
    ret ; offset 0, so we just need to noop

; uint16_t[8]* user_data_get_secret(struct user_data* ud) {
;    return &ud->secret;
; }
user_data_get_secret:
    adi s00, 16 ; name is 16 words, skip that
    ret

; uint16_t* user_data_get_ac_index(struct user_data* ud) {
;    return &ud->ac_index;
; }
user_data_get_ac_index:
    adi s00, 24 ; 16 + 8 to skip (name + secret)
    ret
