TARGET = librecompat.0.dylib
STATIC_TARGET = librecompat.a
CC ?= ccios
AR ?= ar
CFLAGS = -isystem build-include/ -I src/ -DPRIVATE
ifeq ($(DEBUG),1)
CFLAGS += -O0 -g -gdwarf
LDFLAGS += -O0 -g -gdwarf
else
CFLAGS += -Os -flto
LDFLAGS +=  -Wl,-dead_strip
endif

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
        src/malloc/dynarray_at_failure.c \
        src/malloc/dynarray_emplace_enlarge.c \
        src/malloc/dynarray_finalize.c \
        src/malloc/dynarray_resize.c \
        src/malloc/dynarray_resize_clear.c \
        src/musl/network/lookup_serv.c \
        src/netdb/gethostbyname_r.c \
        src/netdb/getservbyname_r.c \
        src/netdb/getservbyport_r.c \
        src/netdb/servent_r.c \
        src/poll/ppoll.c \
        src/regex/regex.c \
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
        src/sys/utsname/uname.c \
        src/unistd/fdatasync.c \
        src/unistd/get_current_dir_name.c \
        src/unistd/getopt.c \
        src/unistd/getopt1.c \
        src/wordexp/wordexp.c

ifeq ($(ROOTLESS),1)
SRCS += src/locale/setlocale.c \
	src/sysctl/sysctl.c \
	src/unistd/confstr.c \
	src/unistd/getusershell.c \
	src/stdio/popen.c
endif

OBJS=   ${SRCS:.c=.o}

all: ${TARGET} ${STATIC_TARGET}

${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS}
	ldid -S $@

${STATIC_TARGET}: ${OBJS}
	${AR} rcs $@ ${OBJS}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f ${OBJS} ${TARGET} ${STATIC_TARGET}

ifeq ($(ROOTLESS),1)
PREFIX = /var/jb/usr
else
PREFIX = /usr
endif

install: all
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/lib/
	ln -sf $(TARGET) $(DESTDIR)$(PREFIX)/lib/librecompat.dylib
	install -m 644 $(STATIC_TARGET) $(DESTDIR)$(PREFIX)/lib/
	mkdir -p $(DESTDIR)$(PREFIX)/include/librecompat
	cp -R install-include/* $(DESTDIR)$(PREFIX)/include/librecompat/
