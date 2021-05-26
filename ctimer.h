/* -*- c -*- */

/**
 * [Include-only header library]
 * C/C++ timer utilities using POSIX `clock_gettime()`.
 *
 * @sa <https://github.com/sillycross/mlpds/blob/master/fasttime.h>
 *
 * @file        ctimer.h
 * @version     0.2.0
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


/**
 * @mainpage
 *
 * @section overview Overview
 *
 * CTimer is an include-only header library with C/C++ timer utilities using
 * POSIX `clock_gettime()`.
 *
 * Stopwatch utilities:
 * - `ctimer_t`         :: stopwatch struct (start/stop/elapsed timespecs)
 * - `ctimer_start()`   :: start stopwatch
 * - `ctimer_stop()`    :: stop stopwatch
 * - `ctimer_reset()`   :: reset elapsed time
 * - `ctimer_measure()` :: measure elapsed time between start & stop
 * - `ctimer_lap()`     :: accumulate elapsed time between start & stop
 *
 * Timespec struct utilities
 * - `timespec_t`       :: alias for `struct timespec`
 * - `timespec_sub()`   :: calculate difference between 2 timespecs
 * - `timespec_add()`   :: calculate sum of 2 timespecs
 * - `timespec_sec()`   :: timespec tv time in sec (double)
 * - `timespec_msec()`  :: timespec tv time in msec (long)
 * - `timespec_usec()`  :: timespec tv time in usec (long)
 * - `timespec_nsec()`  :: timespec tv time in nsec (long)
 *
 * @section usage Using CTimer
 *
 * @subsection c_std C standard
 *
 * C compilers may require standard `gnu99` or later.  Older compilers may also
 * require linking with `-lrt`.
 *
 * @subsection extern_inline External linkage declarations in C
 *
 * All functions in `ctimer.h` are defined as `inline` (*not* `static inline`),
 * following the C99 standard.  When using a C compiler, each translation unit
 * that uses them must provide corresponding `extern inline` declarations.  To
 * facilitate this, `ctimer.h` defines a convenience macro
 * `__CTIMER_EXTERN_INLINE_DECL` which expands to all such declarations.
 *
 * This is not necessary when using a C++ compiler.  In this case, the
 * `__CTIMER_EXTERN_INLINE_DECL` macro will expand to an empty string.
 *
 * @subsection init Initialization
 *
 * There is no guarantee regarding the initial values of timespec fields in a
 * `ctimer_t` stopwatch.  Querying timespecs that haven't been initialized or
 * measured may return arbitrary results; this includes measuring the elapsed
 * time of an unstopped stopwatch.
 *
 * The `elapsed` timespec of a `ctimer_t` stopwatch can be reset to 0 using the
 * `ctimer_reset()` function.  This is not necessary if timings are only
 * measured using `ctimer_measure()`, but it *is* necessary before using
 * `ctimer_lap()` with an otherwise un-measured stopwatch.
 *
 * @subsection measure_on_stop Automatic elapsed-time measurement on stop
 *
 * If the preprocessor macro `CTIMER_MEASURE_ON_STOP` is defined, then
 * `ctimer_stop()` also calls `ctimer_measure()` internally to calculate and
 * store the elapsed time in the input `ctimer_t` object.
 *
 * @subsection example Example usage in C/C++
 *
 * @snippet ctimer_example.c ctimer_example
 */


#ifndef __H_CTIMER__
#define __H_CTIMER__


#include <time.h>
#include <unistd.h>


/**
 * @defgroup ctimer CTimer
 *
 * C/C++ timer utilities.
 *
 * @{
 */


