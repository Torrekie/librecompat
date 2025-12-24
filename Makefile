TARGET = librecompat.0.dylib
CC ?= ccios
CFLAGS = -isystem build-include/ -I src/ -DPRIVATE -Os -flto
LDFLAGS += -flto -Wl,-dead_strip
INSTALL_RPATH = -rpath /usr/lib
ifeq ($(ROOTLESS),1)
CFLAGS += -DLIBRECOMPAT_ROOTLESS=1
INSTALL_RPATH += -rpath /var/jb/usr/lib
endif
LDFLAGS= -dynamiclib $(INSTALL_RPATH) \
         -install_name @rpath/librecompat.0.dylib \
         -Wl,-reexport_library,libiosexec.tbd \
         -Wl,-reexport_library,libSystem.tbd

SRCS=   src/argz/argz-addsep.c \
        src/argz/argz-append.c \
        src/argz/argz-count.c \
        src/argz/argz-create.c \
        src/argz/argz-ctsep.c \
        src/argz/argz-delete.c \
        src/argz/argz-extract.c \
        src/argz/argz-insert.c \
        src/argz/argz-next.c \
        src/argz/argz-replace.c \
        src/argz/argz-stringify.c \
        src/dirent/versionsort.c \
        src/dirent/versionsort64.c \
        src/fcntl/posix_fadvise.c \
        src/fcntl/posix_fallocate.c \
        src/malloc/dynarray/dynarray_at_failure.c \
        src/malloc/dynarray/dynarray_emplace_enlarge.c \
        src/malloc/dynarray/dynarray_finalize.c \
        src/malloc/dynarray/dynarray_resize.c \
        src/malloc/dynarray/dynarray_resize_clear.c \
        src/musl/network/lookup_serv.c \
        src/netdb/gethostbyname_r.c \
        src/netdb/getservbyname_r.c \
        src/netdb/getservbyport_r.c \
        src/netdb/servent_r.c \
        src/poll/ppoll.c \
        src/regex/regcomp.c \
        src/regex/regex_internal.c \
        src/regex/regexec.c \
        src/search/hsearch_r.c \
        src/search/tsearch.c \
        src/stdlib/init_misc.c \
        src/stdio/fclose.c \
        src/stdio/flags.c \
        src/stdio/fopencookie.c \
        src/string/mempcpy.c \
        src/string/memrchr.c \
        src/string/strchrnul.c \
        src/string/strverscmp.c \
        src/sys/socket/accept4.c \
        src/sys/socket/sendfile.c \
        src/sys/sysinfo.c \
        src/unistd/fdatasync.c \
        src/unistd/get_current_dir_name.c \
        src/unistd/getopt.c \
        src/unistd/getopt1.c \
        src/wordexp/wordexp.c

ifeq ($(ROOTLESS),1)
SRCS += src/locale/ascii.c \
	src/locale/big5.c \
	src/locale/collate.c \
	src/locale/euc.c \
	src/locale/gb18030.c \
	src/locale/gb2312.c \
	src/locale/gbk.c \
	src/locale/ldpart.c \
	src/locale/lmessages.c \
	src/locale/lmonetary.c \
	src/locale/lnumeric.c \
	src/locale/mbsnrtowcs.c \
	src/locale/mskanji.c \
	src/locale/none.c \
	src/locale/setlocale.c \
	src/locale/setrunelocale.c \
	src/locale/table.c \
	src/locale/timelocal.c \
	src/locale/utf2.c \
	src/locale/utf8.c \
	src/locale/wcsnrtombs.c \
	src/locale/xlocale.c \
	src/sysctl/sysctl.c \
	src/unistd/getusershell.c \
	src/stdio/popen.c
endif

OBJS=   ${SRCS:.c=.o}

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f ${OBJS} ${TARGET}
