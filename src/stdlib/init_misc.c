#include <string.h>
#include <libgen.h>
#include <crt_externs.h>

char *__progname_full = (char *) "";

char *program_invocation_name = (char *) "";
char *program_invocation_short_name = (char *) "";

// I swear this is the most weird Apple design ever
extern char ***_NSGetArgv();

extern char *__progname;

__attribute__((constructor))
static void _init_prognames(void) {
	__progname_full = *(*_NSGetArgv());
	program_invocation_name = __progname_full;
	program_invocation_short_name = __progname;
}
