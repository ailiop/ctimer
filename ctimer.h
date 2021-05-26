/** -*- c -*-
 *
 * @file ctimer.h
 *
 * C/C++ timer utilities using POSIX `clock_gettime()`.
 *
 * Provides the following:
 *
 * - `ctimer_t`         :: stopwatch struct (start/stop/elapsed timespecs)
 * - `ctimer_start()`   :: start stopwatch
 * - `ctimer_stop()`    :: stop stopwatch
 * - `ctimer_reset()`   :: reset stopwatch elapsed time
 * - `ctimer_measure()` :: measure elapsed time between start & stop
 * - `ctimer_lap()`     :: accumulate elapsed time betweeen start & stop
 *
 * - `timespec_t`       :: alias for `struct timespec`
 * - `timespec_sub()`   :: calculate difference between 2 timespecs
 * - `timespec_add()`   :: calculate sum of 2 timespecs
 * - `timespec_sec()`   :: timespec tv time in sec (double)
 * - `timespec_msec()`  :: timespec tv time in msec (long)
 * - `timespec_usec()`  :: timespec tv time in usec (long)
 * - `timespec_nsec()`  :: timespec tv time in nsec (long)
 *
 * @note It is safe (albeit unnecessary) to measure the elapsed time of a
 * stopped timer multiple times.
 *
 * @warning There is no guarantee regarding the initial values of timespec
 * fields in a `ctimer_t` stopwatch.  Querying timespecs that haven't been
 * initialized or measured may return arbitrary results; this includes measuring
 * the elapsed time of an unstopped stopwatch.
 *
 * @note All functions in `ctimer.h` are defined as `inline` (*not* `static
 * inline`), following the C99 standard.  Each translation unit that uses them
 * must provide corresponding `extern inline` declarations.  To facilitate this,
 * `ctimer.h` defines a convenience macro `__CTIMER_EXTERN_INLINE_DECL` which
 * expands to all such declarations.
 *
 * @note If the preprocessor macro `CTIMER_MEASURE_ON_STOP` is defined, then
 * `ctimer_stop()` also calls `ctimer_measure()` internally to calculate and
 * store the elapsed time in the input `ctimer_t` object.
 *
 * @note Example usage in C/C++:
 * @code
 *      #include <stdio.h>
 *      #include "ctimer.h"
 *
 *      // external linkage for `ctimer.h` functions (*once per translation unit*)
  *      __CTIMER_EXTERN_INLINE_DECL; // only expands in C, not C++
 *
 *      int main () {
 *          ctimer_t t;
 *          ctimer_start( &t );
 *          do_some_work();
 *          ctimer_stop( &t );
 *          ctimer_measure( &t ); // unnecessary if `CTIMER_MEASURE_ON_STOP` is #define'd
 *          printf( "Elapsed time: %f s\n", timespec_sec( t.elapsed ) );
 *          printf( "Elapsed time: %ld ms\n", timespec_msec( t.elapsed ) );
 *          printf( "Elapsed time: %ld us\n", timespec_usec( t.elapsed ) );
 *          printf( "Elapsed time: %ld ns\n", timespec_nsec( t.elapsed ) );
 *          return 0;
 *      }
 * @endcode
 *
 * @warning C compilers may require standard `gnu99` or later.  Older compilers
 * may also require linking with `-lrt`.
 *
 * @sa <https://github.com/sillycross/mlpds/blob/master/fasttime.h>
 *
 * @version     0.1.0
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


#ifndef __H_CTIMER__
#define __H_CTIMER__


#include <time.h>
#include <unistd.h>


/* unit conversion constants */
#ifdef __cplusplus              /* C++ */
static const auto _MSEC_PER_SEC = 1000;
static const auto _USEC_PER_SEC = 1000 * 1000;
static const auto _NSEC_PER_SEC = 1000 * 1000 * 1000;
#else  /* C */
enum {
_MSEC_PER_SEC = 1000,
_USEC_PER_SEC = 1000 * 1000,
_NSEC_PER_SEC = 1000 * 1000 * 1000
};
#endif  /* __cplusplus */


/**
 * Convenience typedef to avoid typing "struct" over and over.
 */
typedef struct timespec timespec_t;


/**
 * Stopwatch timer struct using `clock_gettime()`.
 */
typedef struct {
    timespec_t tic;             /*!< Stopwatch start time  */
    timespec_t toc;             /*!< Stopwatch end time */
    timespec_t elapsed;         /*!< Elapsed time between `tic` & `toc` */
} ctimer_t;


/* prevent C++ compilers from mangling function names */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Calculate the time difference between two `timespec` structs.  Store time in
 * sec and nsec in the `tv_sec` and `tv_nsec` field, respectively, of the output
 * `timespec`.
 *
 * @param [out] td      time difference in s and ns
 * @param [in]  t1      start time
 * @param [in]  t2      end time
 *
 * @sa  ctimer_measure
 *
 * @sa  <https://stackoverflow.com/a/53708448/1036677>
 */
inline void timespec_sub (timespec_t       * td,
                          timespec_t const   t1,
                          timespec_t const   t2) {
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec  - t1.tv_sec;
    if ((td->tv_sec > 0) && (td->tv_nsec < 0))  {
        td->tv_nsec += _NSEC_PER_SEC;
        td->tv_sec--;
    } else if ((td->tv_sec < 0) && (td->tv_nsec > 0)) {
        td->tv_nsec -= _NSEC_PER_SEC;
        td->tv_sec++;
    }
    /* (s > 0 & ns > 0) : do nothing (t1 < t2) */
    /* (s < 0 & ns < 0) : do nothing (t1 > t2) */
}


