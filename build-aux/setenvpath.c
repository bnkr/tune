/*!
\file
\brief Add a custom libdir to wine's user dll loading path.

This simply adds directories to wine's internal PATH environment variable.
This is necessary because that variable is not modifyable from outside the wine
environment and batch files are too brittle to manage it.

Run like this:

  wine ./this-prog.exe [-h] [-v] [-p extra-path]... command [args]...

Remember that arguments to -p must be quoted correctly if they have spaces in
them.

Also note that arguments must be full, so -vp won't work but -v -p will.

It returns the status code of the command which is run, or 255 if something
didn't work.  Unusually, it also returns 255 for the case of -h since this is
intended as a test driver so it's probably an error if that is ever recognised.

Naturally, this program must be compiled for windows.
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#if WIN32
#  include <windows.h>
#endif

#ifndef PROGNAME
#  define PROGNAME "setenvpath.exe"
#endif

const int failure_code = 255;

void print_help() {
  printf(
    "Usage: " PROGNAME " [-h] [-v] [-p extra-path]... command [args]...\n"
    "Add -p paths to the PATH env var and run command.  -h means print help and -v means verbose.\n"
  );
}

// copy the current value of PATH (because that data can change elsewhere).
char *get_path_copy() {
  char *old_path_copy = NULL;
  const char *old_path = getenv("PATH");
  if (old_path == NULL || strcmp(old_path, "") == 0) {
    fprintf(stderr, PROGNAME ": warning: PATH environment variable does not seem to exist.\n");
  }
  else {
    old_path_copy = (char *)malloc((strlen(old_path) + 1) * sizeof(char));
    assert(old_path_copy);
    strcpy(old_path_copy, old_path);
  }
  return old_path_copy;
}

// returns "PATH=%PATH%;" - you may append to it without adding ;.
char *get_initial_putenv(const char *current_path) {
  char *ret = NULL;
  const char * const prefix = "PATH=";
  size_t total_len = strlen(prefix);
  if (current_path) {
    total_len += strlen(current_path);
    total_len += 1; // for semic
  }
  ret = (char *) malloc(total_len + 1); // for semic
  assert(ret);
  ret[0] = '\0';

  strcat(ret, prefix);
  if (current_path) {
    strcat(ret, current_path);
    strcat(ret, ";");
  }
  assert(total_len == strlen(ret));
  return ret;
}

// call putenv on the expr PATH=whatever we build and check it worked
int update_environment(char *env_expr, const char *original_value, int verbose) {
  assert(env_expr);
  if (verbose) {
    printf("Setting new path: %s\n", env_expr);
  }

  if (putenv(env_expr) != 0) {
    fprintf(stderr, PROGNAME ": error: putenv failed: %s", strerror(errno));
    return 1;
  }

  if (original_value) {
    if (strcmp(getenv("PATH"), original_value) == 0) {
      fprintf(stderr, PROGNAME ": error: putenv did not update anything.");
      return 1;
    }
  }
  return 0;
}

// check it's a dir.
int validate_path(const char *p) {
  // just so I can check it on unix
#if WIN32
  assert(p);
  DWORD ret = GetFileAttributes(p);
  if (ret == FILE_ATTRIBUTE_DIRECTORY) {
    return 0;
  }
  else {
    fprintf(stderr, PROGNAME ": error: path '%s' is not a valid directory.\n", p);
    return 1;
  }
#else
  return 0;
#endif
}

// add an extra one for spaces
size_t escaped_len(const char *v) {
  assert(v);
  size_t i = 0, extra = 0;
  while (*v) {
    ++i;
    if (*v == ' ') extra = 2;
    ++v;
  }
  return i + extra;
}

// join the args with "$arg1" "$arg2".  Almost guaranteed I missed some stuff that
// should be escaped...meh.
char *create_args(const char * const *args, int num) {
  assert(args);
  assert(num > 0);
  size_t total_len = 0;
  char *ret = NULL;
  size_t ret_i = 0;
  int i;
  for (i = 0; i < num; ++i) {
    const char *arg = args[i];
    assert(arg);
    total_len += strlen(arg) + 3; // space or the trailing null and two '
  }
  ret = (char *) malloc(total_len);
  assert(ret);

  for (i = 0; i < num; ++i) {
    const char *arg = args[i];
    ret[ret_i++] = '"';
    while (*arg) {
      ret[ret_i++] = *arg;
      ++arg;
    }
    ret[ret_i++] = '"';
    ret[ret_i++] = ' ';
  }
  ret[ret_i-1] = '\0';

  assert(strlen(ret) == total_len - 1); // because we included the null.
  assert(strlen(ret) == ret_i - 1);
  return ret;
}

int run_process(const char *cmd, char *args) {
  WIN32_FIND_DATA fd;
  HANDLE h = FindFirstFile(
    cmd, // LPCTSTR lpFileName,
    &fd  // LPWIN32_FIND_DATA lpFindFileData
  );

  if (h == INVALID_HANDLE_VALUE) {
    fprintf(stderr, PROGNAME ": error: command '%s' does not appear to exist.\n", cmd);
    return 255;
  }

  assert(cmd);
  size_t cmd_len = strlen(cmd);
  size_t args_len = (args) ? strlen(args) + 1 : 0; // space between cmd and args.
  char *shell = (char *)malloc(cmd_len + args_len + 1);
  assert(shell);
  shell[0] = '\0';
  strcat(shell, cmd);
  if (args) {
    strcat(shell, " ");
    strcat(shell, args);
  }

  int ret = system(shell);

  return ret;
}


int main(const int argc, const char *const *const argv) {
  int verbose = 0;
  int args_start = 0;
  int exit_status = failure_code;
  char *old_path_copy = get_path_copy();
  assert(old_path_copy);
  char *putenv_expression = NULL;
  char *command_line = NULL;
  const char *cmd = NULL;
  int i;

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0) {
      print_help();
      goto error_exit;
    }
    else if (strcmp(argv[i], "-v") == 0) {
      verbose = 1;
    }
    else if (strcmp(argv[i], "-p") == 0) {
      ++i;
      if (i == argc) {
        fprintf(stderr, PROGNAME ": error: -p requires an argument.\n");
        print_help();
        goto error_exit;
      }

      const char *const path = argv[i];
      if (validate_path(path) != 0) {
        goto error_exit;
      }

      size_t total_len = 0;
      if (putenv_expression == NULL) {
        putenv_expression = get_initial_putenv(old_path_copy);
        assert(putenv_expression);

        total_len = strlen(putenv_expression) + strlen(path);
        putenv_expression = (char*) realloc(putenv_expression, (total_len + 1) * sizeof(char));
        assert(putenv_expression);
      }
      else {
        total_len += strlen(putenv_expression) + 1 + strlen(path);
        putenv_expression = (char*) realloc(putenv_expression, (total_len + 1) * sizeof(char));
        assert(putenv_expression);
        strcat(putenv_expression, ";");
      }

      strcat(putenv_expression, path);
      assert(strlen(putenv_expression) == total_len);
    }
    else {
      cmd = argv[i];
      args_start = ++i;
      break;
    }
  }

  if (putenv_expression != NULL) {
    if (update_environment(putenv_expression, old_path_copy, verbose) != 0) {
      goto error_exit;
    }
  }

  if (cmd == NULL) {
    fprintf(stderr, PROGNAME ": error: no command given.\n");
    print_help();
    goto error_exit;
  }

  if (args_start < argc) {
    command_line = create_args(&argv[args_start], argc - args_start);
    assert(command_line);
  }

  if (verbose) {
    printf("Run '%s' with argstring '%s'.\n", cmd, command_line);
  }

  exit_status = run_process(cmd, command_line);

error_exit:
  if (old_path_copy) free(old_path_copy);
  if (putenv_expression) free(putenv_expression);
  if (command_line) free(command_line);

  return exit_status;
}
