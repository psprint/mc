/*
   Python scripting interpreter for the Midnight Commander

   Copyright (C) 1994-2020
   Free Software Foundation, Inc.

   Written by:
   Sebastian Gniazdowski, 2020

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file python.c
 *  \brief Source: this is the Python interpreter.
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <pwd.h>                /* for username in xterm title */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>             /* getsid() */

#include "lib/global.h"

#include "lib/event.h"
#include "lib/tty/tty.h"
#include "lib/tty/key.h"        /* For init_key() */
#include "lib/tty/mouse.h"      /* init_mouse() */
#include "lib/timer.h"
#include "lib/skin.h"
#include "lib/filehighlight.h"
#include "lib/fileloc.h"
#include "lib/strutil.h"
#include "lib/util.h"
#include "lib/vfs/vfs.h"        /* vfs_init(), vfs_shut() */

#include "filemanager/midnight.h"       /* current_panel */
#include "filemanager/treestore.h"      /* tree_store_save */
#include "filemanager/layout.h"
#include "filemanager/ext.h"    /* flush_extension_file() */
#include "filemanager/command.h"        /* cmdline */
#include "filemanager/panel.h"  /* panalized_panel */

#include "vfs/plugins_init.h"

#include "events_init.h"
#include "args.h"
#ifdef ENABLE_SUBSHELL
#include "subshell/subshell.h"
#endif
#include "setup.h"              /* load_setup() */

#ifdef HAVE_CHARSET
#include "lib/charsets.h"
#include "selcodepage.h"
#endif /* HAVE_CHARSET */

#include "consaver/cons.saver.h"        /* cons_saver_pid */
#include "editor/edit-impl.h"
#include "editor/editwidget.h"
#include "python/initialize.h"
#include "python/mc.h"
#include <Python.h>



//extern ssize_t mc_read (int handle, void *buffer, size_t count);

/*** global variables ****************************************************************************/

int my_variable = 5;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

static int numargs=0;
static PyObject* mc_numargs(PyObject *self, PyObject *args);
static PyObject* PyInit_mc(void);

/* Return the number of arguments of the application command line */
static PyObject* mc_numargs(PyObject *self, PyObject *args)
{
    printf("Hi! (%p)\n", self);
    if(!PyArg_ParseTuple(args, ":numargs"))
        return NULL;

    return PyLong_FromLong(numargs);
}

static PyMethodDef McMethods[] = {
    {"numargs", mc_numargs, METH_VARARGS,
     "Return the number of arguments received by the process."},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef McModule = {
    PyModuleDef_HEAD_INIT, "mc2", NULL, -1, McMethods,
    NULL, NULL, NULL, NULL
};

static PyObject* PyInit_mc(void)
{
    return PyModule_Create(&McModule);
}


/*** public functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */


int pymc_init_python(void) {
//	return 1;
    FILE *f = fdopen(1, "w+");
    //printf(f,"Getting Python information\n");
    numargs = 5;
    PyImport_AppendInittab("mc2", &PyInit_mc);
    Py_Initialize();
    if( !Py_IsInitialized() ) {
        fputs("Unable to initialize Python interpreter.",f);
        return 1;
    }
    return 1;
    fprintf(f,"Prefix: %S\nExec Prefix: %S\nPython Path: %S\n",Py_GetPrefix(),Py_GetExecPrefix(),Py_GetProgramFullPath());
    fprintf(f,"Module Path: %S\n",Py_GetPath());
    return 1;
}

int pymc_run_python_init_code(void) {
    PyRun_SimpleString("import init");
#if 0
    PyObject *strret, *pymod, *strfunc, *strargs;
    PyObject *ret, *mymod, *class, *method, *args, *object;
    float farenheit;
    char *cstrret;

    pymod = PyImport_ImportModule("reverse");
    printf ("%p\n",pymod); fflush(stdout);
    strfunc = PyObject_GetAttrString(pymod, "rstring");
    printf ("%p\n", strfunc); fflush(stdout);
    strargs = Py_BuildValue("(s)", "___");
    strret = PyEval_CallObject(strfunc, strargs);
    printf ("%p\n", strret); fflush(stdout);
    PyArg_Parse(strret, "s", &cstrret);
    printf("Reversed string: %s\n", cstrret);

    /* Load our module */
    mymod = PyImport_ImportModule("celsius");
    /* If we dont get a Python object back there was a problem */
    if (mymod == NULL)
        error("Can't open module");
    /* Find the class */
    class = PyObject_GetAttrString(mymod, "celsius");
    /* If found the class we can dump mymod, since we won't use itagain */
    Py_DECREF(mymod);
    /* Check to make sure we got an object back */
    if (class == NULL){
        Py_DECREF(class);
        error("Can't find class");
    }
    /* Build the argument call to our class - these are the argumentsthat will be supplied when the object is created */
    args = Py_BuildValue("(f)", 18.0);
    if (args == NULL){
        Py_DECREF(args);
        error("Can't build argument list for class instance");
    }
    /* Create a new instance of our class by calling the classwith our argument list */
    object = PyEval_CallObject(class, args);
    if (object == NULL){
        Py_DECREF(object);
        error("Can't create object instance");
    }
    /* Decrement the argument counter as we'll be using this again */
    Py_DECREF(args);
    /* Get the object method - note we use the object as the objectfrom which we access the attribute by name, not the class */
    method = PyObject_GetAttrString(object, "farenheit");
    if (method == NULL){
        Py_DECREF(method);
        error("Can't find method");
    }
    /* Decrement the counter for our object, since we now just needthe method reference */
    Py_DECREF(object);
    /* Build our argument list - an empty tuple because there aren'tany arguments */
    args = Py_BuildValue("()");
    if (args == NULL){
        Py_DECREF(args);
        error("Can't build argument list for method call");
    }
    /* Call our object method with arguments */
    ret = PyEval_CallObject(method,args);
    if (ret == NULL){
        Py_DECREF(ret);
        error("Couldn't call method");
    }
    /* Convert the return value back into a C variable and display it */
    PyArg_Parse(ret, "f", &farenheit);
    printf("Farenheit: %f\n", farenheit);
    /* Kill the remaining objects we don't need */
    Py_DECREF(method);
    Py_DECREF(ret);

    PyObject* ret1 = NULL;

    PyObject* main = PyImport_AddModule("__main__");
    PyRun_SimpleString("print('1…'); import mc; print('2…'); print(mc.__name__); \
    print(dir(mc));");
#if 0
    PyObject* mc = PyImport_AddModule("mc");
    printf("mc: %p\n", mc);
    PyObject* _g = PyModule_GetDict(mc);
    printf("_g: %p\n", _g);
    PyObject* _l = PyDict_New();

    ret1 = PyRun_String("print(my_variable);", Py_eval_input, _g, _l);
    if(ret1) {
        printf(ret1->ob_type->tp_name);
    } else {
        printf("A (hidden?) problem executing the string…");
    }
#endif
#endif
    return 0;
}


/* --------------------------------------------------------------------------------------------- */
