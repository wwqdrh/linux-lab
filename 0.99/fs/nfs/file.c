/*
 *  linux/fs/nfs/file.c
 *
 *  Copyright (C) 1992  Rick Sladkey
 *
 *  nfs regular file handling functions
 */

#include <asm/segment.h>
#include <asm/system.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/mm.h>
#include <linux/nfs_fs.h>

static int nfs_file_read(struct inode *, struct file *, char *, int);
static int nfs_file_write(struct inode *, struct file *, char *, int);

static struct file_operations nfs_file_operations = {
	NULL,			/* lseek - default */
	nfs_file_read,		/* read */
	nfs_file_write,		/* write */
	NULL,			/* readdir - bad */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open is needed */
	NULL			/* release */
};

struct inode_operations nfs_file_inode_operations = {
	&nfs_file_operations,	/* default file operations */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	NULL,			/* bmap */
	NULL			/* truncate */
};

static int nfs_file_read(struct inode *inode, struct file *file, char *buf,
			 int count)
{
	int result;
	int hunk;
	int i;
	int n;
	struct nfs_fattr fattr;
	char *data;

	if (!inode) {
		printk("nfs_file_read: inode = NULL\n");
		return -EINVAL;
	}
	if (!S_ISREG(inode->i_mode)) {
		printk("nfs_file_read: read from non-file, mode %07o\n",
			inode->i_mode);
		return -EINVAL;
	}
	if (file->f_pos + count > inode->i_size)
		count = inode->i_size - file->f_pos;
	if (count <= 0)
		return 0;
	n = NFS_SERVER(inode)->rsize;
	data = (char *) kmalloc(n, GFP_KERNEL);
	for (i = 0; i < count; i += n) {
		hunk = count - i;
		if (hunk > n)
			hunk = n;
		result = nfs_proc_read(NFS_SERVER(inode), NFS_FH(inode), 
			file->f_pos, hunk, data, &fattr);
		if (result < 0) {
			kfree_s(data, n);
			return result;
		}
		memcpy_tofs(buf, data, result);
		file->f_pos += result;
		buf += result;
		if (result < n) {
			i += result;
			break;
		}
	}
	kfree_s(data, n);
	nfs_refresh_inode(inode, &fattr);
	return i;
}

static int nfs_file_write(struct inode *inode, struct file *file, char *buf,
			  int count)
{
	int result;
	int hunk;
	int i;
	int n;
	struct nfs_fattr fattr;
	char *data;

	if (!inode) {
		printk("nfs_file_write: inode = NULL\n");
		return -EINVAL;
	}
	if (!S_ISREG(inode->i_mode)) {
		printk("nfs_file_write: write to non-file, mode %07o\n",
			inode->i_mode);
		return -EINVAL;
	}
	if (count <= 0)
		return 0;
	if (file->f_flags & O_APPEND)
		file->f_pos = inode->i_size;
	n = NFS_SERVER(inode)->wsize;
	data = (char *) kmalloc(n, GFP_KERNEL);
	for (i = 0; i < count; i += n) {
		hunk = count - i;
		if (hunk >= n)
			hunk = n;
		memcpy_fromfs(data, buf, hunk);
		result = nfs_proc_write(NFS_SERVER(inode), NFS_FH(inode), 
			file->f_pos, hunk, data, &fattr);
		if (result < 0) {
			kfree_s(data, n);
			return result;
		}
		file->f_pos += hunk;
		buf += hunk;
		if (hunk < n) {
			i += hunk;
			break;
		}
	}
	kfree_s(data, n);
	nfs_refresh_inode(inode, &fattr);
	return i;
}

