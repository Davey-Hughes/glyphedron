#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "test.h"
#include "glyphedron.h"
#include "init.h"

/*
 * AddressSanitizer writes its reports to stderr, so silencing that descriptor
 * under a sanitizer build would hide the very failures make test-asan exists
 * to catch. clang exposes __has_feature; anything else is treated as unsanitized
 */
#if defined(__has_feature)
# if __has_feature(address_sanitizer)
#  define TEST_SANITIZED 1
# endif
#endif

#ifndef TEST_SANITIZED
# define TEST_SANITIZED 0
#endif

int test_checks_run;
int test_checks_failed;

/* the real stderr while it is redirected, or -1 when it is not */
static int test_saved_stderr = -1;

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

void
test_silence_stderr(void)
{
	int devnull;

	if (TEST_SANITIZED) {
		return;
	}

	fflush(stderr);

	test_saved_stderr = dup(STDERR_FILENO);
	if (test_saved_stderr < 0) {
		return;
	}

	devnull = open("/dev/null", O_WRONLY);
	if (devnull < 0) {
		close(test_saved_stderr);
		test_saved_stderr = -1;
		return;
	}

	dup2(devnull, STDERR_FILENO);
	close(devnull);
}

void
test_restore_stderr(void)
{
	if (test_saved_stderr < 0) {
		return;
	}

	fflush(stderr);
	dup2(test_saved_stderr, STDERR_FILENO);
	close(test_saved_stderr);
	test_saved_stderr = -1;
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
