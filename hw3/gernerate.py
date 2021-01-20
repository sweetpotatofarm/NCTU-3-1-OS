import random
import numpy as np
import os
size = int(input("The array size is: "))
a = np.random.randint(-1000000000,1000000000,size,int)
file=open('./input.txt', mode='w')
file.writelines([str(size),'\n'])
for i in a:
    file.writelines([str(i)," "])
file.close()

file=open('./output_ans.txt', mode='w')
a.sort()
for i in a:
    file.writelines([str(i)," "])
file.close()
