/* -*- c -*- */

/**
 * Simple CTimer example code.
 *
 * @file        ctimer_example.c
 * @author      Alexandros-Stavros Iliopoulos
 * @license     MIT
 * @copyright   Copyright (c) 2021 Supertech group, CSAIL, MIT
 */


/******************************************************************************/
/* MIT License                                                                */
/*                                                                            */
/* Copyright (c) 2021 Supertech group, CSAIL, MIT                             */
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining      */
/* a copy of this software and associated documentation files (the            */
/* "Software"), to deal in the Software without restriction, including        */
/* without limitation the rights to use, copy, modify, merge, publish,        */
/* distribute, sublicense, and/or sell copies of the Software, and to         */
/* permit persons to whom the Software is furnished to do so, subject to      */
/* the following conditions:                                                  */
/*                                                                            */
/* The above copyright notice and this permission notice shall be             */
/* included in all copies or substantial portions of the Software.            */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,            */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF         */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.     */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY       */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,       */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE          */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                     */
/******************************************************************************/


/**! [ctimer_example] */
#include <stdio.h>
#include <unistd.h>

#include "ctimer.h"

int main () {
    ctimer_t t;
    ctimer_start( &t );
    sleep( 1 );
    ctimer_stop( &t );
    ctimer_measure( &t ); /* unnecessary if `CTIMER_MEASURE_ON_STOP` is #define'd */
    printf( "Elapsed time: %f s\n", timespec_sec( t.elapsed ) );
    printf( "Elapsed time: %ld ms\n", timespec_msec( t.elapsed ) );
    printf( "Elapsed time: %ld us\n", timespec_usec( t.elapsed ) );
    printf( "Elapsed time: %ld ns\n", timespec_nsec( t.elapsed ) );
    return 0;
}
/**! [ctimer_example] */
