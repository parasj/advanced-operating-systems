#include <gt_include.h>

microtime_t getmicroseconds() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (microtime_t) (tv.tv_sec * (long long) 1000000 + tv.tv_usec);
}