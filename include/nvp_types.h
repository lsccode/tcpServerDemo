#ifndef __NVP_TYPES_H__
#define __NVP_TYPES_H__

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#if defined (_WIN64) || defined (__LP64__) || defined (__64BIT__) || defined (_LP64) || (__WORDSIZE == 64) 
typedef unsigned char           NVP_U8;
typedef unsigned short          NVP_U16;
typedef unsigned int            NVP_U32;
typedef unsigned long long		NVP_U64;

typedef signed char				NVP_S8;
typedef signed short            NVP_S16;
typedef signed int              NVP_S32;
typedef signed long long		NVP_S64;

typedef float					NVP_FLOAT;
typedef double                  NVP_DOUBLE;

typedef char                    NVP_BYTE;
typedef char                    NVP_CHAR;
//typedef void                    NVP_VOID;
#define NVP_VOID                void

typedef void*					NVP_HANDLE;
typedef int						NVP_BOOL;
typedef unsigned long long      NVP_POINTER;
#else
typedef unsigned char           NVP_U8;
typedef unsigned short          NVP_U16;
typedef unsigned int            NVP_U32;
typedef unsigned long long		NVP_U64;

typedef signed char				NVP_S8;
typedef signed short            NVP_S16;
typedef signed int              NVP_S32;
typedef signed long long		NVP_S64;

typedef float					NVP_FLOAT;
typedef double                  NVP_DOUBLE;

typedef char                    NVP_BYTE;
typedef char                    NVP_CHAR;
//typedef void                    NVP_VOID;
#define NVP_VOID                void

typedef void*					NVP_HANDLE;
typedef int						NVP_BOOL;
typedef unsigned int            NVP_POINTER;
#endif

#define	NVP_FALSE				(0)
#define	NVP_TRUE				(1)
#define NVP_NULL				(0L)

#define NVP_SUCCEED             (0)
#define NVP_FAILURE             (-1)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __NVP_TYPES_H__ */

