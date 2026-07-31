#ifndef TEST_H
#define TEST_H

#include "glyphedron.h"

/* running totals for the whole binary; test.c owns the definitions */
extern int test_checks_run;
extern int test_checks_failed;

void test_fail(const char *expr, const char *file, int line);
void test_fail_eq(const char *expr, long actual, long expected,
		  const char *file, int line);
void test_fail_near(const char *expr, double actual, double expected,
		    const char *file, int line);

#define CHECK(cond)							\
	do {								\
		test_checks_run++;					\
		if (!(cond)) {						\
			test_fail(#cond, __FILE__, __LINE__);		\
		}							\
	} while (0)

#define CHECK_EQ(actual, expected)					\
	do {								\
		long check_a = (long) (actual);				\
		long check_e = (long) (expected);			\
									\
		test_checks_run++;					\
		if (check_a != check_e) {				\
			test_fail_eq(#actual, check_a, check_e,		\
				     __FILE__, __LINE__);		\
		}							\
	} while (0)

#define CHECK_NEAR(actual, expected, eps)				\
	do {								\
		double check_a = (double) (actual);			\
		double check_e = (double) (expected);			\
									\
		test_checks_run++;					\
		if (!(fabs(check_a - check_e) <= (eps))) {		\
			test_fail_near(#actual, check_a, check_e,	\
				       __FILE__, __LINE__);		\
		}							\
	} while (0)

/*
 * loads a shape for a test. path must have static storage duration, because
 * init_from_file() keeps the pointer in s->fname
 */
int test_shape_load(char *path, struct shape *s);
void test_shape_unload(struct shape *s);

/*
 * brackets a call that is expected to report a parse error on stderr, so a
 * passing run does not print diagnostics that look like something went wrong.
 * failed checks are unaffected: they are written to stdout
 *
 * these do nothing under a sanitizer build, because AddressSanitizer writes
 * its reports to the same file descriptor and losing one would defeat the
 * coverage that only make test-asan provides
 */
void test_silence_stderr(void);
void test_restore_stderr(void);

/* suites, one per test_*.c file */
void suite_vector(void);
void suite_occlusion(void);
void suite_raster(void);
void suite_init(void);

#endif /* TEST_H */