/**
 * Calculate the sum of two `timespec` structs.  Store time in sec and nsec in
 * the `tv_sec` and `tv_nsec` field, respectively, of the first `timespec`
 * operand.
 *
 * @param [in,out]  t1
 * @param [in]      t2
 *
 * @sa  ctimer_measure
 */
inline void timespec_add (timespec_t       * t1,
                          timespec_t const   t2) {
    t1->tv_nsec = t1->tv_nsec + t2.tv_nsec;
    t1->tv_sec  = t1->tv_sec  + t2.tv_sec;
    if (t1->tv_nsec >= _NSEC_PER_SEC) {
        t1->tv_nsec -= _NSEC_PER_SEC;
        t1->tv_sec++;
    }
}


/**
 * Measure elapsed time of `ctimer_t` stopwatch in s+ns and *store* it in the
 * `elapsed` field.
 *
 * @param [in]  t       `ctimer_t` stopwatch pointer
 *
 * @sa  timespec_sub
 * @sa  ctimer_start
 * @sa  ctimer_stop
 * @sa  ctimer_t
 */
inline void ctimer_measure (ctimer_t * t) {
    timespec_sub( &(t->elapsed), t->tic, t->toc );
}


/**
 * Measure elapsed time of `ctimer_t` stopwatch in s+ns and *add* it to the
 * `elapsed` field.
 *
 * @warning It is up to the user to ensure that the `elapsed` field of the input
 * stopwarch has been properly initialized (e.g. with `ctimer_reset()`) before
 * `ctimer_lap()` is called.
 *
 * @param [in]  t       `ctimer_t` stopwatch pointer
 */
inline void ctimer_lap (ctimer_t * t) {
    timespec_t lap_buf;
    timespec_sub( &lap_buf, t->tic, t->toc );
    timespec_add( &(t->elapsed), lap_buf );
}


/**
 * Zero out the `elapsed` field of a `ctimer_t` stopwatch.
 *
 * @param [in] t        `ctimer_t` stopwatch pointer
 */
inline void ctimer_reset (ctimer_t * t) {
    t->elapsed = (timespec_t) {0};
}


/**
 * Start a `ctimer_t` stopwatch.
 *
 * @param [in]  t       `ctimer_t` stopwatch pointer
 *
 * @sa  ctimer_stop
 * @sa  ctimer_measure
 * @sa  ctimer_t
 */
inline void ctimer_start (ctimer_t * t) {
    clock_gettime( CLOCK_MONOTONIC, &(t->tic) );
}


/**
 * Stop a `ctimer_t` stopwatch.
 *
 * If the `CTIMER_MEASURE_ON_STOP` preprocessor macro is defined, then
 * `ctimer_stop` also calculates the elapsed time between `tic` and `toc` and
 * stores it in the `elapsed` field.
 *
 * @param [in]  t       `ctimer_t` stopwatch pointer
 *
 * @sa  ctimer_start
 * @sa  ctimer_measure
 * @sa  ctimer_t
 */
inline void ctimer_stop (ctimer_t * t) {
    clock_gettime( CLOCK_MONOTONIC, &(t->toc) );
#ifdef CTIMER_MEASURE_ON_STOP
    ctimer_measure( t );
#endif
}


/**
 * Return `timespec` time in sec.
 *
 * @param [in]  t       timespec
 * @return              (t.tv_sec + tv_nsec) in sec
 */
inline double timespec_sec (timespec_t const t) {
    return (double)t.tv_sec
        + (double)t.tv_nsec / _NSEC_PER_SEC;
}


/**
 * Return `timespec` time in msec.
 *
 * @param [in]  t       timespec
 * @return              (t.tv_sec + tv_nsec) in msec
 */
inline long timespec_msec (timespec_t const t) {
    return t.tv_sec * _MSEC_PER_SEC
        + t.tv_nsec / _USEC_PER_SEC;
}


/**
 * Return `timespec` time in usec.
 *
 * @param [in]  t       timespec
 * @return              (t.tv_sec + tv_nsec) in usec
 */
inline long timespec_usec (timespec_t const t) {
    return t.tv_sec * _USEC_PER_SEC
        + t.tv_nsec / _MSEC_PER_SEC;
}


/**
 * Return `timespec` time in nsec.
 *
 * @param [in]  t       timespec
 * @return              (t.tv_sec + tv_nsec) in nsec
 */
inline long timespec_nsec (timespec_t const t) {
    return t.tv_sec * _NSEC_PER_SEC
        + t.tv_nsec;
}


#ifdef __cplusplus
} /* end extern "C" */
#endif


#ifndef __cplusplus             /* C */
/**
 * Convenience macro for declaring all `ctimer.h` functions as `extern inline`
 * in a .c file.
 *
 * @warning This should be used *exactly once* per translation unit and is only
 * required when compiling with a C compiler (which must be C99-compliant).
 */
#define __CTIMER_EXTERN_INLINE_DECL \
    extern inline void ctimer_start (ctimer_t *);                                       \
    extern inline void ctimer_stop (ctimer_t *);                                        \
    extern inline void ctimer_reset (ctimer_t *);                                       \
    extern inline void ctimer_measure (ctimer_t *);                                     \
    extern inline void ctimer_lap (ctimer_t *);                                         \
    extern inline void timespec_sub (timespec_t *, timespec_t const, timespec_t const); \
    extern inline void timespec_add (timespec_t *, timespec_t const);                   \
    extern inline double timespec_sec (timespec_t const);                               \
    extern inline long timespec_msec (timespec_t const);                                \
    extern inline long timespec_usec (timespec_t const);                                \
    extern inline long timespec_nsec (timespec_t const);
#else  /* C++ */
#define __CTIMER_EXTERN_INLINE_DECL
#endif  /* !(__cplusplus) */



#endif  /* __H_CTIMER__ */