/* prevent C++ compilers from mangling function names */
#ifdef __cplusplus
extern "C" {
#endif


/* ==================================================
 * CONSTANTS
 * ================================================== */


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


/* ==================================================
 * TIMESPEC API
 * ================================================== */


/**
 * @defgroup ctimer_timespec Timespec API
 *
 * Functions for manipulating and inspecting timespec structs.
 *
 * @{
 */


/**
 * Convenience typedef to avoid typing "struct" over and over.
 */
typedef struct timespec timespec_t;


/**
 * Calculate the time difference between two `timespec` structs.  Store time in
 * sec and nsec in the `tv_sec` and `tv_nsec` field, respectively, of the output
 * `timespec`.
 *
 * @sa <https://stackoverflow.com/a/53708448/1036677>
 */
inline
void timespec_sub (
    timespec_t       * td,      /**<[out] time difference in s and ns */
    timespec_t const   t1,      /**<[in]  start time */
    timespec_t const   t2       /**<[in]  end time */
) {
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
 */
inline
void timespec_add (
    timespec_t       * t1,      /**<[in,out] accumulation timer */
    timespec_t const   t2       /**<[in]     added timer */
) {
    t1->tv_nsec = t1->tv_nsec + t2.tv_nsec;
    t1->tv_sec  = t1->tv_sec  + t2.tv_sec;
    if (t1->tv_nsec >= _NSEC_PER_SEC) {
        t1->tv_nsec -= _NSEC_PER_SEC;
        t1->tv_sec++;
    }
}


/**
 * Return `timespec` time in sec.
 *
 * @return (t.tv_sec + tv_nsec) in sec
 */
inline
double timespec_sec (
    timespec_t const t          /**<[in] timespec */
) {
    return (double)t.tv_sec
        + (double)t.tv_nsec / _NSEC_PER_SEC;
}


/**
 * Return `timespec` time in msec.
 *
 * @return (t.tv_sec + tv_nsec) in msec
 */
inline
long timespec_msec (
    timespec_t const t          /**<[in] timespec */
) {
    return t.tv_sec * _MSEC_PER_SEC
        + t.tv_nsec / _USEC_PER_SEC;
}


/**
 * Return `timespec` time in usec.
 *
 * @return (t.tv_sec + tv_nsec) in usec
 */
inline
long timespec_usec (
    timespec_t const t          /**<[in] timespec */
) {
    return t.tv_sec * _USEC_PER_SEC
        + t.tv_nsec / _MSEC_PER_SEC;
}


/**
 * Return `timespec` time in nsec.
 *
 * @return (t.tv_sec + tv_nsec) in nsec
 */
inline
long timespec_nsec (
    timespec_t const t          /**<[in] timespec */
) {
    return t.tv_sec * _NSEC_PER_SEC
        + t.tv_nsec;
}


/** @} */ /* end group ctimer_timespec */


/* ==================================================
 * STOPWATCH API
 * ================================================== */


/**
 * @defgroup ctimer_stopwatch Stopwatch API
 *
 * Functions for manipulating CTimer stopwatches.
 *
 * @{
 */


/**
 * Stopwatch timer struct using `clock_gettime()`.
 */
typedef struct {
    timespec_t tic;             /**< Stopwatch start time  */
    timespec_t toc;             /**< Stopwatch end time */
    timespec_t elapsed;         /**< Elapsed/measured time */
} ctimer_t;


/**
 * Measure elapsed time of `ctimer_t` stopwatch in s+ns and *store* it in the
 * `elapsed` timer.
 *
 * @note It is safe (albeit unnecessary) to measure the elapsed time of a
 * stopped timer multiple times.
 */
inline
void ctimer_measure (
    ctimer_t * t                /**<[in,out] stopwatch pointer */
) {
    timespec_sub( &(t->elapsed), t->tic, t->toc );
}


/**
 * Measure elapsed time of `ctimer_t` stopwatch in s+ns and *add* it to the
 * `elapsed` timer.
 *
 * @warning It is up to the user to ensure that the `elapsed` field of the input
 *          stopwatch has been properly initialized (e.g. with `ctimer_reset()`)
 *          before `ctimer_lap()` is called.
 */
inline
void ctimer_lap (
    ctimer_t * t                /**<[in,out] stopwatch pointer */
) {
    timespec_t lap_buf;
    timespec_sub( &lap_buf, t->tic, t->toc );
    timespec_add( &(t->elapsed), lap_buf );
}


/**
 * Zero out the `elapsed` timer of a `ctimer_t` stopwatch.
 */
inline
void ctimer_reset (
    ctimer_t * t                /**<[in,out] stopwatch pointer */
) {
    t->elapsed = (timespec_t) {0};
}


/**
 * Start a `ctimer_t` stopwatch.  Sets the the `tic` timer of the stopwatch.
 */
inline
void ctimer_start (
    ctimer_t * t                /**<[in,out] stopwatch pointer */
) {
    clock_gettime( CLOCK_MONOTONIC, &(t->tic) );
}


/**
 * Stop a `ctimer_t` stopwatch.  Sets the `toc` timer of the stopwatch.
 *
 * @note If the `CTIMER_MEASURE_ON_STOP` preprocessor macro is defined, then
 * `ctimer_stop` also calculates the elapsed time between `tic` and `toc` and
 * stores it in the `elapsed` field.
 */
inline
void ctimer_stop (
    ctimer_t * t                /**<[in,out] stopwatch pointer */
) {
    clock_gettime( CLOCK_MONOTONIC, &(t->toc) );
#ifdef CTIMER_MEASURE_ON_STOP
    ctimer_measure( t );
#endif
}


/** @} */ /* end group ctimer_stopwatch */


#ifdef __cplusplus
} /* end extern "C" */
#endif


/* ==================================================
 * EXTERN INLINE DECLARATIONS MACRO
 * ================================================== */


#ifndef __cplusplus             /* C */
/**
 * Convenience macro for declaring all `ctimer.h` functions as `extern inline`
 * in a .c file.
 *
 * @warning This should be used *exactly once* per translation unit.  The macro
 * is only expanded when compiling with a C compiler (which must be
 * C99-compliant).  When compiling with a C++ compiler, the macro is still
 * defined but expands to the empty string.
 */
#define __CTIMER_EXTERN_INLINE_DECL                                                     \
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


/** @} */ /* end group ctimer */


#endif  /* __H_CTIMER__ */
