#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <check.h>

#include <defy/nil>

#include "ny.h"
#include "error.h"
#include "alloc.h"

START_TEST(test_ny_version) {
	ck_assert(ny_version_major() == NY_VERSION_MAJOR);
	ck_assert(ny_version_minor() == NY_VERSION_MINOR);
	ck_assert(ny_version_patch() == NY_VERSION_PATCH);
} END_TEST

START_TEST(test_ny_init) {
	struct ny ny;
	int _ = ny_init(&ny);
	ck_assert(_ == 0);
	ck_assert(ny.loop != nil);
	ck_assert(ny.page_size >= 0);
} END_TEST

START_TEST(test_ny_destroy) {
	struct ny ny;
	ny_init(&ny);
	ny_destroy(&ny);
	ck_assert(ny.loop == nil);
} END_TEST

START_TEST(test_ny_error_set) {
	struct ny_error error;
	ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, ENOTSUP);
	ck_assert_str_eq(error.file, "test.c");
	ck_assert_str_eq(error.func, "test_ny_error_set");
	ck_assert(error.line == __LINE__ - 3);
	ck_assert(error.domain == NY_ERROR_DOMAIN_ERRNO);
	ck_assert(error.code == ENOTSUP);
} END_TEST

START_TEST(test_ny_error_unknown) {
	struct ny_error error;
	ny_error_set(&error, -1, -1);
	char const *str = ny_error(&error);
	ck_assert(str != nil);
	ck_assert_str_eq(str, "Unknown error");
} END_TEST

START_TEST(test_ny_error_errno) {
	struct ny_error error;
	ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, ENOTSUP);
	char const *str = ny_error(&error);
	ck_assert(str != nil);
	ck_assert(strlen(str) > 0);
} END_TEST

START_TEST(test_ny_alloc_init) {
	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	int _ = ny_alloc_init(&alloc, &ny, 1021, 23);
	ck_assert(_ == 0);
	ck_assert(alloc.raw != nil && alloc.raw != MAP_FAILED);
	ck_assert(alloc.pool == alloc.raw + ny.page_size);
	ck_assert(alloc.alloc >= 3 * ny.page_size);
	ck_assert(alloc.alloc % ny.page_size == 0);
	ck_assert(alloc.free != UINT32_MAX);
	ck_assert(alloc.size >= 4);
	ck_assert(alloc.size % sizeof (void *) == 0);
} END_TEST

START_TEST(test_ny_alloc_destroy) {
	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	ny_alloc_init(&alloc, &ny, 1021, 23);

	ny_alloc_destroy(&alloc);
	ck_assert(alloc.raw == nil);
	ck_assert(alloc.pool == nil);
	ck_assert(alloc.alloc == 0);
} END_TEST

START_TEST(test_ny_alloc_overlap) {
	uint_least32_t const num = 256;
	uint_least16_t const len = 64;

	uint8_t **ptr = calloc(num, sizeof (uint8_t *));
	ck_assert(ptr != nil);

	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	ny_alloc_init(&alloc, &ny, num, len);

	/* Acquire memory */
	for (size_t iter = 0; iter < num; ++iter) {
		ptr[iter] = ny_alloc_acquire(&alloc);
		ck_assert(ptr[iter] != nil);

		/* Check for overlaps */
		memset(ptr[iter], 0xff, len);
		for (size_t jter = 0; jter < iter; ++jter)
			for (size_t kter = 0; kter < len; ++kter)
				ck_assert(ptr[jter][kter] == 0x00);
		memset(ptr[iter], 0x00, len);
	}

	ny_alloc_destroy(&alloc);
} END_TEST

START_TEST(test_ny_alloc_linear) {
	uint_least32_t const num = 262144;
	uint_least16_t const len = 64;

	uint8_t **ptr = calloc(num, sizeof (uint8_t *));
	ck_assert(ptr != nil);

	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	ny_alloc_init(&alloc, &ny, num, len);

	/* Acquire memory */
	for (size_t iter = 0; iter < num; ++iter) {
		ptr[iter] = ny_alloc_acquire(&alloc);
		ck_assert(ptr[iter] != nil);

		memset(ptr[iter], 0x02, len);
	}

	/* Release memory */
	for (size_t iter = 0; iter < num; ++iter)
		ny_alloc_release(&alloc, ptr[iter]);

	ny_alloc_destroy(&alloc);
} END_TEST

START_TEST(test_ny_alloc_random) {
	uint_least32_t const num = 262144;
	uint_least16_t const len = 16;

	void **ptr = calloc(num, sizeof (void *));
	ck_assert(ptr != nil);

	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	ny_alloc_init(&alloc, &ny, num, len);

	for (size_t pass = 0; pass < 64; ++pass) {
		/* Random acquire */
		for (size_t iter = 0; iter < num; ++iter) {
			if (!ptr[iter] && random() % 4 == 0) {
				ptr[iter] = ny_alloc_acquire(&alloc);
				ck_assert(ptr[iter] != nil);
				memset(ptr[iter], 0x02, len);
			}
		}

		/* Random release */
		for (size_t iter = 0; iter < num; ++iter) {
			if (ptr[iter] && random() % 4 == 0) {
				ny_alloc_release(&alloc, ptr[iter]);
				ptr[iter] = nil;
			}
		}
	}

	ny_alloc_destroy(&alloc);
} END_TEST

Suite *ny_suite() {
	Suite *suite = suite_create("ny");

	TCase *tc_core = tcase_create("core");
	tcase_add_test(tc_core, test_ny_version);
	tcase_add_test(tc_core, test_ny_init);
	tcase_add_test(tc_core, test_ny_destroy);
	suite_add_tcase(suite, tc_core);

	TCase *tc_error = tcase_create("error");
	tcase_add_test(tc_error, test_ny_error_set);
	tcase_add_test(tc_error, test_ny_error_unknown);
	tcase_add_test(tc_error, test_ny_error_errno);
	suite_add_tcase(suite, tc_error);

	TCase *tc_alloc = tcase_create("alloc");
	tcase_add_test(tc_alloc, test_ny_alloc_init);
	tcase_add_test(tc_alloc, test_ny_alloc_destroy);
	tcase_add_test(tc_alloc, test_ny_alloc_overlap);
	tcase_add_test(tc_alloc, test_ny_alloc_linear);
	tcase_add_test(tc_alloc, test_ny_alloc_random);
	suite_add_tcase(suite, tc_alloc);

	return suite;
}

int main(int argc, char *argv[]) {
	Suite *suite = ny_suite();
	SRunner *runner = srunner_create(suite);
	srunner_run_all(runner, CK_VERBOSE);
	int failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
