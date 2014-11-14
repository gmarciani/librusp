#ifndef RUDIMENU_H_
#define RUDIMENU_H_

#include "rudicore.h"


/* ACTION MENU ENTRIES */

typedef enum menuChoice_t {
	MENU_GTCWD,
	MENU_CHDIR,
	MENU_LSDIR,
	MENU_MKDIR,
	MENU_RMDIR,
	MENU_CPDIR,
	MENU_MVDIR,
	MENU_MKFILE,
	MENU_OPFILE,
	MENU_DWFILE,
	MENU_UPFILE,
	MENU_RMFILE,
	MENU_CPFILE,
	MENU_MVFILE,
	MENU_EXIT
} menuChoice_t;


/* MENU AND INPUTS */

menuChoice_t rudiMenu(message_t *msg);

#endif /* RUDIMENU_H_ */
