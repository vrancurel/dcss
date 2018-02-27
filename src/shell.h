/*
 * Copyright 2017-2018 the QuadIron authors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __KAD_SHELL_H__
#define __KAD_SHELL_H__

#include <cstdlib>
#include <cstring>
#include <string>

#include <sys/wait.h>
#include <unistd.h>
#if defined(HAVE_READLINE)
#include <readline/history.h>
#include <readline/readline.h>
#else
#include <editline/readline.h>
#endif /* HAVE_READLINE */

namespace kad {

#define SHELL_MAX_ARG_LEN 128
#define SHELL_MAX_ARGV 16

enum shell_error {
    SHELL_ERROR_NONE,
    SHELL_ERROR_TOO_MANY_ARGS,
    SHELL_ERROR_ARG_TOO_LONG,
    SHELL_ERROR_DBL_QUOTE,
};

#define SHELL_CONT 0
#define SHELL_EPARSE (-1)
#define SHELL_RETURN (-2)

class Shell;

struct cmd_def {
    const char* name;
    const char* purpose;
    int (*func)(Shell* shell, int argc, char** argv);
};

class Shell {
  public:
    Shell();

    void set_cmds(struct cmd_def** definitions);
    void set_handle(void* hdl);
    void set_handle2(void* hdl);
    void set_prompt(const std::string& ps1);
    void* get_handle();
    void* get_handle2();
    void loop();

  protected:
    struct cmd_def** defs;
    void* handle;
    void* handle2;
    std::string prompt;
    static bool class_initialized;

    int do_cmd(struct cmd_def** definitions, int argc, char** argv);
    int parse(struct cmd_def** definitions, char* str, enum shell_error* errp);
};

} // namespace kad

#endif
