#ifndef __SHELL_H__
#define __SHELL_H__ 1

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <sys/wait.h>
#include <unistd.h>
#if defined(HAVE_READLINE)
#include <readline/history.h>
#include <readline/readline.h>
#else
#include <editline/readline.h>
#endif /* HAVE_READLINE */

#define SHELL_MAX_ARG_LEN 128
#define SHELL_MAX_ARGV 16

enum shell_error {
    SHELL_ERROR_NONE,
    SHELL_ERROR_TOO_MANY_ARGS,
    SHELL_ERROR_ARG_TOO_LONG,
    SHELL_ERROR_DBL_QUOTE,
};

#define SHELL_CONT 0
#define SHELL_EPARSE -1
#define SHELL_RETURN -2

class Shell;

struct cmd_def {
    const char* name;
    const char* purpose;
    int (*func)(Shell* shell, int argc, char** argv);
};

class Shell {
  public:
    Shell();

    void set_cmds(struct cmd_def** defs);
    void set_handle(void* handle);
    void set_handle2(void* handle);
    void set_prompt(const char* prompt);
    void* get_handle();
    void* get_handle2();
    void loop();

  protected:
    struct cmd_def** defs;
    void* handle;
    void* handle2;
    char* prompt;
    static bool class_initialized;

    int do_cmd(struct cmd_def** defs, int argc, char** argv);
    int parse(struct cmd_def** defs, char* str, enum shell_error* errp);
};

#endif
