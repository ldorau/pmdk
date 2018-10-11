/*
 * Copyright 2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * obj_verify.c -- tool for creating and verifying a pmemobj pool
 */

#include <stddef.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "libpmemobj.h"
#include "set.h"

#define SIGNATURE_LEN 10
#define NUMBER_LEN 10
#define FILL_SIZE 245 /* so that size of one record is 1024 bytes */
#define SKIP_OFFSET offsetof(struct data_s, checksum)

#define POOL_MIN_SIZE	8388608
#define PART_MIN_SIZE	2097152

static const char *Signature = "OBJ_VERIFY";

POBJ_LAYOUT_BEGIN(obj_verify);
POBJ_LAYOUT_ROOT(obj_verify, struct root_s);
POBJ_LAYOUT_ROOT(obj_verify, struct data_s);
POBJ_LAYOUT_END(obj_verify);

struct data_s {
	char signature[SIGNATURE_LEN];
	char number_str[NUMBER_LEN];
	uint64_t number;
	uint32_t fill[FILL_SIZE];
	uint64_t checksum;
};

struct root_s {
	uint64_t count;
};

/*
 * record_constructor -- constructor of a list element
 */
static int
record_constructor()
{
	PMEMobjpool *pop = NULL;
	void *ptr = NULL;
	void *arg = NULL;

	struct data_s *rec = (struct data_s *)ptr;
	uint64_t *count = arg;

	memcpy(rec->signature, Signature, sizeof(rec->signature));
	snprintf(rec->number_str, NUMBER_LEN, "%09lu", *count);
	rec->number = *count;

	for (int i = 0; i < FILL_SIZE; i++)
		rec->fill[i] = (uint32_t)rand();

	util_checksum(rec, sizeof(*rec), &rec->checksum,
			1 /* insert */, SKIP_OFFSET);

	pmemobj_persist(pop, rec, sizeof(*rec));

	(*count)++;

	pmemobj_persist(pop, count, sizeof(*count));

	return 0;
}

/*
 * do_create -- (internal) create a pool to be verified
 */
static void
do_create(const char *path)
{
	struct pool_set *set;
	struct pool_attr attr;
	uint64_t count = 0;

	srand((unsigned int)time(NULL));

	memset(&attr, 0, sizeof(attr));

	if (util_pool_create(&set, path, 0, POOL_MIN_SIZE, PART_MIN_SIZE,
						&attr, NULL, 1)) {
		if (errno != EEXIST) {
			out("!%s: util_pool_create: %s",
				path, out_get_errormsg());
			exit(-1);
		}

		if (util_pool_open(&set, path, PART_MIN_SIZE,
					NULL, 0, NULL, 0)) {
			out("!%s: util_pool_open: %s",
				path, out_get_errormsg());
			exit(-1);
		}
	}

	ASSERT(set->nreplicas > 0);

	out("create(%s): allocating records in the pool ...", path);

	out("* 1");
	size_t size = set->poolsize;
	out("* 2");
	void *addr = set->replica[0]->part[0].addr;
	out("* 3");
	ASSERT(set->poolsize <= set->replica[0]->repsize);

	out("* 4");
	size_t rec_size = sizeof(struct data_s);
	out("* 5");
	ASSERTeq(size, 1024);

	(void) size;
	(void) addr;
	(void) rec_size;

	out("* 6");

	while (0) {
		/* record_constructor(); */
		count++;
	}

	if (count) {
		out("create(%s): allocated %lu records (of size %zu)",
			path, count, sizeof(struct data_s));
	} else {
		out("create(%s): pool is full", path);
	}

	util_poolset_close(set, DO_NOT_DELETE_PARTS);
}


/*
 * do_verify -- (internal) verify a poolset
 */
static void
do_verify(const char *path)
{
	PMEMobjpool *pop = NULL;
	PMEMoid oid;
	uint64_t count = 0;
	int error = 0;
	const char *layout = NULL;

	record_constructor();

	if ((pop = pmemobj_open(path, layout)) == NULL) {
		out("!%s: pmemobj_open: %s",
			path, pmemobj_errormsg());
		exit(-1);
	}

	TOID(struct root_s) root = POBJ_ROOT(pop, struct root_s);
	TOID(struct data_s) rec;

	POBJ_FOREACH(pop, oid) {
		TOID_ASSIGN(rec, oid);
		if (!util_checksum(D_RW(rec), sizeof(*D_RW(rec)),
					&D_RW(rec)->checksum,
					0 /* verify */, SKIP_OFFSET)) {
			out("verify(%s): incorrect record: %s (#%lu)",
				path, D_RW(rec)->signature, count);
			error = 1;
			break;
		}

		count++;
	}

	if (D_RO(root)->count != count) {
		out(
			"verify(%s): incorrect number of records (is: %lu, should be: %lu)",
			path, count, D_RO(root)->count);
		error = 1;
	}

	pmemobj_close(pop);

	if (error) {
		out("verify(%s): pool file contains error", path);
		exit(-1);
	}

	out(
		"verify(%s): pool file successfully verified (%lu records of size %zu)",
		path, count, sizeof(struct data_s));
}

int
main(int argc, char *argv[])
{
	util_init();
	out_init("OBJ_VERIFY", "OBJ_VERIFY", "", 1, 0);

	if (argc < 4) {
		out("Usage: %s <obj_pool> <op:c|v>\n"
		    "Options:\n"
		    "   c - create\n"
		    "   v - verify\n",
		    argv[0]);
		exit(-1);
	}

	const char *path = argv[1];
	const char *op;

	/*
	 * This application can be very time-consuming
	 * when it is run on an non-pmem filesystem,
	 * so let's set PMEM_IS_PMEM_FORCE to 1 for this case.
	 */
	setenv("PMEM_IS_PMEM_FORCE", "1", 1 /* overwrite */);

	/* go through all arguments one by one */
	for (int arg = 2; arg < argc; arg++) {
		op = argv[arg];

		if (op[1] != '\0') {
			out("op must be c or v (c=create, v=verify)");
			exit(-1);
		}

		switch (op[0]) {
		case 'c': /* create and verify (no debug) */
			do_create(path);
			break;

		case 'v': /* verify (no debug) */
			do_verify(path);
			break;

		default:
			out("op must be c or v (c=create, v=verify)");
			exit(-1);
			break;
		}
	}

	out_fini();

	return 0;
}
