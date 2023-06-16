#include <errno.h>
#include <stm32f7xx.h>
#include <stdio.h>

#include "../fglib/usart.h"

extern int errno;
register char * stack_ptr __asm("sp");

// TODO implement a str_buffer here to avoid overflowing the TRACE pin?

int _write(int file, char* data, int len)
{
#ifndef RELEASE
    usart_print(data,len); // enqueues a string
#endif // RELEASE
    return len;
}

// use an #ifdef to choose ITM or USART (CLI flag?)
int fputc(int ch, FILE *f)
{
    f = f;
    ITM_SendChar( (uint32_t)ch );
    return ch;
}

caddr_t _sbrk(int incr)
{
    extern char end __asm("end");
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0)
        heap_end = &end;

    prev_heap_end = heap_end;
    if (heap_end + incr > stack_ptr)
    {
        errno = ENOMEM;
        return (caddr_t) -1;
    }

    heap_end += incr;

    return (caddr_t) prev_heap_end;
}
