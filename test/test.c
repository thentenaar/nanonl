#include <stdlib.h>
#include <check.h>

#include "nl.h"
#include "gen.h"
#include "nfqueue.h"

int main(void)
{
	SRunner *sr;
	int failed;

	/* Add test suites */
	sr = srunner_create(NULL);
	srunner_add_suite(sr, nl_suite());
	srunner_add_suite(sr, nfqueue_suite());
	srunner_add_suite(sr, gen_suite());

	/* Run them, and check for failure */
	srunner_run_all(sr, CK_ENV);
	failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

