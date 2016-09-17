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
    for el in s:
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

print(base64dec(base64enc('ABCDE')))
