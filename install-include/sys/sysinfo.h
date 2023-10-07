#ifndef _LIBRECOMPAT_SYS_INFO_H_
#define _LIBRECOMPAT_SYS_INFO_H_

#include <features.h>

#define SI_LOAD_SHIFT   16

struct sysinfo {
	long		uptime;		/* Seconds since boot */
	unsigned long	loads[3];	/* 1, 5, and 15 minute load averages */
	unsigned long	totalram;	/* Total usable main memory size */
	unsigned long	freeram;	/* Available memory size */
	unsigned long	sharedram;	/* Amount of shared memory */
	unsigned long	bufferram;	/* Memory used by buffers */
	unsigned long	totalswap;	/* Total swap space size */
	unsigned long	freeswap;	/* swap space still available */
	unsigned short	procs;		/* Number of current processes */
	unsigned long	totalhigh;	/* Total high memory size (0) */
	unsigned long	freehigh;	/* Available high memory size (0) */
	unsigned int	mem_unit;	/* Memory unit size in bytes (1) */
	char		_f[20-2*sizeof(unsigned long)-sizeof(unsigned int)];	/* padding */
};


__BEGIN_DECLS

/* Returns information on overall system statistics.  */
extern int sysinfo (struct sysinfo *__info) __THROW;

#if 0
// TODO:

/* Return number of configured processors.  */
extern int get_nprocs_conf (void) __THROW;

/* Return number of available processors.  */
extern int get_nprocs (void) __THROW;


/* Return number of physical pages of memory in the system.  */
extern long int get_phys_pages (void) __THROW;

/* Return number of available physical pages of memory in the system.  */
extern long int get_avphys_pages (void) __THROW;
#endif

__END_DECLS

#endif
