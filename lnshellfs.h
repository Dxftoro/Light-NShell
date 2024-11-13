/*#include "linux/fs.h"
#define PAGE_CACHE_SIZE 4096

static struct inode* lnfs_make_inode(struct super_block* sb, int mode) {
	struct inode *ret = new_inode(sb);

	if (ret) {
		ret->i_mode = mode;
		ret->i_uid->i_gid = 0;
		ret->i_blksize = PAGE_CACHE_SIZE;
		ret->blocks = 0;
	}
	return ret;
}*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>

#define ROOT_INO 1

void tutorfs_read_inode(struct inode *inode) {
        ino_t ino = inode->i_ino;

	/* Defaults */
	inode->i_mode = S_IRUGO | S_IWUSR;
        inode->i_nlink = 1;
       	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
       	inode->i_blocks = 0;
       	inode->i_blksize = inode->i_sb->s_blocksize;

	if(ino == ROOT_INO) {
		/* Special case for the root directory */
		inode->i_mode |= S_IFDIR | S_IXUGO;
        	inode->i_nlink = 2;
                inode->i_fop = &simple_dir_operations;
                inode->i_op = &simple_dir_inode_operations;
	}
}

static void tutorfs_put_super(struct super_block *sb) {
        return;
}

static struct super_operations tutorfs_ops = {
        .read_inode     = tutorfs_read_inode,
        .put_super      = tutorfs_put_super,
};

int tutorfs_fill_sb(struct super_block *sb, void *data, int silent) {
        struct inode *root;

        sb->s_op = &tutorfs_ops;

        sb->s_blocksize = 4096;
        sb->s_blocksize_bits = 12;

        root = iget(sb, ROOT_INO);
        sb->s_root = d_alloc_root(root);
        if(!sb->s_root) return -EINVAL;

        /* Indicate success */
        return 0;
}

struct super_block *tutorfs_get_sb(struct file_system_type *fs_type,
        int flags, const char *dev_name, void *data) {

        return get_sb_nodev(fs_type, flags, data, tutorfs_fill_sb);
}

static struct file_system_type tutorfs_type = {
        .name           = "tutorfs",
        .get_sb         = tutorfs_get_sb,
        .kill_sb        = kill_anon_super
};
