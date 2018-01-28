#include <iostream>

#include "shell.h"

static char sargmem[SHELL_MAX_ARGV][SHELL_MAX_ARG_LEN];
static char* sargv[SHELL_MAX_ARGV + 1];
static int sargc;

const char* shell_err_strings[] = {
    "No error",
    "too many args",
    "argument is too long",
    "double quote error",
    NULL,
};

const char* shell_error_str(enum shell_error err)
{
    return (shell_err_strings[err]);
}

static struct cmd_def** g_cmd_defs = NULL;

/** For completion.
 *
 * @param defs
 */
void shell_install_cmd_defs(struct cmd_def** defs)
{
    g_cmd_defs = defs;
}

static char* command_generator(const char* text, int state)
{
    static int list_index, len;
    const char* name;
    struct cmd_def* def;

    if (state == 0) {
        list_index = 0;
        len = strlen(text);
    }

    while (g_cmd_defs[list_index] != nullptr) {
        def = g_cmd_defs[list_index];
        name = def->name;

        list_index++;

        if (strncmp(name, text, len) == 0) {
            char* nptr = strdup(name);
            assert(NULL != nptr);
            return nptr;
        }
    }

    /* If no names matched, then return NULL. */
    return ((char*)NULL);
}

char** shell_completion(const char* text, int start, int end)
{
    char** matches;

    if (NULL == g_cmd_defs) {
        return NULL;
    }

    matches = (char**)NULL;

    if (start == 0) {
        matches = rl_completion_matches(text, command_generator);
    }

    return (matches);
}

bool Shell::class_initialized = false;

Shell::Shell()
{
    if (!Shell::class_initialized) {
        rl_attempted_completion_function = shell_completion;
        Shell::class_initialized = true;
    }

    defs = NULL;
    handle = NULL;
    handle2 = NULL;
    prompt = NULL;
}

int Shell::do_cmd(struct cmd_def** defs, int argc, char** argv)
{
    struct cmd_def *def, *tmp;
    int ret, len, found;
    char* p;
    int i;

    if (argc == 0) {
        return SHELL_CONT;
    }

    if ('!' == argv[0][0]) {
        pid_t pid;

        pid = fork();
        if (-1 == pid) {
            perror("fork");
            return SHELL_CONT;
        }
        if (0 == pid) {
            argv[0]++;

            ret = execvp(argv[0], argv);
            if (-1 == ret) {
                perror("execvp");
                exit(1);
            }
        } else {
        retry:
            ret = wait(0);
            if (-1 == ret) {
                if (EINTR == errno) {
                    goto retry;
                } else {
                    perror("wait");
                    exit(1);
                }
            }
        }
        return SHELL_CONT;
    }
    if (NULL != (p = index(argv[0], '='))) {
        *p++ = 0;
        return SHELL_CONT;
    }
    len = strlen(argv[0]);

    def = NULL;
    found = 0;
    for (i = 0; defs[i] != nullptr; i++) {
        tmp = defs[i];
        if (strcmp(argv[0], tmp->name) == 0) {
            found = 1;
            def = tmp;
            break;
        }
        if (strncmp(argv[0], tmp->name, len) == 0) {
            if (1 == found) {
                fprintf(stderr, "ambiguous command: %s\n", argv[0]);
                return SHELL_CONT;
            }
            found = 1;
            def = tmp;
        }
    }

    if (0 == found) {
        fprintf(stderr, "cmd %s: not found\n", argv[0]);
        return SHELL_CONT;
    }

    ret = def->func(this, argc, argv);
    return ret;
}

/** Parse a string into multiple commands.
 *
 * Everything is done static.
 *
 * Understands comments (sharp sign), double quotes, semicolon. ignore
 * whitspaces
 *
 * @param str the input string
 * @param cmd the callback
 * @param errp the error code
 *
 * @return 0 if OK, -1 on failure
 */
int Shell::parse(struct cmd_def** defs, char* str, enum shell_error* errp)
{
    int pos, ret;
    bool comment = false;
    bool dblquote = false;
    bool skip_ws = true;
    char c;

    sargc = pos = 0;

    while (true) {
        c = *str;

        switch (c) {
        case '"':
            if (dblquote) {
                dblquote = false;
                if (0 == pos) {
                    skip_ws = false;
                    c = 0;
                    goto store_it;
                }
            } else {
                dblquote = true;
            }
            break;

        case '#':
            comment = true;
            break;

        case '\0':
        case ';':
        case '\n':

            if (c == ';' && dblquote) {
                skip_ws = false;
                goto store_it;
            }

            sargv[sargc] = &sargmem[sargc][0];

            if (!skip_ws) {
                sargc++;
            }

            sargv[sargc] = NULL;

            if (dblquote) {
                if (errp != nullptr) {
                    *errp = SHELL_ERROR_DBL_QUOTE;
                }
                return -1;
            }

            if ((ret = do_cmd(defs, sargc, sargv)) != SHELL_CONT) {
                return ret;
            }

            memset(sargmem, 0, sizeof(sargmem));

            sargc = pos = 0;
            comment = false;
            dblquote = false;
            skip_ws = true;

            if (c == '\0') {
                return 0;
            }

            break;

        case ' ':
        case '\t':

            if (comment) {
                break;
            }

            if (dblquote) {
                skip_ws = false;
                goto store_it;
            }

            if (!skip_ws) {
                if ((sargc + 1) < SHELL_MAX_ARGV) {
                    sargv[sargc] = &sargmem[sargc][0];
                    sargc++;
                    sargv[sargc] = NULL;
                    pos = 0;
                } else {
                    if (errp != nullptr) {
                        *errp = SHELL_ERROR_TOO_MANY_ARGS;
                    }
                    return -1;
                }
                skip_ws = true;
            }
            break;

        default:

            if (comment) {
                break;
            }
            skip_ws = false;

        store_it:
            if ((pos + 1) < SHELL_MAX_ARG_LEN) {
                sargmem[sargc][pos++] = c;
                sargmem[sargc][pos] = 0;
            } else {
                if (errp != nullptr) {
                    *errp = SHELL_ERROR_ARG_TOO_LONG;
                }
                return -1;
            }
        }
        str++;
    }

    return 0;
}

void Shell::set_cmds(struct cmd_def** defs)
{
    g_cmd_defs = this->defs = defs;
}

void Shell::set_handle(void* handle)
{
    this->handle = handle;
}

void Shell::set_handle2(void* handle)
{
    this->handle2 = handle;
}

void* Shell::get_handle()
{
    return handle;
}

void* Shell::get_handle2()
{
    return handle2;
}

void Shell::set_prompt(const char* prompt)
{
    char* nprompt = strdup(prompt);
    if (NULL == nprompt) {
        perror("strdup");
        exit(1);
    }
    free(this->prompt);
    this->prompt = nprompt;
}

void Shell::loop()
{
    using_history();

    while (true) {
        char* line = NULL;

        if ((line = readline(prompt)) != nullptr) {
            enum shell_error shell_err = SHELL_ERROR_NONE;
            int ret;

            ret = parse(defs, line, &shell_err);
            if (ret == SHELL_EPARSE) {
                fprintf(stderr, "parsing: %s\n", shell_error_str(shell_err));
            } else if (ret == SHELL_RETURN) {
                return;
            }
            if (strcmp(line, "") != 0) {
                add_history(line);
            }
            free(line);
        } else {
            fprintf(stderr, "quit\n");
            return;
        }
    }
}
