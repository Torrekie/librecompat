#include <crt_externs.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vis.h>

#define SPT_BUFSIZE 2048

static pthread_mutex_t spt_lock = PTHREAD_MUTEX_INITIALIZER;
static int spt_space_init;
static char *spt_start;
static size_t spt_space_len;
static char *spt_progname;

static char *
spt_join_argv(char **argv, int argc)
{
	size_t need = 1;
	size_t off = 0;
	char *out;
	int i;

	if (argv == NULL || argc <= 0)
		return (NULL);

	for (i = 0; i < argc && argv[i] != NULL; i++) {
		need += strlen(argv[i]);
		if (i != 0)
			need++;
	}

	out = malloc(need);
	if (out == NULL)
		return (NULL);

	for (i = 0; i < argc && argv[i] != NULL; i++) {
		size_t n = strlen(argv[i]);

		if (i != 0)
			out[off++] = ' ';
		memcpy(out + off, argv[i], n);
		off += n;
	}
	out[off] = '\0';

	return (out);
}

static int
spt_prepare_space_locked(char **saved_argv)
{
	char ***pargvp;
	char ***penvp;
	int *pargcp;
	char **argv;
	char **envp;
	char **newenv;
	char *last;
	int argc;
	int envc;
	int i;

	if (spt_space_init)
		return (spt_start != NULL && spt_space_len > 1);

	pargvp = _NSGetArgv();
	pargcp = _NSGetArgc();
	penvp = _NSGetEnviron();
	if (pargvp == NULL || pargcp == NULL || penvp == NULL)
		return (0);

	argv = *pargvp;
	argc = *pargcp;
	envp = *penvp;
	if (argv == NULL || envp == NULL || argc <= 0 || argv[0] == NULL)
		return (0);

	for (envc = 0; envp[envc] != NULL; envc++)
		;
	newenv = calloc((size_t)envc + 1, sizeof(*newenv));
	if (newenv == NULL)
		return (0);

	for (i = 0; i < envc; i++) {
		newenv[i] = strdup(envp[i]);
		if (newenv[i] == NULL)
			break;
	}
	if (i != envc) {
		while (i-- > 0)
			free(newenv[i]);
		free(newenv);
		return (0);
	}
	newenv[envc] = NULL;
	*penvp = newenv;

	last = NULL;
	for (i = 0; i < argc && argv[i] != NULL; i++) {
		if (last == NULL || last + 1 == argv[i])
			last = argv[i] + strlen(argv[i]);
	}
	for (i = 0; envp[i] != NULL; i++) {
		if (last != NULL && last + 1 == envp[i])
			last = envp[i] + strlen(envp[i]);
	}
	if (last == NULL || last < argv[0])
		return (0);

	spt_start = argv[0];
	spt_space_len = (size_t)(last - argv[0]) + 1;
	if (spt_space_len <= 1)
		return (0);

	if (getprogname() != NULL)
		spt_progname = strdup(getprogname());
	if (spt_progname == NULL)
		spt_progname = strdup(argv[0]);

	*saved_argv = spt_join_argv(argv, argc);
	if (*saved_argv == NULL && spt_progname != NULL)
		*saved_argv = strdup(spt_progname);

	spt_space_init = 1;
	return (1);
}

static char *
setproctitle_internal(const char *fmt, va_list ap)
{
	static char *buf;
	static char *obuf;
	char ptitle[SPT_BUFSIZE];
	const char *prog;
	size_t len;
	va_list ap_copy;

	pthread_mutex_lock(&spt_lock);

	if (!spt_prepare_space_locked(&obuf))
		goto fail;

	if (buf == NULL) {
		buf = malloc(SPT_BUFSIZE);
		if (buf == NULL)
			goto fail;
	}

	prog = spt_progname != NULL ? spt_progname : "program";

	if (fmt != NULL) {
		if (fmt[0] == '-') {
			fmt++;
			buf[0] = '\0';
			len = 0;
		} else {
			(void)snprintf(buf, SPT_BUFSIZE, "%s: ", prog);
			len = strlen(buf);
		}

		va_copy(ap_copy, ap);
		(void)vsnprintf(buf + len, SPT_BUFSIZE - len, fmt, ap_copy);
		va_end(ap_copy);
	} else if (obuf != NULL && obuf[0] != '\0') {
		(void)strlcpy(buf, obuf, SPT_BUFSIZE);
	} else {
		goto fail;
	}

	(void)strnvis(ptitle, sizeof(ptitle), buf,
	    VIS_CSTYLE | VIS_NL | VIS_TAB | VIS_OCTAL);

	len = strlcpy(spt_start, ptitle, spt_space_len);
	if (len < spt_space_len)
		memset(spt_start + len, '\0', spt_space_len - len);
	else
		spt_start[spt_space_len - 1] = '\0';

	pthread_mutex_unlock(&spt_lock);
	return (spt_start);

fail:
	pthread_mutex_unlock(&spt_lock);
	return (NULL);
}

static int fast_update;

void
setproctitle_fast(const char *fmt, ...)
{
	va_list ap;
	char *title;

	va_start(ap, fmt);
	title = setproctitle_internal(fmt, ap);
	va_end(ap);

	if (title != NULL && !fast_update)
		fast_update = 1;
}

void
setproctitle(const char *fmt, ...)
{
	va_list ap;
	char *title;

	va_start(ap, fmt);
	title = setproctitle_internal(fmt, ap);
	va_end(ap);

	if (title != NULL)
		fast_update = 0;
}
