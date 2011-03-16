/*
 * Copyright (C) 2005-2010 Junjiro R. Okajima
 *
 * This program, aufs is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * file private data
 */

#include <linux/file.h>
#include "aufs.h"

void au_hfput(struct au_hfile *hf, struct file *file)
{
	if (file->f_mode & FMODE_EXEC)
		allow_write_access(hf->hf_file);
	fput(hf->hf_file);
	hf->hf_file = NULL;
	atomic_dec(&hf->hf_br->br_count);
	hf->hf_br = NULL;
}

void au_set_h_fptr(struct file *file, aufs_bindex_t bindex, struct file *val)
{
	struct au_finfo *finfo = au_fi(file);
	struct au_hfile *hf;
	struct au_fidir *fidir;

	fidir = finfo->fi_hdir;
	if (!fidir) {
		AuDebugOn(finfo->fi_btop != bindex);
		hf = &finfo->fi_htop;
	} else
		hf = fidir->fd_hfile + bindex;

	if (hf && hf->hf_file)
		au_hfput(hf, file);
	if (val) {
		FiMustWriteLock(file);
		hf->hf_file = val;
		hf->hf_br = au_sbr(file->f_dentry->d_sb, bindex);
	}
}

void au_update_figen(struct file *file)
{
	atomic_set(&au_fi(file)->fi_generation, au_digen(file->f_dentry));
	/* smp_mb(); */ /* atomic_set */
}

/* ---------------------------------------------------------------------- */

void au_fi_mmap_lock(struct file *file)
{
	FiMustWriteLock(file);
	lockdep_off();
	mutex_lock(&au_fi(file)->fi_mmap);
	lockdep_on();
}

void au_fi_mmap_unlock(struct file *file)
{
	lockdep_off();
	mutex_unlock(&au_fi(file)->fi_mmap);
	lockdep_on();
}

/* ---------------------------------------------------------------------- */

struct au_fidir *au_fidir_alloc(struct super_block *sb)
{
	struct au_fidir *fidir;
	int nbr;

	nbr = au_sbend(sb) + 1;
	if (nbr < 2)
		nbr = 2; /* initial allocate for 2 branches */
	fidir = kzalloc(au_fidir_sz(nbr), GFP_NOFS);
	if (fidir) {
		fidir->fd_bbot = -1;
		fidir->fd_nent = nbr;
		fidir->fd_vdir_cache = NULL;
	}

	return fidir;
}

int au_fidir_realloc(struct au_finfo *finfo, int nbr)
{
	int err;
	struct au_fidir *fidir, *p;

	AuRwMustWriteLock(&finfo->fi_rwsem);
	fidir = finfo->fi_hdir;
	AuDebugOn(!fidir);

	err = -ENOMEM;
	p = au_kzrealloc(fidir, au_fidir_sz(fidir->fd_nent), au_fidir_sz(nbr),
			 GFP_NOFS);
	if (p) {
		p->fd_nent = nbr;
		finfo->fi_hdir = p;
		err = 0;
	}

	return err;
}

/* ---------------------------------------------------------------------- */

void au_finfo_fin(struct file *file)
{
	struct au_finfo *finfo;

	finfo = au_fi(file);
	AuDebugOn(finfo->fi_hdir);
	AuRwDestroy(&finfo->fi_rwsem);
	au_cache_free_finfo(finfo);
}

void au_fi_init_once(void *_finfo)
{
	struct au_finfo *finfo = _finfo;
	static struct lock_class_key aufs_fi, aufs_fi_vm, aufs_fi_mmap;

	au_rw_init(&finfo->fi_rwsem);
	au_rw_class(&finfo->fi_rwsem, &aufs_fi);
	mutex_init(&finfo->fi_vm_mtx);
	lockdep_set_class(&finfo->fi_vm_mtx, &aufs_fi_vm);
	mutex_init(&finfo->fi_mmap);
	lockdep_set_class(&finfo->fi_mmap, &aufs_fi_mmap);
}

int au_finfo_init(struct file *file, struct au_fidir *fidir)
{
	int err;
	unsigned long ul;
	struct au_finfo *finfo;
	struct dentry *dentry;

	err = -ENOMEM;
	dentry = file->f_dentry;
	finfo = au_cache_alloc_finfo();
	if (unlikely(!finfo))
		goto out;

	err = 0;
	au_rw_write_lock(&finfo->fi_rwsem);
	finfo->fi_btop = -1;
	finfo->fi_hdir = fidir;
	atomic_set(&finfo->fi_generation, au_digen(dentry));
	/* smp_mb(); */ /* atomic_set */

	/* cf. au_store_oflag() */
	ul = (unsigned long)file->private_data;
	file->f_mode |= (ul & FMODE_EXEC);
	file->private_data = finfo;

out:
	return err;
}
