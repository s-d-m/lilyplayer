#include <signal.h>
#include <cstring> // for memset
#include <cerrno>
#include <stdexcept>
#include <string>

#include "signals_handler.hh"

extern volatile sig_atomic_t pause_requested;
extern volatile sig_atomic_t continue_requested;
extern volatile sig_atomic_t exit_requested;
extern volatile sig_atomic_t new_signal_received;

volatile sig_atomic_t pause_requested = 0;
volatile sig_atomic_t continue_requested = 0;
volatile sig_atomic_t exit_requested = 0;
volatile sig_atomic_t new_signal_received = 0;

static
const char* signum_to_str(int signum)
{
  switch (signum)
  {
    case  SIGHUP:     return  "SIGHUP";
    case  SIGINT:     return  "SIGINT";
    case  SIGQUIT:    return  "SIGQUIT";
    case  SIGILL:     return  "SIGILL";
    case  SIGABRT:    return  "SIGABRT";
    case  SIGFPE:     return  "SIGFPE";
    case  SIGKILL:    return  "SIGKILL";
    case  SIGSEGV:    return  "SIGSEGV";
    case  SIGPIPE:    return  "SIGPIPE";
    case  SIGALRM:    return  "SIGALRM";
    case  SIGTERM:    return  "SIGTERM";
    case  SIGUSR1:    return  "SIGUSR1";
    case  SIGUSR2:    return  "SIGUSR2";
    case  SIGCHLD:    return  "SIGCHLD";
    case  SIGCONT:    return  "SIGCONT";
    case  SIGSTOP:    return  "SIGSTOP";
    case  SIGTSTP:    return  "SIGTSTP";
    case  SIGTTIN:    return  "SIGTTIN";
    case  SIGTTOU:    return  "SIGTTOU";
    case  SIGBUS:     return  "SIGBUS";
    case  SIGPOLL:    return  "SIGPOLL";
    case  SIGPROF:    return  "SIGPROF";
    case  SIGSYS:     return  "SIGSYS";
    case  SIGTRAP:    return  "SIGTRAP";
    case  SIGURG:     return  "SIGURG";
    case  SIGVTALRM:  return  "SIGVTALRM";
    case  SIGXCPU:    return  "SIGXCPU";
    case  SIGXFSZ:    return  "SIGXFSZ";
    case  SIGSTKFLT:  return  "SIGSTKFLT";
    case  SIGPWR:     return  "SIGPWR";
    case  SIGWINCH:   return  "SIGWINCH";

    default:
      return "unknown signal";
  }
}

static void signal_handler(int signum)
{
  switch (signum)
  {
    case SIGTSTP:
      new_signal_received = 1;
      pause_requested = 1;
      break;

    case SIGCONT: // Continue if stopped
      new_signal_received = 1;
      continue_requested = 1;
      break;

    case SIGQUIT: // stop program
    case SIGTERM:
    case SIGINT:  // Interrupt from keyboard
      new_signal_received = 1;
      exit_requested = 1;
      break;

    case SIGUSR1: // User-defined signal 1
    case SIGUSR2: // User-defined signal 2
    default:
      break; // ignore these errors, but don't let the program stop for these.
  }
}


#if defined(__clang__)
  // clang will complain that sa_handler is a recursive macro.
  // It issue this warning even when explictly disabling warnings from system headers
  // using -Wno-system-headers Below is the error it produces.
  //
  // signals_handler.cc:102:6: error: disabled expansion of recursive macro
  //       [-Werror,-Wdisabled-macro-expansion]
  //   sa.sa_handler = &signal_handler;
  //      ^
  // /usr/include/i386-linux-gnu/bits/sigaction.h:36:41: note: expanded from macro 'sa_handler'
  // # define sa_handler     __sigaction_handler.sa_handler
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

void set_signal_handler()
{
  struct sigaction sa;
  std::memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = &signal_handler;

  for (const auto signum : { SIGINT, // Interrupt from keyboard
			     SIGCONT, // Continue if stopped
			     SIGQUIT, // Quit from keyboard
			     SIGTERM, // Termination signal
			     SIGTSTP, // Stop typed at terminal
			     SIGUSR1, // User-defined signal 1
			     SIGUSR2 // User-defined signal 2
			   })
  {
    if (sigaction(signum, &sa, nullptr) == -1)
    {

      throw std::runtime_error(std::string{"Error while setting the interrupt hangler for signal "} + signum_to_str(signum) +
			       " (" + std::strerror(errno) + ")");
    }
  }

}
#if defined(__clang__)
  #pragma clang diagnostic pop
#endif
