#ifndef I386_ATOMIC_H
#define I386_ATOMIC_H

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

inline int atomic_inc(volatile atomic_t *ptr)
{
    unsigned char ret;
    asm volatile("lock incl %0\n"
                 "setne %1"
                 : "=m" (ptr->counter), "=qm" (ret)
                 : "m" (ptr->counter)
                 : "memory");
    return static_cast<int>(ret);
}

inline int atomic_dec(volatile atomic_t *ptr)
{
    unsigned char ret;
    asm volatile("lock decl %0\n"
                 "setne %1"
                 : "=m" (ptr->counter), "=qm" (ret)
                 : "m" (ptr->counter)
                 : "memory");
    return static_cast<int>(ret);
}

inline int atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
	unsigned char ret;
    asm volatile("lock cmpxchgl %2,%3\n"
                 "sete %1\n"
                 : "=a" (newval), "=qm" (ret)
                 : "r" (newval), "m" (*ptr), "0" (expected)
                 : "memory");
    return static_cast<int>(ret);
}

inline int atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    return atomic_test_and_set_int(reinterpret_cast<volatile int *>(ptr),
                                     reinterpret_cast<int>(expected),
                                     reinterpret_cast<int>(newval));
}

#endif
