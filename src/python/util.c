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
#include "python/mc.h"

#include <Python.h>

#include "python/util.h"

//extern ssize_t mc_read (int handle, void *buffer, size_t count);

/*** global variables ****************************************************************************/

static const Widget* current_widget = NULL;
static const Widget* frontent_dialog = NULL;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

/* --------------------------------------------------------------------------------------------- */

void pymc_set_current_widget(const Widget *new_current) {
    //printf("------ Set [2] to: %p --------", current_widget);
    FILE *log = fopen("log.txt","a+");
    fprintf(log,"set_current_widget sets… —→ ·˙°%p°˙·\n", new_current);
    fclose(log);
    current_widget = new_current;
}

const Widget* pymc_get_current_widget(void) {
    FILE *log = fopen("log.txt","a+");
    fprintf(log,"get_current_widget returns… —→ ·˙°%p°˙·\n", current_widget);
    fclose(log);
    return current_widget;
}

void pymc_set_frontent_dialog(const Widget *front) {
    //printf("------ Set [2] to: %p --------", frontent_dialog);
    FILE *log = fopen("log.txt","a+");
    fprintf(log,"set_frontent_dialog sets… —→ ·˙°%p°˙·\n", front);
    fclose(log);
    frontent_dialog = front;
}

const Widget* pymc_get_frontent_dialog(void) {
    FILE *log = fopen("log.txt","a+");
    fprintf(log,"get_frontent_dialog returns… —→ ·˙°%p°˙·\n", frontent_dialog);
    fclose(log);
    return frontent_dialog;
}
