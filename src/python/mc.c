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
#include "src/python/mc.h"
#include "src/python/util.h"

#include <Python.h>

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

#include "src/python/util.h"

OpenFiles*
pymc_get_open_files ()
{
    WDialog *h = DIALOG(pymc_get_frontent_dialog());

    struct OpenFiles *ret;
    ret = (OpenFiles*) calloc(1,sizeof(OpenFiles));

    // Open the log…
    FILE *log = fopen("log.txt","a+");
    
    // No active widget (—→ so also no text editor)?
    if (!h) {
        fprintf(log,"Returning @@@ NULL @@@…\n");
        fclose(log);
        return ret;
    }
    
    // Get the group of the widgets ↔ the text editors,
    // presumably → that the currently active widget
    // belongs.
    const WGroup *g = CONST_GROUP (h);

    fprintf(log, "1) WGroup *g —→ %p\n", g); fflush(log);

    GList *w, *objs = NULL;
    int i, count = 0;

    for (w = g->widgets; w != NULL; w = g_list_next (w)) {
        fprintf(log, "1) w->data ˙˙el from g->widgets˙˙ —→ %p\n", CONST_WIDGET(w->data));

        if (edit_widget_is_editor (CONST_WIDGET (w->data)))
        {
            WEdit *e = (WEdit *) w->data;
            char *fname;

            if (e->filename_vpath == NULL)
                fname = g_strdup_printf ("%c [%s]", e->modified ? '*' :
                 ' ', _("NoName"));
            else
                fname = g_strdup_printf ("%c%s",
                            e->modified ? '*' : ' ',
                            vfs_path_as_str (e->filename_vpath));

    	    OpenFile *file = (OpenFile *) malloc(sizeof(OpenFile));
            file->filename = fname;
            file->filepath = g_strdup(fname);
            file->modified = e->modified;

            objs = g_list_append(objs, file);
            count ++;
        }
    }

    fprintf(log, "1) The count is… —→ °%d°\n", count);
    fflush(log);

    // Create the array of OpenFile objects.
    ret->_array = (OpenFile*) calloc(count+1,sizeof(OpenFile));
    ret->_array[count].modified = -1;
    
    // The index in the _array, incremented while populating it.
    i = 0;
    
    for (w = g_list_first(objs); w != NULL; 
	w = g_list_next (w))
    {
	OpenFile *file = (OpenFile *) w->data;
        ret->_array[i].filename = file->filename;
        ret->_array[i].filepath = file->filepath;
        ret->_array[i].modified = file->modified;
        i++;

        fprintf(log, "1) Set the #%d file ··%s °°\n", i, file->filename);
        fflush(log);
     
        // FIXME
	free(file);
   
        if (i>=count)
            break;

//            listbox_add_item (listbox->list, LISTBOX_APPEND_AT_END, get_hotkey (i++),
//                              str_term_trim (fname, WIDGET (listbox->list)->cols - 2), e, FALSE);
//            g_free (fname);
    }

    ret->_count = count;
    g_list_free(objs);
    
    // Close the log…
    fclose(log);
    return ret;
}
/* --------------------------------------------------------------------------------------------- */
