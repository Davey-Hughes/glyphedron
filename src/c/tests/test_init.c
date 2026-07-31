#include "test.h"
#include "term_shapes.h"
#include "init.h"

#define CUBE "./shapes/platonic_solids/cube.txt"
#define BAD_FACE "./tests/fixtures/bad_face_index.txt"
#define AT_BOUND "./tests/fixtures/face_index_at_bound.txt"

static void
test_parses_a_good_file(void)
{
	struct shape s;

	CHECK_EQ(test_shape_load(CUBE, &s), 0);

	CHECK_EQ(s.num_v, 8);
	CHECK_EQ(s.num_e, 12);
	CHECK_EQ(s.num_f, 6);

	/* every face of a cube is a quadrilateral */
	CHECK_EQ(s.faces[0].num_v, 4);
	CHECK_EQ(s.faces[5].num_v, 4);

	test_shape_unload(&s);
}

/*
 * a face index out of bounds must fail cleanly. the cleanup path frees the
 * face array of every declared face, but only the faces parsed so far have
 * been allocated, so the rest are freed from uninitialised memory
 *
 * this only bites under make test-asan, where AddressSanitizer fills every
 * fresh allocation with 0xbe and freeing that faults. a plain build usually
 * frees the garbage in silence, so there the -1 below is the whole assertion
 */
static void
test_bad_face_index_fails_cleanly(void)
{
	struct shape s;

	test_silence_stderr();
	CHECK_EQ(init_from_file(BAD_FACE, &s), -1);
	test_restore_stderr();
}

/*
 * valid vertex indices run from 0 to num_v - 1, so a face naming num_v itself
 * must be rejected. accepting it stored the index and let the normal
 * calculation dereference one element past the vertex array. the edge bounds
 * check in the same file already rejected the equivalent index
 */
static void
test_face_index_one_past_end_rejected(void)
{
	struct shape s;
	int err;

	test_silence_stderr();
	err = init_from_file(AT_BOUND, &s);
	test_restore_stderr();

	CHECK_EQ(err, -1);

	/*
	 * if the bounds check wrongly accepted the index the shape did load,
	 * so unload it to keep a failing run's output free of leak noise
	 */
	if (err == 0) {
		test_shape_unload(&s);
	}
}

void
suite_init(void)
{
	test_parses_a_good_file();
	test_bad_face_index_fails_cleanly();
	test_face_index_one_past_end_rejected();
}
