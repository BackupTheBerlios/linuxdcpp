/* 
 * Atomic increment and decrement for some architectures.
 * The actual code in these functions comes from Qt.
 * Not tested at all except for x86.
 * Copyright 2005 Jens Oknelid. Licensed under GNU GPL.
 */

#ifndef WULFOR_ATOMIC_H
#define WULFOR_ATOMIC_H

typedef struct _atomic_t
{
	int counter;
} atomic_t;

inline atomic_t ATOMIC_INIT(volatile long &v)
{
	atomic_t tmp;
	tmp.counter = (int)v;
	return tmp;
}

/*
 * The code in Qt was exactly the same for x86 and x86_64.
 * Not sure if this is appropriate, does this mean the 
 * program will run in 32 bit mode or something?
 */
#ifdef TARGET_X86_64
#define TARGET_X86
#endif

#ifdef TARGET_X86
inline int atomic_inc(volatile atomic_t *ptr)
{
    unsigned char ret;
    asm volatile("lock incl %0\n"
                 "setne %1"
                 : "=m" (*ptr), "=qm" (ret)
                 : "m" (*ptr)
                 : "memory");
    return static_cast<int>(ret);
}

inline int atomic_dec(volatile atomic_t *ptr)
{
    unsigned char ret;
    asm volatile("lock decl %0\n"
                 "setne %1"
                 : "=m" (*ptr), "=qm" (ret)
                 : "m" (*ptr)
                 : "memory");
    return static_cast<int>(ret);
}
#endif

#ifdef TARGET_ALPHA
inline int atomic_inc(volatile atomic_t *ptr)
{
    register int old, tmp;
    asm volatile("1:\n"
                 "ldl_l %0,%2\n"   /* old=*ptr;                               */
                 "addl  %0,1,%1\n" /* tmp=old+1;                              */
                 "stl_c %1,%2\n"   /* if ((*ptr=tmp)!=tmp) tmp=0; else tmp=1; */
                 "beq   %1,2f\n"   /* if (tmp == 0) goto 2;                   */
                 "br    3f\n"      /* goto 3;                                 */
                 "2: br 1b\n"      /* goto 1;                                 */
                 "3:\n"
                 : "=&r" (old), "=&r" (tmp), "+m"(ptr->counter)
                 :
                 : "memory");
    return old != -1;
}

inline int atomic_dec(volatile atomic_t *ptr)
{
    register int old, tmp;
    asm volatile("1:\n"
                 "ldl_l %0,%2\n"   /* old=*ptr;                               */
                 "subl  %0,1,%1\n" /* tmp=old-1;                              */
                 "stl_c %1,%2\n"   /* if ((*ptr=tmp)!=tmp) tmp=0; else tmp=1; */
                 "beq   %1,2f\n"   /* if (tmp==0) goto 2;                     */
                 "br    3f\n"      /* goto 3;                                 */
                 "2: br 1b\n"      /* goto 1;                                 */
                 "3:\n"
                 : "=&r" (old), "=&r" (tmp), "+m"(ptr->counter)
                 :
                 : "memory");
    return old != 1;
}
#endif

#ifdef TARGET_POWERPC
inline int atomic_inc(volatile atomic_t *ptr)
{
    register int ret;
    register int one = 1;
    asm volatile("lwarx  %0, 0, %1\n"
                 "add    %0, %2, %0\n"
                 "stwcx. %0, 0, %1\n"
                 "bne-   $-12\n"
                 : "=&r" (ret)
                 : "r" (ptr), "r" (one)
                 : "cc", "memory");
    return ret;
}

inline int atomic_dec(volatile atomic_t *ptr)
{
    register int ret;
    register int one = -1;
    asm volatile("lwarx  %0, 0, %1\n"
                 "add    %0, %2, %0\n"
                 "stwcx. %0, 0, %1\n"
                 "bne-   $-12\n"
                 : "=&r" (ret)
                 : "r" (ptr), "r" (one)
                 : "cc", "memory");
    return ret;
}
#endif

#ifdef TARGET_SPARC
//The code for this one in Qt is complicated, it calls functions from
//a separate asm file and that's more than my poor asm skills can handle =)

