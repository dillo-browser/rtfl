#include "testtools.hh"
#include "common/tools.hh"

#include <unistd.h>

using namespace rtfl::tools;

namespace rtfl {

namespace tests {

int openPipe (const char *command)
{
   int pipefd[2];

   if (pipe (pipefd) == -1)
      syserr ("pipe failed");

   switch (fork ()) {
   case -1:
      syserr ("fork failed");
      break;
      
   case 0:
      if (close (pipefd[0]) == -1)
         syserr ("close(%d) failed", pipefd[0]);
      if (close (1) == -1)
         syserr ("close(%d) failed", 1);
      if (dup2 (pipefd[1], 1) == -1)
         syserr ("dup2(%d, %d) failed", pipefd[1], 1);
      if (close (pipefd[1]) == -1)
         syserr ("close(%d) failed", pipefd[1]);
      execlp ("sh", "sh", "-c", command, NULL);
      syserr ("exec(\"%s\", \"%s\", \"%s\", \"%s\", NULL) failed",
              "sh", "sh", "-c", command);
      break;

   default:
      if (close (pipefd[1]) == -1)
         syserr ("close(%d) failed", pipefd[1]);
      return pipefd[0];
   }

   return -1;        
}

} // namespace tests
   
} // namespace rtfl
