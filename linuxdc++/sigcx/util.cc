#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sigcx/sigcxconfig.h>

#include <errno.h>

#include <sigcx/util.h>


namespace SigCX
{

std::string errno_string(int e)
{
  std::string result;
  
#if SIGCX_THREADS
#if defined(HAVE_STRERROR_R)
  char buf[128];
  result = strerror_r(e, buf, sizeof(buf));
#elif defined(HAVE___STRERROR_R)
  char buf[128];
  result = __strerror_r(e, buf, sizeof(buf));
#else
  // We don't have a thread-safe strerror
  result = strerror(e); 
#endif
#else
  result = strerror(e);
#endif // SIGCX_THREADS

  return result;
}

std::string errno_string()
{
  return errno_string(errno);
}

}
