#include <stddef.h>
#include <sys/time.h>

#include <sigcx/timeval.h>

namespace SigCX
{

TimeVal TimeVal::operator-(const TimeVal& tv) const
{
  TimeVal y = tv;

  /* Perform the carry for the later subtraction by updating Y. */
  if (tv_usec < y.tv_usec)
  {
    int nsec = (y.tv_usec - tv_usec) / 1000000 + 1;
    y.tv_usec -= 1000000 * nsec;
    y.tv_sec += nsec;
  }
  if (tv_usec - y.tv_usec > 1000000)
  {
    int nsec = (tv_usec - y.tv_usec) / 1000000;
    y.tv_usec += 1000000 * nsec;
    y.tv_sec -= nsec;
  }
     
  return TimeVal(tv_sec - y.tv_sec, tv_usec - y.tv_usec);
}

void TimeVal::get_current_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  tv_sec = tv.tv_sec;
  tv_usec = tv.tv_usec;
}

}
