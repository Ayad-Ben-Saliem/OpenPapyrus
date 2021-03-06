/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * __bam_reclaim --
 *	Free a database.
 *
 * PUBLIC: int __bam_reclaim __P((DB *, DB_THREAD_INFO *, DB_TXN *, uint32));
 */
int __bam_reclaim(DB * dbp, DB_THREAD_INFO * ip, DB_TXN * txn, uint32 flags)
{
	DBC * dbc;
	DB_LOCK meta_lock;
	int ret, t_ret;
	/* Acquire a cursor. */
	if((ret = __db_cursor(dbp, ip, txn, &dbc, 0)) != 0)
		return ret;
	/* Write lock the metapage for deallocations. */
	if((ret = __db_lget(dbc, 0, PGNO_BASE_MD, DB_LOCK_WRITE, 0, &meta_lock)) != 0)
		goto err;
	/* Avoid locking every page, we have the handle locked exclusive. */
	F_SET(dbc, DBC_DONTLOCK);
	/* Walk the tree, freeing pages. */
	ret = __bam_traverse(dbc, DB_LOCK_WRITE, PGNO_INVALID, __db_reclaim_callback, (void *)(uintptr_t)flags);
	if((t_ret = __TLPUT(dbc, meta_lock)) != 0 && ret == 0)
		ret = t_ret;
	/* Discard the cursor. */
err:
	if((t_ret = __dbc_close(dbc)) != 0 && ret == 0)
		ret = t_ret;
	return ret;
}
/*
 * __bam_truncate --
 *	Truncate a database.
 *
 * PUBLIC: int __bam_truncate __P((DBC *, uint32 *));
 */
int __bam_truncate(DBC * dbc, uint32 * countp)
{
	uint32 count;
	int ret;
#ifdef HAVE_COMPRESSION
	uint32 comp_count = 0;
	if(DB_IS_COMPRESSED(dbc->dbp) && (ret = __bam_compress_count(dbc, NULL, &comp_count)) != 0)
		return ret;
#endif
	count = 0;
	/* Walk the tree, freeing pages. */
	ret = __bam_traverse(dbc, DB_LOCK_WRITE, PGNO_INVALID, __db_truncate_callback, &count);
#ifdef HAVE_COMPRESSION
	if(DB_IS_COMPRESSED(dbc->dbp)) {
		ASSIGN_PTR(countp, comp_count);
	}
	else
#endif
	ASSIGN_PTR(countp, count);
	return ret;
}
