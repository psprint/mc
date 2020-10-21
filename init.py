from reverse import *
from os import *

print(rstring("!)gnirts esrever( ereht iH"))

import mc
olist = mc.OpenFiles()
print (len(olist))
for i in iter(olist):
    print(i.filename)
    system("printf '" + i.filename + "'\\\\n >> log2.txt")
    