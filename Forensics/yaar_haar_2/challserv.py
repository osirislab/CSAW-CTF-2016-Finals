#!/usr/bin/env python
import time, random, sys
ims = []
lines = []
with open("b64strs.txt","r") as f:
    lines = f.read().split("\n")
for line in lines:
    ims.append(line.split("."))
print "YAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARRRRRRRR, THANKS FOR HELPIN' ME FIND ME BOOTY IN QUALS. HOWEVR, I SEEM TO HAF' LOST ME MATEYS! COULD YE HELP ME FIND THEM? BEWAAAAAAAAAAARRNED, I HAF LOTS A' MATEYS! I'LL GIV YA THE LAY OF THE LAND, AND YE BE TELLIN ME WHICH QUADRANT ME MATE BE IN! (0 for no matey, 1 for upper right, 2 for upper left, 3 for lower left, 4 for lower right)\n"
time.sleep(1)
numlist = random.sample(range(800),800)
failed = False
for num in numlist:
    print ims[num][2]+"\n"
    sys.stdout.flush()
    ans = raw_input()

    if ans[0] == ims[num][1]:
        continue
    else:
        failed = True
        break
if failed == True:
    print "YYYYYYYYYYYYYYYYYYYAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARRRRRRRRRRRRRRRRRRRRRRRR THERE BE NO MATEY HERE, BE GONE WITH YE!"
else:
    print "YYYYYYYYYAAAAAAAAAAAAAAAAAAAAAAAAAARRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR YE FOUND ME MATEYS! AS A REWARD, HERE BE A SHARE OF THE TREASURE: flag{lazytown_is_best_town}"

