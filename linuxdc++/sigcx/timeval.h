/* This is for emacs: -*-Mode: C++;-*- */
#if !defined(__INC_TIMEVAL_H)
#define __INC_TIMEVAL_H

namespace SigCX
{

/** Timespan. */
class TimeVal
{
  public:
    /** Seconds. */
    long tv_sec;
    /** Microseconds. */
    long tv_usec;

    /** Constructor. 
     * \param secs Seconds.
     * \param usecs Microseconds. */
    explicit TimeVal(long secs = 0, long usecs = 0) {
      tv_sec = secs;
      tv_usec = usecs;
    }
    /** Comparision operator. */
    bool operator<(const TimeVal& tv) const {
      return tv_sec < tv.tv_sec || 
        (tv_sec == tv.tv_sec && tv_usec < tv.tv_usec);
    }
    /** Comparision operator. */
    bool operator==(const TimeVal& tv) const {
      return tv_sec == tv.tv_sec && tv_usec == tv.tv_usec;
    }
    /** Comparision operator. */
    bool operator<=(const TimeVal& tv) const { 
      return *this < tv || *this == tv; 
    }
    /** Comparision operator. */
    bool operator>=(const TimeVal& tv) const { 
      return !(*this < tv); 
    }
    
    /** Substraction operator. */
    TimeVal operator-(const TimeVal& tv) const;
    /** Addition operator. */
    TimeVal operator+(const TimeVal& tv) const {
      return *this - TimeVal(-tv.tv_sec, -tv.tv_usec);
    }
    TimeVal& operator+=(const TimeVal& tv) {
      return (*this = *this + tv);
    }
    /** Get the current time.
     * Store the current time in \c tv_sec and \c tv_usec. */
    void get_current_time();
};

}

#endif