#error Sorry, there's no sparc support for atomic functions. Try turning \
atomic increment/decrement off. (scons atomic=0)
#endif

#ifdef TARGET_ARM
inline char atomic_swp(volatile char *ptr, char newval)
{
    register int ret;
    asm volatile("swpb %0,%1,[%2]"
                 : "=r"(ret)
                 : "r"(newval), "r"(ptr)
                 : "cc", "memory");
    return ret;
}

int atomic_inc(volatile int *ptr)
{
	char *lock = getLock(ptr);
	while (atomic_swp(lock, ~0) != 0)
		;
	int originalValue = *ptr;
	*ptr = originalValue + 1;
	(void) q_atomic_swp(lock, 0);
	return originalValue != -1;
}

int atomic_dec(volatile int *ptr)
{
	char *lock = getLock(ptr);
	while (atomic_swp(lock, ~0) != 0)
		;
	int originalValue = *ptr;
	*ptr = originalValue - 1;
	(void) q_atomic_swp(lock, 0);
	return originalValue != 1;
}
#endif

#ifdef TARGET_MIPS
/*
 * atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v.  Note that the guaranteed useful range
 * of an atomic_t is only 24 bits.
 */
inline void atomic_add(int i, atomic_t * v)
{
	unsigned long temp;

	__asm__ __volatile__(
		".set push                            \n"
		".set mips2                           \n"
		"1:   ll      %0, %1      # atomic_add\n"
		"     addu    %0, %2                  \n"
		"     sc      %0, %1                  \n"
		"     beqz    %0, 1b                  \n"
		".set pop                             \n"
		: "=&r" (temp), "=m" (v->counter)
		: "Ir" (i), "m" (v->counter));
}

/*
 * atomic_sub - subtract the atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
inline void atomic_sub(int i, atomic_t * v)
{
	unsigned long temp;

	__asm__ __volatile__(
		".set push                            \n"
		".set mips2                           \n"
		"1:   ll      %0, %1      # atomic_sub\n"
		"     subu    %0, %2                  \n"
		"     sc      %0, %1                  \n"
		"     beqz    %0, 1b                  \n"
		".set pop                             \n"
		: "=&r" (temp), "=m" (v->counter)
		: "Ir" (i), "m" (v->counter));
}


#define atomic_dec(v) atomic_sub(1,(v))
#define atomic_inc(v) atomic_add(1,(v))

#endif  /*  mips */

#ifdef TARGET_IA64
/* stripped version - we need only 4byte version */
#define ia64_cmpxchg(sem,ptr,old,new,size) \
({ \
	__typeof__(ptr) _p_ = (ptr); \
	__typeof__(new) _n_ = (new); \
	unsigned long _o_, _r_; \
	_o_ = (unsigned int) (long) (old); \
	__asm__ __volatile__ ("mov ar.ccv=%0;;" :: "rO"(_o_)); \
	__asm__ __volatile__ ("cmpxchg4."sem" %0=[%1],%2,ar.ccv" \
			      : "=r"(_r_) : "r"(_p_), "r"(_n_) : "memory"); \
	(__typeof__(old)) _r_; \
})

inline int
ia64_atomic_add (int i, atomic_t *v)
{
	int iold, inew;
	// CMPXCHG_BUGCHECK_DECL

	do {
		// CMPXCHG_BUGCHECK(v);
		iold = (v->counter);
		inew = iold + i;
	} while (ia64_cmpxchg("acq", v, iold, iold + i, sizeof(atomic_t)) != iold);
	return inew;
}

inline int
ia64_atomic_sub (int i, atomic_t *v)
{
	int iold, inew;
	// CMPXCHG_BUGCHECK_DECL

	do {
		// CMPXCHG_BUGCHECK(v);
		iold = atomic_read(v);
		inew = iold - i;
	} while (ia64_cmpxchg("acq", v, iold, inew, sizeof(atomic_t)) != iold);
	return inew;
}

#define atomic_inc(v)			atomic_add(1, (v))
#define atomic_dec(v)			atomic_sub(1, (v))

#endif /* ia64 */



#endif
