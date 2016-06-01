/*
 * Copyright 2016, Intel Corporation
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
 * rpmem.c -- main source file for librpmem
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "librpmem.h"
#include "libpmem.h"
#include "rpmemd_db.h"

static struct rpmemd_db *Rpmemddb;

/*
 * struct rpmem_pool -- remote pool context
 */
struct rpmem_pool {
	void *local_pool_addr;
	void *remote_pool_addr;
	size_t pool_size;
	struct rpmemd_db_pool *db_pool;
};

/*
 * rpmem_create -- create remote pool on target node
 *
 * target        -- name of target node in format <target_name>[:<port>]
 * pool_set_name -- remote pool set name
 * pool_addr     -- local pool memory address which will be replicated
 * pool_size     -- required pool size
 * nlanes        -- number of lanes
 * pool_attr     -- pool attributes, received from remote host
 */
RPMEMpool *
rpmem_create(const char *target, const char *pool_set_name,
	void *pool_addr, size_t pool_size, unsigned *nlanes,
	const struct rpmem_pool_attr *create_attr)
{
	/* XXX */
	printf("***************** rpmem_create *****************\n");
	printf("target = %s\n", target);
	printf("pool_set_name = %s\n", pool_set_name);
	printf("pool_addr = %p\n", pool_addr);
	printf("pool_size = %zu\n", pool_size);
	printf("nlanes = %u\n", *nlanes);
	printf("create_attr = %p\n", create_attr);

	RPMEMpool *rpp = malloc(sizeof(*rpp));
	if (rpp == NULL) {
		printf("ERROR: out of memory\n");
		return NULL;
	}

	if (Rpmemddb == NULL) {
		Rpmemddb = rpmemd_db_init("/home/ldorau/work/nvml-github/"
						"src/test/obj_pool", 0644);
		if (Rpmemddb == NULL) {
			printf("ERROR: rpmemd_db_init()\n");
			return NULL;
		}
	}

	rpp->db_pool = rpmemd_db_pool_create(Rpmemddb, pool_set_name, pool_size,
								create_attr);
	if (rpp->db_pool == NULL) {
		printf("ERROR: rpmemd_db_pool_create() failed\n");
		return NULL;
	}

	rpp->pool_size = rpp->db_pool->pool_size;
	rpp->remote_pool_addr = rpp->db_pool->pool_addr;
	rpp->local_pool_addr = pool_addr;

	printf("OUT:\n");
	printf("rpp->local_pool_addr = %p\n", rpp->local_pool_addr);
	printf("rpp->remote_pool_addr = %p\n", rpp->remote_pool_addr);
	printf("***************** rpmem_create OK *****************\n");

	return rpp;
}

/*
 * rpmem_open -- open remote pool on target node
 *
 * target        -- name of target node in format <target_name>[:<port>]
 * pool_set_name -- remote pool set name
 * pool_addr     -- local pool memory address which will be replicated
 * pool_size     -- required pool size
 * nlanes        -- number of lanes
 * pool_attr     -- pool attributes, received from remote host
 */
RPMEMpool *
rpmem_open(const char *target, const char *pool_set_name,
	void *pool_addr, size_t pool_size, unsigned *nlanes,
	struct rpmem_pool_attr *open_attr)
{
	/* XXX */
	printf("***************** rpmem_open *****************\n");
	printf("target = %s\n", target);
	printf("pool_set_name = %s\n", pool_set_name);
	printf("pool_addr = %p\n", pool_addr);
	printf("pool_size = %zu\n", pool_size);
	printf("nlanes = %u\n", *nlanes);
	printf("open_attr = %p\n", open_attr);

	RPMEMpool *rpp = malloc(sizeof(*rpp));
	if (rpp == NULL) {
		printf("ERROR: out of memory\n");
		return NULL;
	}

	if (Rpmemddb == NULL) {
		Rpmemddb = rpmemd_db_init("/home/ldorau/work/nvml-github/"
						"src/test/obj_pool", 0644);
		if (Rpmemddb == NULL) {
			printf("ERROR: rpmemd_db_init()\n");
			return NULL;
		}
	}

	rpp->db_pool = rpmemd_db_pool_open(Rpmemddb, pool_set_name, pool_size,
								open_attr);
	if (rpp->db_pool == NULL) {
		printf("ERROR: rpmemd_db_pool_open() failed\n");
		return NULL;
	}

	rpp->pool_size = rpp->db_pool->pool_size;
	rpp->remote_pool_addr = rpp->db_pool->pool_addr;
	rpp->local_pool_addr = pool_addr;

	printf("OUT:\n");
	printf("rpp->local_pool_addr = %p\n", rpp->local_pool_addr);
	printf("rpp->remote_pool_addr = %p\n", rpp->remote_pool_addr);
	printf("***************** rpmem_open OK *****************\n");

	return rpp;
}

/*
 * rpmem_remove -- remove remote pool on target node
 *
 * target        -- name of target node in format <target_name>[:<port>]
 * pool_set_name -- remote pool set name
 */
int
rpmem_remove(const char *target, const char *pool_set_name)
{
	/* XXX */
	printf("***************** rpmem_remove *****************\n");
	return -1;
}

/*
 * rpmem_close -- close remote pool on target node
 */
int
rpmem_close(RPMEMpool *rpp)
{
	/* XXX */
	printf("***************** rpmem_close *****************\n");
	rpmemd_db_pool_close(Rpmemddb, rpp->db_pool);
	rpmemd_db_fini(Rpmemddb);
	free(rpp);
	return 0;
}

/*
 * rpmem_persist -- persist operation on target node
 *
 * rpp           -- remote pool handle
 * offset        -- offset in pool
 * length        -- length of persist operation
 * lane          -- lane number
 */
int
rpmem_persist(RPMEMpool *rpp, size_t offset, size_t length, unsigned lane)
{
	/* XXX */

	uintptr_t dst = (uintptr_t)rpp->remote_pool_addr + (uintptr_t)offset;
	uintptr_t src = (uintptr_t)rpp->local_pool_addr + (uintptr_t)offset;

	printf(">>> rpmem_persist - memcpy(%p -> %p (%zu))\n",
		(void *)src, (void *)dst, length);
	memcpy((void *)dst, (void *)src, length);

	printf(">>> rpmem_persist - pmem_msync()\n");
	pmem_msync((void *)dst, length);

	return 0;
}

/*
 * rpmem_read -- read data from remote pool:
 *
 * rpp           -- remote pool handle
 * buff          -- output buffer
 * offset        -- offset in pool
 * length        -- length of read operation
 */
int
rpmem_read(RPMEMpool *rpp, void *buff, size_t offset, size_t length)
{
	/* XXX */
	printf("***************** rpmem_read *****************\n");
	return -1;
}
