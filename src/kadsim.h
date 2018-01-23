#ifndef __KADSIM_H__
#define __KADSIM_H__

#include "bignum.h"
#include "cmds.h"
#include "config.h"
#include "exceptions.h"
#include "kad_conf.h"
#include "kad_file.h"
#include "kad_network.h"
#include "kad_node.h"
#include "kad_routable.h"
#include "shell.h"
#include "utils.h"

/* io.cpp */
extern void do_put(const char *filename, int n_data, int n_parities);
#endif
