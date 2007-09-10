#ifndef __TINYAP_WALKER_H__
#define __TINYAP_WALKER_H__

#include "pilot_manager.h"

const char** known_pilots();

void* walk(wast_t, pilot_t);
void* do_walk(wast_t node, const char* pilot, void* init_data);

#endif

