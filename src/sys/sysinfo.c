#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/task_info.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdbool.h>

#if defined(__APPLE__) && !defined(_KERNEL)
// If we do FreeBSD support in future, we will need to provide these functions in userspace (they are in kernel)
#ifdef PAGE_SIZE
#undef PAGE_SIZE
#endif

static inline void
swap_pager_status(int *total, int *used)
{
	vm_size_t PAGE_SIZE;
	struct xsw_usage swap;
	mach_port_t mach_port;
	size_t len = sizeof(swap);
	host_page_size(mach_port, &PAGE_SIZE);
	if (sysctlbyname("vm.swapusage", &swap, &len, NULL, 0) != 0) {
		*total = 0;
		*used = 0;
	} else {
		*total = swap.xsu_total / PAGE_SIZE;
		*used = (swap.xsu_total - swap.xsu_avail) / PAGE_SIZE;
	}
	return;
}
#endif

struct sysinfo {
	long		uptime;		/* Seconds since boot */
	unsigned long	loads[3];	/* 1, 5, and 15 minute load averages */
#define LINUX_SYSINFO_LOADS_SCALE 65536
	unsigned long	totalram;	/* Total usable main memory size */
	unsigned long	freeram;	/* Available memory size */
	unsigned long	sharedram;	/* Amount of shared memory */
	unsigned long	bufferram;	/* Memory used by buffers */
	unsigned long	totalswap;	/* Total swap space size */
	unsigned long	freeswap;	/* swap space still available */
	unsigned short	procs;		/* Number of current processes */
	unsigned long	totalhigh;
	unsigned long	freehigh;
	unsigned int	mem_unit;
	char		_f[20-2*sizeof(unsigned long)-sizeof(unsigned int)];	/* padding */
};

int
sysinfo(struct sysinfo *args)
{
	struct sysinfo sysinfo;
	int i, j;
	struct timespec ts;
#ifndef NO_SYSCTL
	int mib[2];
	size_t len;
#endif
	// PAGE_SIZE defined in sys/pipes.h but we prefer to get it at runtime although it always be 16384
	vm_size_t PAGE_SIZE;
	mach_port_t mach_port;
	mach_msg_type_number_t count;
	vm_statistics64_data_t vm_stats;
	bool guessed = false;
	mach_port = mach_host_self();
	count = sizeof(vm_stats) / sizeof(natural_t);

	host_page_size(mach_port, &PAGE_SIZE); // This function only fails when invalid arguments

	if (host_statistics64(mach_port, HOST_VM_INFO64, (host_info64_t)&vm_stats, &count) != KERN_SUCCESS) {
		guessed = true;
	}

	bzero(&sysinfo, sizeof(sysinfo));
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	if (ts.tv_nsec != 0)
		ts.tv_sec++;

	sysinfo.uptime = ts.tv_sec;

#ifndef _KERNEL
	/* averunnable is not exposed to userspace */
	struct loadavg averunnable;
#ifndef NO_SYSCTL
	mib[0] = CTL_VM;
	mib[1] = VM_LOADAVG;
	len = sizeof(averunnable);
	sysctl(mib, 2, &averunnable, &len, NULL, 0);
#else
	getloadavg((double []){averunnable.ldavg[0], averunnable.ldavg[1], averunnable.ldavg[2]}, 3);
	/* Already devided by averunnable.fscale inside getloadavg impl */
	averunnable.fscale = 1;
#endif // NO_SYSCTL
#endif // _KERNEL
	/* Use the information from the mib to get our load averages */
	for (i = 0; i < 3; i++)
		sysinfo.loads[i] = averunnable.ldavg[i] *
		    LINUX_SYSINFO_LOADS_SCALE / averunnable.fscale;

	sysinfo.freeram = 0;
	sysinfo.totalram = 0;
	/*
	 * On Darwin we don't have global variable 'physmem', but same thing
	 * was provided by hw.physmem or hw.memsize
	 * Maybe we can add this to librecompat?
	 */
	if (!guessed) {
#ifndef NO_SYSCTL
		mib[0] = CTL_HW;
		mib[1] = HW_MEMSIZE; // Why not HW_PHYSMEM? I've referred to many other implementations and they all use memsize
		len = sizeof(sysinfo.totalram);
		sysctl(mib, 2, &sysinfo.totalram, &len, NULL, 0);

		len = sizeof(sysinfo.freeram);
		// VM_PAGE_FREE_COUNT does not having fixed mib
		sysctlbyname("vm.page_free_count", &sysinfo.freeram, &len, NULL, 0);
#else
		sysinfo.freeram = (unsigned long)(vm_stats.free_count + vm_stats.inactive_count) * PAGE_SIZE;
		sysinfo.totalram = ((vm_stats.active_count + vm_stats.wire_count) * PAGE_SIZE) + sysinfo.freeram;
#endif
	} else {
		// Keep 100MB
		sysinfo.freeram = (100 * 1024 * 1024);
		sysinfo.totalram = (100 * 1024 * 1024);
	}

	/*
	 * sharedram counts pages allocated to named, swap-backed objects such
	 * as shared memory segments and tmpfs files.  There is no cheap way to
	 * compute this, so just leave the field unpopulated.  Linux itself only
	 * started setting this field in the 3.x timeframe.
	 */
	sysinfo.sharedram = 0;
	sysinfo.bufferram = 0;

//	sysinfo.sharedram = vm_stats.wire_count * PAGE_SIZE;
	struct task_basic_info_64 task_info_64;
	mach_msg_type_number_t task_info_64_count = TASK_BASIC_INFO_64_COUNT;
	if (task_info(mach_task_self(), TASK_BASIC_INFO_64, (task_info_t)&task_info_64, &task_info_64_count) == KERN_SUCCESS) {
		sysinfo.sharedram = task_info_64.virtual_size;
		sysinfo.bufferram = task_info_64.resident_size;
	}

	swap_pager_status(&i, &j);
	sysinfo.totalswap = i * PAGE_SIZE;
	sysinfo.freeswap = (i - j) * PAGE_SIZE;

	sysinfo.procs = 0;
	int mib3[3] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
	len = 0;
	if (sysctl(mib3, sizeof(mib3) / sizeof(mib3[0]), NULL, &len, NULL, 0) == 0) {
		sysinfo.procs = (len / sizeof(struct kinfo_proc));
	}

	/*
	 * Platforms supported by the emulation layer do not have a notion of
	 * high memory.
	 */
	sysinfo.totalhigh = 0;
	sysinfo.freehigh = 0;

	sysinfo.mem_unit = 1;

	*args = sysinfo;
	return 0;
}
