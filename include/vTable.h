#ifndef __VTABLE__H__
#define __VTABLE__H__
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define M_CAT2(a, b) a##_##b
#define M_CAT(a, b) M_CAT2(a, b)

// Example:
// You should use M_CAT and M_CAT2 to concatenate 2 type
//
// #define M_FUNC(T1, T2)                                                 \
// 	typedef struct M_CAT(Map, M_CAT2(T1, T2)) M_CAT(Map, M_CAT2(T1, T2)); \
// 	struct M_CAT(Map, M_CAT2(T1, T2)) {                                   \
// 		T1 val1;                                                          \
// 		T2 val2;                                                          \
// 	};

// The vtable is simulate this: https://en.wikipedia.org/wiki/Virtual_method_table
// You should use a pointer to share your vtable from your class
#define decl_Object(typename)                                                  \
	typedef struct vTable_t_##typename vTable_t_##typename;                    \
	typedef struct Object_##typename Object_##typename;                        \
	struct vTable_t_##typename {                                               \
		typename* (*copy_ctor_##typename)(const typename* ref_##typename);     \
		typename* (*move_ctor_##typename)(typename * ref_##typename);          \
		void (*dtor_##typename)(void* self);                                   \
	};                                                                         \
	inline typename* copy_ctor_##typename(const typename* ref) {               \
		if(!ref)                                                               \
			return NULL;                                                       \
		typename* v = malloc(sizeof(typename));                                \
		memcpy(v, ref, sizeof(typename));                                      \
		return v;                                                              \
	}                                                                          \
	inline typename* move_ctor_##typename(typename * ref) {                    \
		typename* me = ref;                                                    \
		ref = NULL;                                                            \
		return me;                                                             \
	}                                                                          \
	inline void dtor_##typename(void* self) {                                  \
		free(self);                                                            \
	}                                                                          \
	inline vTable_t_##typename* gen_vt_##typename(void) {                      \
		vTable_t_##typename* vt = malloc(sizeof(vTable_t_##typename));         \
		vt->copy_ctor_##typename = copy_ctor_##typename;                       \
		vt->move_ctor_##typename = move_ctor_##typename;                       \
		vt->dtor_##typename = dtor_##typename;                                 \
		return vt;                                                             \
	}                                                                          \
	struct Object_##typename {                                                 \
		vTable_t_##typename* vt;                                               \
		typename* data;                                                        \
	};                                                                         \
	inline Object_##typename* new_obj_shadow_##typename(typename * data) {     \
		Object_##typename* o = malloc(sizeof(Object_##typename));              \
		o->data = data;                                                        \
		o->vt = gen_vt_##typename();                                           \
		return o;                                                              \
	}                                                                          \
	inline Object_##typename* new_obj_##typename(typename * data) {            \
		Object_##typename* o = malloc(sizeof(Object_##typename));              \
		o->data = malloc(sizeof(typename));                                    \
		memcpy(o->data, data, sizeof(typename));                               \
		o->vt = gen_vt_##typename();                                           \
		return o;                                                              \
	}                                                                          \
	inline Object_##typename* dup_obj_##typename(const Object_##typename* o) { \
		if(!o)                                                                 \
			return NULL;                                                       \
		Object_##typename* new_o = malloc(sizeof(Object_##typename));          \
		new_o->data = malloc(sizeof(typename));                                \
		memcpy(new_o->data, o->data, sizeof(typename));                        \
		new_o->vt = malloc(sizeof(vTable_t_##typename));                       \
		memcpy(new_o->vt, o->vt, sizeof(vTable_t_##typename));                 \
		return new_o;                                                          \
	}                                                                          \
	inline void default_dtor_##typename(Object##typename * o) {                \
		o->vt->dtor(o->data);                                                  \
		free(o->vt);                                                           \
	}

// You are not expected to use this
#define ptr_decl__(typename) typedef typename* typename##_ptr
#define const_decl__(typename) typedef const typename typename##_const
#define const_prt_decl__(typename) typedef const typename* typename##_const_ptr
#define prt_const_decl__(typename) typedef typename* const typename##_ptr_const
#define const_prt_const_decl__(typename) typedef const typename* const typename##_const_ptr_const

#define type_declarator(typename) \
	ptr_decl__(typename);         \
	const_decl__(typename);       \
	const_prt_decl__(typename);   \
	prt_const_decl__(typename);   \
	const_prt_const_decl__(typename)

type_declarator(int);
type_declarator(float);
type_declarator(char);
type_declarator(double);
type_declarator(long);
type_declarator(size_t);

#endif
