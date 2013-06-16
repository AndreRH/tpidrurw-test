/*
 * Quick&Dirty program for testing tpidrurw register with threading on ARM
 *
 * Copyright 2013 André Hentschel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include <sys/types.h>

static unsigned int errorcount;

#ifdef __arm__
#define __ASM_GLOBAL_FUNC(name,code) asm(".text\n\t.align 4\n\t.globl " #name "\n\t.type " #name ",2\n" #name ":\n\t.cfi_startproc\n\t" code "\n\t.cfi_endproc\n\t.previous");

extern unsigned int get_tls2( void );
__ASM_GLOBAL_FUNC( get_tls2,
                   "mrc	p15, 0, r0, c13, c0, 2\n\t"
                   "bx LR\n\t"
                   )

extern void set_tls2( unsigned int );
__ASM_GLOBAL_FUNC( set_tls2,
                   "mcr	p15, 0, r0, c13, c0, 2\n\t"
                   "bx lr\n\t"
                   )
#else
#warning This test should be compiled for ARM
static unsigned int x86TPIDRURW;

unsigned int get_tls2( void )
{
    return x86TPIDRURW;
}

void set_tls2( unsigned int t )
{
    x86TPIDRURW = t;
}
#endif

static void test_thread( unsigned int *info )
{
    int i;
    set_tls2(*info);
    printf("INFO: test_thread with %08x\n", *info);

    for (i=0; i<5; i++)
    {
        unsigned int r = get_tls2();
        if (r != *info)
        {
            printf("ERROR: TPIDRURW is %08x, expected %08x\n", r, *info);
            errorcount++;
        }
        sleep(1);
    }
}

static void test_fork_thread( unsigned int info )
{
    int i;
    printf("INFO: test_fork_thread with %08x\n", info);

    for (i=0; i<5; i++)
    {
        unsigned int r = get_tls2();
        if (r != info)
        {
            printf("ERROR: TPIDRURW is %08x, expected %08x\n", r, info);
            errorcount++;
        }
        sleep(1);
    }
}

static void test_fork( unsigned int *info )
{
    int i;
    pid_t tpid;
    set_tls2(*info);
    printf("INFO: test_fork with %08x\n", *info);


    tpid = fork();
    if (tpid == -1)
        printf("WARNING: failed to fork\n");
    else if (tpid == 0) /* one for the child */
        test_fork_thread(*info);
    else /* and one for the parent */
        test_fork_thread(*info);
}

int main(void)
{
    pthread_t pthread_id1, pthread_id2, pthread_id3;
    pthread_attr_t attr1, attr2, attr3;
    unsigned int t1 = 0xcafebabe, t2 = 0xdeadbeef, t3 = 0x12345678, r;
    set_tls2(0xdeadc0de);
    r = get_tls2();
    if (r != 0xdeadc0de)
    {
        printf("ERROR: TPIDRURW is %08x, expected %08x\n", r, 0xdeadc0de);
        errorcount++;
    }

    pthread_attr_init( &attr1 );
    if (pthread_create( &pthread_id1, &attr1, (void * (*)(void *))test_thread, &t1 ))
        printf("WARNING: failed to create first thread\n");

    pthread_attr_init( &attr2 );
    if (pthread_create( &pthread_id2, &attr2, (void * (*)(void *))test_thread, &t2 ))
        printf("WARNING: failed to create first thread\n");

    pthread_attr_init( &attr3 );
    if (pthread_create( &pthread_id3, &attr3, (void * (*)(void *))test_fork, &t3 ))
        printf("WARNING: failed to create first thread\n");

    set_tls2(0x1badbabe);
    r = get_tls2();
    if (r != 0x1badbabe)
    {
        printf("ERROR: TPIDRURW is %08x, expected %08x\n", r, 0x1badbabe);
        errorcount++;
    }

    sleep(10);

    printf("\nYour kernel %s suitable for running Windows RT Applications with Wine!\n\n", errorcount?"IS NOT":"IS");

    return 0;
}
