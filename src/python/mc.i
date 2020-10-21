%module mc
%include "/lib/gcc/x86_64-pc-linux-gnu/10.2.0/include/stddef.h"

//
// typemaps.
//

/* Convert from Python --> C */
%typemap(in) (off_t) {
  $1 = PyInt_AsLong($input);
}

/* Convert from C --> Python */
%typemap(out) (off_t) {
  $result = PyInt_FromLong($1);
}

//
// #include directives.
//

%inline %{
    #include "glib-2.0/glib.h"
    #include "lib/vfs/path.h"
    #include "lib/vfs/vfs.h"
    #include "lib/widget.h"
    #include "python/mc.h"
%}

//
// IMPORT of a C-struct.
//

%include "python/mc.h"

//
// EXTEND of the C-struct.
//

%define AddSubscripting(name, T)
%feature("python:slot", "mp_subscript", functype="binaryfunc") name::__getitem__;
%extend name {
    T __getitem__(int index) {
        T empty = {0};
        // Todo: throw IndexError
        if (index<0 || index>=$self->_count)
            return empty;
        return $self->_array[index];
    }
}
%enddef

%define AddLen(name)
%feature("python:slot", "sq_length", functype="lenfunc") name::__len__;
%extend name {
    size_t __len__() {
        FILE *f = fopen("log.txt","a+");
        fprintf(f, "__len__ called…\n");
        fflush(f);
        fclose(f);
        return $self->_count;
    }
}
%enddef

%immutable OpenFile;
%extend OpenFile {
    OpenFile() {
        OpenFile *obj = (OpenFile*) calloc(1, sizeof(OpenFile));
        obj->modified = -1;
        return obj;
    }
}

%immutable OpenFiles;

%extend OpenFiles {
	OpenFiles() {
	    OpenFiles* lista = pymc_get_open_files();
	    return lista;
	}
	~OpenFiles() {
	    if ($self->_array != NULL) {
                //free($self->_array);
                $self->_array = NULL;
            }
            $self->_count = 0;
	    //free($self);
        }
}

AddSubscripting(OpenFiles, OpenFile)
AddLen(OpenFiles)
