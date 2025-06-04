import random
import time

for i in range(0,10):
    count = random.randrange(1, 10, 1)
    snore = ['z' for x in range(0, count)]
    print(''.join(snore))
    time.sleep(count)