#ifndef __GENERIC_H__
#define __GENERIC_H__

#include "list.h"
#include "map.h"
#include "rbtree.h"
#include "splay.h"
#include "utility.h"

#define new(type, ...) new_##type(__VA_ARGS__)

#endif