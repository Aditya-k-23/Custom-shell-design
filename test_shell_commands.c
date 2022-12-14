#include <stdio.h>
#include "custom_shell.h"

int main(void)
{
  int r;

  // {
  //   echo A
  //   { ls -l ; echo B > f1 ; cat f1 ; } > f2
  //   echo C
  // }
  char *lsl[] = { "ls", "-l", NULL };
  char *catf1[] = { "cat", "f1", NULL };
  struct cmd innerarray[3] = {
    { .redir_stdout = NULL,
      .type = FORX,
      .data = { .forx = { "ls", lsl } }
    },
    { .redir_stdout = "f1",
      .type = ECHO,
      .data = { .echo = { "B\n" } }
    },
    { .redir_stdout = NULL,
      .type = FORX,
      .data = { .forx = { "cat", catf1 } }
    }
  };
  struct cmd innerarray_2[3] = {
    { .redir_stdout = NULL,
      .type = FORX,
      .data = { .forx = { "ls", lsl } }
    },
    { .redir_stdout = "f5",
      .type = ECHO,
      .data = { .echo = { "B\n" } }
    },
    { .redir_stdout = NULL,
      .type = FORX,
      .data = { .forx = { "cat", catf1 } }
    }
  };
  struct cmd outerarray[3] = {
    { .redir_stdout = NULL,
      .type = ECHO,
      .data = { .echo = { "A\n" } }
    },
    { .redir_stdout = "f2",
      .type = LIST,
      .data = { .list = { 3, innerarray } }
    },
    { .redir_stdout = NULL,
      .type = ECHO,
      .data = { .echo = { "C\n" } }
    },
    { .redir_stdout = "f6",
      .type = LIST,
      .data = { .list = { 3, innerarray_2 } }
    }
  };
  struct cmd example1 = {
    .redir_stdout = NULL,
    .type = LIST,
    .data = { .list = { 3, outerarray } }
  };

  r = interp(&example1);
  printf("return value = %d\n", r);
  // Expected outermost stdout:
  // A
  // C
  // Expected return value: 0
  // Expected f1:
  // B
  // Expected f2:
  // <output of ls -l, followed by:>
  // B   # because of "cat f1"


  // Now we change to:
  // {
  //   echo A
  //   { ./silly -INT ; echo B > f3 ; cat f3 ; } > f4
  //   echo C
  // }
  // "./silly -INT" kills itself with SIGINT
  // This means we don't run "echo B > f3", "cat f3", or "echo C".
  // Expected outmost stdout:
  // A
  // Expected return value: 128 + SIGINT (2) = 130
  // Expected f4: exists but empty
  // f3 should be untouched (neither created nor modified)
  innerarray[1].redir_stdout = "f3";
  catf1[1] = "f3";
  outerarray[1].redir_stdout = "f4";
  char *sillyint[] = { "silly", "-INT", NULL };
  innerarray[0].type = FORX;
  innerarray[0].data.forx.pathname = "./silly";
  innerarray[0].data.forx.argv = sillyint;
  r = interp(&example1);
  printf("return value = %d\n", r);

  // If you change -INT to -TERM or -SEGV, the other commands should run fine.
  // Or 42 to give exit status 42 for example.

  return 0;
}
