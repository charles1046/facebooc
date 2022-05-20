#ifndef __PAIR__H__
#define __PAIR__H__

#include "vTable.h"

typedef int (*CMP)(const void*, const void*);
CMP type_cmp(const char* type);

// The T1 is typename 1
// The T2 is typename 2
#define decl_pair(T1, T2)                                                       \
	decl_Object(T1); /*To declare copy_ctor_##T1 , dtor_##T1 ... */             \
	decl_Object(T2); /*To declare copy_ctor_##T2 , dtor_##T2... */              \
	typedef struct vTable_t_##T1_##T2 vTable_t_##T1_##T2;                       \
	typedef struct Pair_##T1_##T2 Pair_##T1_##T2;                               \
	/*For pair only*/                                                           \
	struct vTable_t_##T1_##T2 {                                                 \
		Pair_##T1_##T2* (*copy_ctor_##T1_##T2)(const T1* t1, const T2* t2);     \
		Pair_##T1_##T2* (*move_ctor_##T1_##T2)(T1 * t1, T2* t2);                \
		void (*dtor_##T1_T2)(void* self);                                       \
	};                                                                          \
	inline Pair_##T1_##T2* copy_ctor_##T1_##T2(const T1* t1, const T2* t2) {    \
		Pair_##T1_##T2* p = malloc(sizeof(Pair_##T1_##T2));                     \
		p->key = copy_ctor_##T1(t1);                                            \
		p->value = copy_ctor_##T2(t2);                                          \
		p->vt = gen_vt_##T1_##T2();                                             \
		return p;                                                               \
	}                                                                           \
	inline Pair_##T1_##T2* move_ctor_##T1_##T2(T1* t1, T2* t2) {                \
		Pair_##T1_##T2 p = malloc(sizeof(Pair_##T1_##T2));                      \
		p->key = t1;                                                            \
		key = NULL;                                                             \
		p->value = t2;                                                          \
		value = NULL;                                                           \
		p->vt = gen_vt_##T1_##T2();                                             \
		return p;                                                               \
	}                                                                           \
	inline void dtor_##T1_##T2(void* self) {                                    \
		Pair_##T1_##T2* p = (Pair_##T1_##T2*)self;                              \
		dtor_##T1(p->key);                                                      \
		dtor_##T2(p->value);                                                    \
		free(p->vt);                                                            \
		free(p);                                                                \
	}                                                                           \
	inline vTable_t_##T1_##T2* gen_vt_##T1_##T2(void) {                         \
		vTable_t_##T1_##T2* vt = malloc(sizeof(vTable_t_##T1_##T2));            \
		vt->copy_ctor_##T1_##T2 = copy_ctor_##T1_##T2;                          \
		vt->move_ctor_##T1_##T2 = move_ctor_##T1_##T2;                          \
		vt->dtor_##T1_##T2 = dtor_##T1_##T2;                                    \
		return vt;                                                              \
	};                                                                          \
	struct Pair_##T1_##T2 {                                                     \
		vTable_t_##T1_##T2* vt;                                                 \
		T1* key;                                                                \
		T2* value;                                                              \
	};                                                                          \
	inline Pair_##T1_##T2* new_pair_##T1_##T2(const T1* key, const T2* value) { \
		return copy_ctor_##T1_##T2(key, value);                                 \
	}                                                                           \
	inline Pair_##T1_##T2* new_pair_shadow_##T1_##T2(T1* key, T2* value) {      \
		return move_ctor_##T1_##T2(key, value);                                 \
	}                                                                           \
	inline Pair_##T1_##T2* new_pair_move_##T1_##T2(Pair_##T1_##T2* ref) {       \
		Pair_##T1_##T2* me = ref;                                               \
		ref = NULL;                                                             \
		return me;                                                              \
	}                                                                           \
	inline int Pair_cmp_##T1_##T2(const T1* a, const T1* b) {                   \
		CMP f = type_cmp(#T1);                                                  \
		return f(a, b);                                                         \
	}

// Make an xvalue pair via designed initializer
#define make_x_pair(p1, p2) \
	{ .vt = NULL, .key = (p1), .value = (p2) }

#endif