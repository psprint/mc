import os
from xdress.utils import apiname

package = 'mc'
packagedir = 'src'
includes = [ '/usr/include', '/tmp', '/usr/include/glib-2.0/',
		'/usr/lib/glib-2.0/include/',
		'/usr/lib/gcc/x86_64-pc-linux-gnu/10.2.0/include' ]

plugins = ('xdress.autoall', 'xdress.pep8names', 'xdress.cythongen', 
           'xdress.extratypes')

extra_types = 'mc_extra_types'  # non-default value

_fromsrcdir = lambda x: os.path.join('src', x)
_ineditbuf = {'srcfiles': _fromsrcdir('editor/editbuffer.[ch]'),
             'incfiles': 'editbuffer.h',
             }

classes = [
    apiname('edit_buffer_t', tarbase='mc', **_ineditbuf)
    ]

del os
del apiname
