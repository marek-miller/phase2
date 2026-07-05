/*
 * t-ph2run -- CLI-level meta-test for the ph2run binary.
 *
 * Regression: the CSV summary printed after a run reported
 * n_ranks = 0 because main() never populated the file-static
 * world_info snapshot.  Run one Trotter step on the water
 * fixture as an MPI singleton and assert the summary row
 * carries n_ranks = 1 (plus fixture sanity fields).
 *
 * Like t-run, this test does not world_init itself: the
 * child owns the MPI world.  Invoked by the runner with
 * cwd = repo root, so ./build and ./test paths resolve.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "test.h"
#include "t-data.h"

#define PH2RUN_BIN "./build/ph2run/ph2run"

int main(void)
{
	/*
	 * Under `make check-mpi` this test itself runs inside
	 * mpirun; an exec'd ph2run would inherit the OMPI/PMIX
	 * environment and try to join the parent job instead
	 * of starting a singleton.  Serial-mode only, like the
	 * Python oracle tests.
	 */
	if (getenv("OMPI_COMM_WORLD_SIZE") != NULL)
		return 0;

	char tmp[] = "/tmp/t-ph2run-XXXXXX";
	const int fd = mkstemp(tmp);
	TEST_ASSERT(fd >= 0, "mkstemp");
	close(fd);
	test_fixture_copy(TEST_DATA[0].filename, tmp);

	char cmd[512];
	const int n = snprintf(cmd, sizeof cmd,
		"PHASE2_LOG=info " PH2RUN_BIN
		" -S %s trott -D 0.1 -s 1 2>&1", tmp);
	TEST_ASSERT(n > 0 && (size_t)n < sizeof cmd, "command too long");

	FILE *p = popen(cmd, "r");
	TEST_ASSERT(p != NULL, "popen: %s", cmd);

	char buf[16384];
	size_t len = 0, got;
	while (len + 1 < sizeof buf &&
	       (got = fread(buf + len, 1, sizeof buf - 1 - len, p)) > 0)
		len += got;
	buf[len] = '\0';

	const int rc = pclose(p);
	TEST_ASSERT(WIFEXITED(rc) && WEXITSTATUS(rc) == 0,
		"ph2run exited non-zero; output:\n%.2000s", buf);
	unlink(tmp);

	/* Summary row follows the CSV header line. */
	const char *hdr = strstr(buf,
		"n_qb,n_terms,n_dets,delta,n_steps,n_ranks,t_tot");
	TEST_ASSERT(hdr != NULL, "CSV header not found; output:\n%.2000s",
		buf);
	const char *row = strstr(hdr, "\n");
	row = row ? strstr(row, "> ") : NULL;
	TEST_ASSERT(row != NULL, "CSV row not found; output:\n%.2000s", buf);

	unsigned n_qb;
	size_t n_terms, n_dets, n_steps;
	double delta, t_tot;
	int n_ranks;
	TEST_ASSERT(sscanf(row + 2, "%u,%zu,%zu,%lf,%zu,%d,%lf", &n_qb,
			&n_terms, &n_dets, &delta, &n_steps, &n_ranks,
			&t_tot) == 7,
		"CSV row malformed: %.200s", row);

	TEST_EQ(n_qb, TEST_DATA[0].num_qubits);
	TEST_EQ(n_terms, TEST_DATA[0].num_terms);
	TEST_EQ(n_steps, (size_t)1);
	TEST_EQ(n_ranks, 1);	/* singleton run; was 0 pre-fix */

	return 0;
}
