#include <stdio.h>

#include "test.h"
#include "term_shapes.h"
#include "init.h"

int test_checks_run;
int test_checks_failed;

void
test_fail(const char *expr, const char *file, int line)
{
	test_checks_failed++;
	printf("  %s:%d: CHECK(%s) failed\n", file, line, expr);
}

void
test_fail_eq(const char *expr, long actual, long expected,
	     const char *file, int line)
{
	test_checks_failed++;
	printf("  %s:%d: %s == %ld, expected %ld\n",
	       file, line, expr, actual, expected);
}

void
test_fail_near(const char *expr, double actual, double expected,
	       const char *file, int line)
{
	test_checks_failed++;
	printf("  %s:%d: %s == %.17g, expected %.17g\n",
	       file, line, expr, actual, expected);
}

int
test_shape_load(char *path, struct shape *s)
{
	return init_from_file(path, s);
}

void
test_shape_unload(struct shape *s)
{
	destroy_shape(s);

	/*
	 * init_from_file() opens a log file that nothing ever closes. without
	 * this every shape a test loads is reported as a leak under
	 * -fsanitize=address, which would drown out the one leak we care about
	 */
	if (s->log != NULL) {
		fclose(s->log);
		s->log = NULL;
	}
}

struct suite {
	const char *name;
	void (*fn)(void);
};

static const struct suite suites[] = {
	{"vector", suite_vector},
	{"occlusion", suite_occlusion},
	{"raster", suite_raster},
	{"init", suite_init}
};

int
main(void)
{
	size_t i;
	int run_before, failed_before, ran, failed;

	for (i = 0; i < sizeof(suites) / sizeof(suites[0]); ++i) {
		run_before = test_checks_run;
		failed_before = test_checks_failed;

		suites[i].fn();

		ran = test_checks_run - run_before;
		failed = test_checks_failed - failed_before;

		printf("%-12s %d/%d\n", suites[i].name, ran - failed, ran);
	}

	printf("\n%s: %d passed, %d failed\n",
	       test_checks_failed ? "FAIL" : "OK",
	       test_checks_run - test_checks_failed, test_checks_failed);

	return test_checks_failed != 0;
}
