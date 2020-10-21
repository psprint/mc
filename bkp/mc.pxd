cdef extern from "<glib.h>":
    ctypedef unsigned int guint
    ctypedef void* gpointer
    cdef struct _GPtrArray:
        gpointer *pdata
        guint    len

    ctypedef _GPtrArray GPtrArray


cdef extern from "<sys/types.h>":
    ctypedef long off_t

cdef extern from "src/editor/editbuffer.h":

    ctypedef struct edit_buffer_t:
        off_t curs1
        off_t curs2
        GPtrArray *b1
        GPtrArray *b2
        off_t size
        long lines
        long curs_line
