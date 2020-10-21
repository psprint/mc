from os import *
import mc

olist = mc.OpenFiles()

for i in range(len(olist)):
    system("printf '" + olist[i].filename + "'\\\\n >> log.txt")
