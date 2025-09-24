/* SPDX-License-Identifier: MIT */
#ifndef __BENCH_H__
#define __BENCH_H__
#include <stdint.h>
#include <stdio.h>
#include <linux/limits.h>

/* architecture dependent configuration */ 
#define PAGE_SIZE 4096
#define CACHELINE_SIZE 64
#define CACHELINE_ALIGNED __attribute__((aligned(CACHELINE_SIZE)))

/* benchmark control structures */ 
#define BENCH_ARG_BYTES (PATH_MAX * 4)
#define BENCH_PROFILE_CMD_BYTES (PATH_MAX * 2)
#define WORKER_MAX_PRIVATE 4

struct bench;
struct worker;

struct bench_operations {
	void (*report_bench)(struct bench *bench, FILE *out);
	int (*pre_work)(struct worker*);
	int (*main_work)(struct worker*);
	int (*post_work)(struct worker*);
};

struct bench {
	volatile int start;
	volatile int stop;

	int ncpu;
	int nbg;
	unsigned int duration;
	unsigned int iterations;
	int timebased;
	int	directio;
	struct worker *workers; 
	struct bench_operations ops;
	char profile_start_cmd[BENCH_PROFILE_CMD_BYTES];
	char profile_stop_cmd[BENCH_PROFILE_CMD_BYTES];
	char profile_stat_file[PATH_MAX];
	char args[BENCH_ARG_BYTES];
} CACHELINE_ALIGNED;

struct worker {
	struct bench *bench;
	int id;
	int is_bg;

	volatile int ready;
	volatile int ret;

	volatile uint64_t clocks; 
	volatile uint64_t usecs;
	volatile double   works;

	uint64_t private[WORKER_MAX_PRIVATE];
	char *page;		/*private data buffer*/
} CACHELINE_ALIGNED;

struct bench *alloc_bench(int ncpu, int nbg);
void run_bench(struct bench *bench);
void report_bench(struct bench *bench, FILE *out);

/* Helper function for loop conditions - supports both time-based and iteration-based runs */
static inline int should_continue(struct bench *bench, uint64_t iter)
{
	if (bench->timebased) {
		return !bench->stop;  /* time-based: run until alarm signal */
	} else {
		return iter < bench->iterations && !bench->stop;  /* iteration-based: run until iteration limit */
	}
}

/* cpuinfo */
extern const unsigned int PHYSICAL_CHIPS;
extern const unsigned int CORE_PER_CHIP;
extern const unsigned int SMT_LEVEL;
extern const unsigned int CACHE_PER_CORE;
extern const unsigned int seq_cores[];
extern const unsigned int rr_cores[];

/* debug stuff */
#include <stdio.h>
#define HERE() printf("%s:%d\n", __func__, __LINE__)

#endif /* __BENCH_H__ */
