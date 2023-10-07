# librecompat

Reimplemented userspace libc compatibility layer for Darwin.

***
## What and Why?

`librecompat` is designed for userspace C API compatibility on Darwin systems (macOS/iOS) and others (?further future), while Apple XNU & Libc only provide functions that meeting POSIX standards, few BSD extensions being added by Apple with 'Apple Concept'.

Since Open-Source world has many contributors that using different C implementations, some of those codes were not portable, which causing feature delimitations in our environments. As myself is maintaining an APT repository for jailbroken iOS devices, finding code compatibilities is essential for softwares to work as how they expected on other systems.

## How you impl it?

For some simple C extensions I could just do it by myself (like `pipe2` which just operating fds on `pipe`), but most scene we are facing complex problems (e.g. `re_compile_*` from GNU C) that needs porting their original implementations to Darwin with patches, I KNOW this could cause some sort of licensing problems, but for current stage I want to make sure those compat functions is working as expected before dealing with these licenses.

### Current orig sources this library uses

- [GNU C Library](https://www.gnu.org/software/libc/)  (4.4 BSD parts)
- [musl libc](https://musl.libc.org) (MIT parts)
- [FreeBSD](https://github.com/freebsd/freebsd-src) (4.4 BSD)
- [NetBSD](https://github.com/NetBSD/src) (3-clause BSD)
- [Apple XNU](https://github.com/apple-oss-distributions/xnu/) (APSL 2.0)
- [Apple Libc](https://github.com/apple-oss-distributions/Libc/) (APSL 2.0)

Currently all 'stolen' codes are having their licenses included in source files, all other codes that not mentioned above is written by me, Torrekie Gen, which owns this project.

## Installation (TODO)

`librecompat` uses GNU Autotools to manage the configuration processes.

This library only works on Darwin currently. Target Darwin version has to be specified during configure stage.
```bash
# Default configurations
./configure

# Targeting pre Darwin 20 (pre macOS 11/iOS 14), which configured to provide post Darwin 19 functions
./configure --with-target-darwin-version=19 

# Specify which part of code is being enabled/disabled
./configure --with-regex --without-string --without-musl --with-glibc --with-fbsd --with-apple

# Code with specified licenses is being disabled
./configure --without-gpl --without-apsl

# Compile librecompat.dylib to work as libSystem/libc
./configure --with-libsystem-reexport

# Compile supporting libraries and do reexport
./configure --with-librecompat-objc --with-libobjc-reexport

# Build
make -j8

# Install
make install
```

## Use librecompat

### Use in iPhoneOS.sdk to replace libSystem
```bash
# Create SDK copy
rsync -Kah /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/ /usr/local/share/SDKs/iPhoneOS.sdk/
# Replace libSystem install name 
sed -i "s|'/usr/lib/libSystem.B.dylib'|'/usr/local/lib/librecompat.0.dylib'|g"
# Append librecompat symbols to libSystem.tbd
sed -i "s|___crashreporter_info__, |$(llvm-nm librecompat.dylib -U --extern-only --just-symbol-name | xargs -L1 printf "%s, ")___crashreporter_info__, |g" /usr/local/share/SDKs/iPhoneOS.sdk/usr/lib/libSystem.tbd
# Well done, use your modified SDK
export SDKROOT=/usr/local/share/SDKs/iPhoneOS.sdk
# Use librecompat headers
cc -I/usr/local/include/librecompat test.c
```

## Currently provided functions

### From GNU C Library
- [x] `<argz.h>` `<string/argz.h>`
- [x] `<getopt.h>` and it's relatives
- [x] `<regex.h>`
- [x] `<features.h>`
- [x] `<malloc/dynarray.h>`
- [x] `<misc/sys/cdefs.h>`
- [x] `<bits/long-double.h>` `<bits/wordsize.h>`
- [x] `<stdc-predef.h>`
- [ ] `mremap` in `<sys/mman.h>`
- [x] `__set_errno` in `<errno.h>`
- [x] `_Static_assert` in `<sys/cdefs.h>`
### From musl libc
- [x] `getservbyname_r`, `getservbyport_r` in `<netdb.h>`
### From FreeBSD
- [x] `fopencookie`, `fdclose` in `<stdio.h>`
- [x] `mempcpy` in `<string.h>`
### From NetBSD
- [x] `setservent_r`, `getservent_r`, `endservent_r` in `<netdb.h>`
### From Apple Libc
- [x] `reallocarray`, `reallocarrayf` that hidden by Apple, we define it in `<stdlib.h>`
- [x] `<wordexp.h>` that not compiled to iOS libc
- [x] `strtonum` (for pre Darwin 20)
### From librecompat
- [x] Linux compatible `<sys/sysinfo.h>` 
- [x] `gethostbyname_r` in `<netdb.h>`
- [x] `posix_fadvise`, `posix_fallocate` in `<fcntl.h>`
- [x] GLibc compatible `get_current_dir_name` in `<unistd.h>`
- [x] GLibc compatible `program_invocation_short_name` in `<errno.h>`
- [x] `fdatasync` in `<unistd.h>`
- [x] `accept4` in `<sys/socket.h>`
- [x] `ppoll` in `<poll.h>`
- [x] `strchrnul` in `<string.h>`

## Limitations

- As this library implement functions in pure userspace, some of them might not having desired efficiency than others does (like `accept4`).
- While not building with re-export configured, you will have to add linker flags to ensure symbols can be found by linker.
- While building with re-export configured, you will have to edit original libSystem.tbd to ensure librecompat is used as libSystem, and symbols should also de defined in tbd.
- You should not use `librecompat` while system shipped functions already meets the required conditions.

## TODO

See [TODO](TODO)

## LICENSE

This project is mixed license, all codes from `librecompat` is licensed under BSD 2-Clause.

## Disclaimer

THIS LIBRARY IS NOT FOR PRODUCTION, USE IT AT YOUR OWN RISK!
