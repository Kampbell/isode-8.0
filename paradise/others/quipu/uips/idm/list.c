#include <stdio.h>
#include "util.h"
#include "namelist.h"
#include "quipu/util.h"

void
addToList(lp, str)
struct namelist ** lp;
char *str;
{
	for (; *lp != NULLLIST; lp = &(*lp)->next) {};
	*lp = list_alloc();
	(*lp)->name = copy_string(str);
	(*lp)->next = NULLLIST;
}

