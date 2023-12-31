/*
 *  linux/fs/nfs/inode.c
 *
 *  Copyright (C) 1992  Rick Sladkey
 *
 *  nfs inode and superblock handling functions
 */

#include <asm/system.h>
#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/nfs_fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/locks.h>

extern int close_fp(struct file *filp);

static int nfs_notify_change(struct inode *);
static void nfs_put_super(struct super_block *);
static void nfs_statfs(struct super_block *, struct statfs *);

static struct super_operations nfs_sops = { 
	NULL,			/* read inode */
	nfs_notify_change,	/* notify change */
	NULL,			/* write inode */
	NULL,			/* put inode */
	nfs_put_super,		/* put superblock */
	NULL,			/* write superblock */
	nfs_statfs		/* stat filesystem */
};

void nfs_put_super(struct super_block *sb)
{
	close_fp(sb->u.nfs_sb.s_server.file);
	lock_super(sb);
	sb->s_dev = 0;
	unlock_super(sb);
}

/*
 * The way this works is that the mount process passes a structure
 * in the data argument which contains an open socket to the NFS
 * server and the root file handle obtained from the server's mount
 * daemon.  We stash theses away in the private superblock fields.
 * Later we can add other mount parameters like caching values.
 */

struct super_block *nfs_read_super(struct super_block *sb, void *raw_data)
{
	struct nfs_mount_data *data = (struct nfs_mount_data *) raw_data;
	struct nfs_server *server;
	unsigned int fd;
	struct file *filp;
	dev_t dev = sb->s_dev;

	if (!data) {
		printk("nfs_read_super: missing data argument\n");
		sb->s_dev = 0;
		return NULL;
	}
	fd = data->fd;
	if (data->version != NFS_MOUNT_VERSION) {
		printk("nfs warning: mount version %s than kernel\n",
			data->version < NFS_MOUNT_VERSION ? "older" : "newer");
	}
	if (fd >= NR_OPEN || !(filp = current->filp[fd])) {
		printk("nfs_read_super: invalid file descriptor\n");
		sb->s_dev = 0;
		return NULL;
	}
	if (!S_ISSOCK(filp->f_inode->i_mode)) {
		printk("nfs_read_super: not a socket\n");
		sb->s_dev = 0;
		return NULL;
	}
	filp->f_count++;
	lock_super(sb);
	sb->s_blocksize = 1024; /* XXX */
	sb->s_magic = NFS_SUPER_MAGIC;
	sb->s_dev = dev;
	sb->s_op = &nfs_sops;
	server = &sb->u.nfs_sb.s_server;
	server->file = filp;
	server->lock = 0;
	server->wait = NULL;
	server->flags = data->flags;
	server->rsize = data->rsize;
	if (server->rsize <= 0)
		server->rsize = NFS_DEF_FILE_IO_BUFFER_SIZE;
	else if (server->rsize >= NFS_MAX_FILE_IO_BUFFER_SIZE)
		server->rsize = NFS_MAX_FILE_IO_BUFFER_SIZE;
	server->wsize = data->wsize;
	if (server->wsize <= 0)
		server->wsize = NFS_DEF_FILE_IO_BUFFER_SIZE;
	else if (server->wsize >= NFS_MAX_FILE_IO_BUFFER_SIZE)
		server->wsize = NFS_MAX_FILE_IO_BUFFER_SIZE;
	server->timeo = data->timeo*HZ/10;
	server->retrans = data->retrans;
	server->acregmin = data->acregmin*HZ;
	server->acregmax = data->acregmax*HZ;
	server->acdirmin = data->acdirmin*HZ;
	server->acdirmax = data->acdirmax*HZ;
	strcpy(server->hostname, data->hostname);
	sb->u.nfs_sb.s_root = data->root;
	unlock_super(sb);
	if (!(sb->s_mounted = nfs_fhget(sb, &data->root, NULL))) {
		sb->s_dev = 0;
		printk("nfs_read_super: get root inode failed\n");
		return NULL;
	}
	return sb;
}

void nfs_statfs(struct super_block *sb, struct statfs *buf)
{
	int error;
	struct nfs_fsinfo res;

	put_fs_long(NFS_SUPER_MAGIC, &buf->f_type);
	error = nfs_proc_statfs(&sb->u.nfs_sb.s_server, &sb->u.nfs_sb.s_root,
		&res);
	if (error) {
		if (error != -EINTR)
			printk("nfs_statfs: statfs error = %d\n", -error);
		res.bsize = res.blocks = res.bfree = res.bavail = 0;
	}
	put_fs_long(res.bsize, &buf->f_bsize);
	put_fs_long(res.blocks, &buf->f_blocks);
	put_fs_long(res.bfree, &buf->f_bfree);
	put_fs_long(res.bavail, &buf->f_bavail);
#if 0
	put_fs_long(-1, &buf->f_files);
	put_fs_long(-1, &buf->f_ffree);
#else
	put_fs_long(0, &buf->f_files);
	put_fs_long(0, &buf->f_ffree);
#endif
}

/*
 * This is our own version of iget that looks up inodes by file handle
 * instead of inode number.  We use this technique instead of using
 * the vfs read_inode function because there is no way to pass the
 * file handle or current attributes into the read_inode function.
 * We just have to be careful not to subvert iget's special handling
 * of mount points.
 */

struct inode *nfs_fhget(struct super_block *sb, struct nfs_fh *fhandle,
			struct nfs_fattr *fattr)
{
	struct nfs_fattr newfattr;
	int error;
	struct inode *inode;

	if (!sb) {
		printk("nfs_fhget: super block is NULL\n");
		return NULL;
	}
	if (!fattr) {
		error = nfs_proc_getattr(&sb->u.nfs_sb.s_server, fhandle,
			&newfattr);
		if (error) {
			printk("nfs_fhget: getattr error = %d\n", -error);
			return NULL;
		}
		fattr = &newfattr;
	}
	if (!(inode = iget(sb, fattr->fileid))) {
		printk("nfs_fhget: iget failed\n");
		return NULL;
	}
	if (inode->i_dev == sb->s_dev) {
		if (inode->i_ino != fattr->fileid) {
			printk("nfs_fhget: unexpected inode from iget\n");
			return inode;
		}
		*NFS_FH(inode) = *fhandle;
		nfs_refresh_inode(inode, fattr);
	}
	return inode;
}

int nfs_notify_change(struct inode *inode)
{
	struct nfs_sattr sattr;
	struct nfs_fattr fattr;
	int error;

	sattr.mode = inode->i_mode;
	sattr.uid = inode->i_uid;
	sattr.gid = inode->i_gid;
	sattr.size = S_ISREG(inode->i_mode) ? inode->i_size : -1;
	sattr.mtime.seconds = inode->i_mtime;
	sattr.mtime.useconds = 0;
	sattr.atime.seconds = inode->i_atime;
	sattr.atime.useconds = 0;
	error = nfs_proc_setattr(NFS_SERVER(inode), NFS_FH(inode),
		&sattr, &fattr);
	if (!error)
		nfs_refresh_inode(inode, &fattr);
	inode->i_dirt = 0;
	return error;
}

