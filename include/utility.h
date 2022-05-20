#ifndef __UTILITY_H__
#define __UTILITY_H__
#include <stdlib.h>

/* container_of() - Calculate address of object that contains address ptr
 * @ptr: pointer to member variable
 * @type: type of the structure containing ptr
 * @member: name of the member variable in struct @type
 *
 * Return: @type pointer of object containing ptr
 */
#define container_of(ptr, type, member)                          \
	__extension__({                                              \
		const __typeof__(((type*)0)->member)* __pmember = (ptr); \
		(type*)((char*)__pmember - offsetof(type, member));      \
	})

// Using __typeof__ extension by GCC
// ! Be careful, the type of value might not be deduced as you expect
#define CONST_INIT(x, value) *(__typeof__(value)*)(&x) = value

int string_hash(const char* s);
int obj_hash(const void* data, size_t size);

#endif