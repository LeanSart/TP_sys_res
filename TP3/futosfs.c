/*
  FUSE: Filesystem in Userspace
  gcc -Wall futosfs.c `pkg-config fuse --cflags --libs` -o futosfs
*/

#define FUSE_USE_VERSION 26


#include <fuse_lowlevel.h>
#include "tosfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

static char *fs = NULL;
static struct tosfs_inode *inode_table = NULL;

/* Renvoie l'inode (pointeur dans le mmap) ou NULL si numéro invalide */
static const struct tosfs_inode *get_inode(fuse_ino_t ino)
{
    if (ino < 1 || ino > 32)
        return NULL;
    return &inode_table[ino];
}

/* Renvoie le bloc de données de l'inode ou NULL si hors de l'image */
static const char *get_data_block(const struct tosfs_inode *inode)
{
    if ((size_t)(inode->block_no + 1) * TOSFS_BLOCK_SIZE > 40960)
        return NULL;
    return fs + (size_t)inode->block_no * TOSFS_BLOCK_SIZE;
}

static int futosfs_stat(fuse_ino_t ino, struct stat *stbuf)
{
    const struct tosfs_inode *inode = get_inode(ino);
    if (inode == NULL)
        return -1;
    memset(stbuf, 0, sizeof(struct stat));
    stbuf->st_ino = inode->inode;
    stbuf->st_mode = inode->mode | inode->perm; // Fusion du mode (fichier/dossier) et des permissions
    stbuf->st_nlink = inode->nlink;
    stbuf->st_uid = inode->uid;
    stbuf->st_gid = inode->gid;
    stbuf->st_size = inode->size;
    stbuf->st_blocks = (inode->size + 512 - 1) / 512; // Nombre de blocs de 512 octets

    return 0;
}

static void futosfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
	struct stat stbuf;

	(void) fi;

	memset(&stbuf, 0, sizeof(stbuf));
	if (futosfs_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}

static void futosfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    const struct tosfs_inode *dir = get_inode(parent);
    if (dir == NULL) {
        fuse_reply_err(req, ENOENT);
        return;
    }
    if (!S_ISDIR(dir->mode)) {
        fuse_reply_err(req, ENOTDIR);
        return;
    }

    const char *block = get_data_block(dir);
    if (block == NULL) {
        fuse_reply_err(req, EIO);
        return;
    }

    const struct tosfs_dentry *dentries = (const struct tosfs_dentry *)block;
    int num_dentries = TOSFS_BLOCK_SIZE / sizeof(struct tosfs_dentry);
    fuse_ino_t target = 0;

    for (int i = 0; i < num_dentries; i++) {
        /* name n'est pas forcément terminé par '\0' s'il fait 32 caractères */
        if (dentries[i].inode != 0 &&
            strncmp(dentries[i].name, name, TOSFS_MAX_NAME_LENGTH) == 0 &&
            strlen(name) <= TOSFS_MAX_NAME_LENGTH) {
            target = dentries[i].inode;
            break;
        }
    }

    if (target == 0) {
        fuse_reply_err(req, ENOENT);
        return;
    }

    struct fuse_entry_param e;
    memset(&e, 0, sizeof(e));
    e.ino = target;
    e.attr_timeout = 1.0;
    e.entry_timeout = 1.0;
    if (futosfs_stat(target, &e.attr) == -1) {
        fuse_reply_err(req, ENOENT);
        return;
    }
    fuse_reply_entry(req, &e);
}

struct dirbuf {
	char *p;
	size_t size;
};

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
		       fuse_ino_t ino)
{
	struct stat stbuf;
	size_t oldsize = b->size;
	b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	b->p = (char *) realloc(b->p, b->size);
	memset(&stbuf, 0, sizeof(stbuf));
	stbuf.st_ino = ino;
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
			  b->size);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize)
{
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}

static void futosfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi)
{
	(void) fi;

    const struct tosfs_inode *inode = get_inode(ino);
    if (inode == NULL) {
        fuse_reply_err(req, ENOENT);
        return;
    }
    if (!S_ISDIR(inode->mode)) {
        fuse_reply_err(req, ENOTDIR);
        return;
    }

    const char *block = get_data_block(inode);
    if (block == NULL) {
        fuse_reply_err(req, EIO);
        return;
    }

    const struct tosfs_dentry *dentries = (const struct tosfs_dentry *)block;
    int num_dentries = TOSFS_BLOCK_SIZE / sizeof(struct tosfs_dentry);

    struct dirbuf b;
    memset(&b, 0, sizeof(b));

    for (int i = 0; i < num_dentries; i++) {
        if (dentries[i].inode == 0)
            continue;
        char name[TOSFS_MAX_NAME_LENGTH + 1];
        memcpy(name, dentries[i].name, TOSFS_MAX_NAME_LENGTH);
        name[TOSFS_MAX_NAME_LENGTH] = '\0';
        dirbuf_add(req, &b, name, dentries[i].inode);
    }

    reply_buf_limited(req, b.p, b.size, off, size);
    free(b.p);
}

static void futosfs_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi)
{
	(void) fi;

    const struct tosfs_inode *inode = get_inode(ino);
    if (inode == NULL) {
        fuse_reply_err(req, ENOENT);
        return;
    }
    if (S_ISDIR(inode->mode)) {
        fuse_reply_err(req, EISDIR);
        return;
    }

    const char *data = get_data_block(inode);
    if (data == NULL) {
        fuse_reply_err(req, EIO);
        return;
    }

    /* reply_buf_limited renvoie un buffer vide si off >= taille du fichier */
    reply_buf_limited(req, data, inode->size, off, size);
}

static struct fuse_lowlevel_ops futosfs_oper = {
    .getattr	= futosfs_getattr,
    .readdir	= futosfs_readdir,
	.lookup		= futosfs_lookup,
	.read		= futosfs_read,
};

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_chan *ch;
	char *mountpoint;
	int err = -1;
    
    int fd = open("test_tosfs_files", O_RDWR);
    if (fd < 0) {
        perror("Erreur ouverture test_tosfs_files");
        return 1;
    }
    fs = mmap(NULL, 40960, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //taille de fd : 40960 octets (trouvé en utilisant stat) 
    inode_table = (struct tosfs_inode *)(fs + TOSFS_INODE_BLOCK * TOSFS_BLOCK_SIZE);
    close(fd); //fichier stocké dans le mmap, plus besoin de garder le fichier ouvert
	if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
	    (ch = fuse_mount(mountpoint, &args)) != NULL) {
		struct fuse_session *se;

		se = fuse_lowlevel_new(&args, &futosfs_oper,
				       sizeof(futosfs_oper), NULL);
		if (se != NULL) {
			if (fuse_set_signal_handlers(se) != -1) {
				fuse_session_add_chan(se, ch);
				err = fuse_session_loop(se);
				fuse_remove_signal_handlers(se);
				fuse_session_remove_chan(ch);
			}
			fuse_session_destroy(se);
		}
		fuse_unmount(mountpoint, ch);
	}
	fuse_opt_free_args(&args);
    msync(fs, 40960, MS_SYNC);
    munmap(fs, 40960);
	return err ? 1 : 0;
}
