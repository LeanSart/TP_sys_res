#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "tosfs.h"

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Erreur open");
        return 1;
    }
    void *map = mmap(NULL, 40960, PROT_READ, MAP_SHARED, fd, 0); //taille de fd : 40960 octets (trouvé en utilisant stat) 
    close(fd); //fichier stocké dans le mmap, plus besoin de garder le fichier ouvert

    struct tosfs_superblock *sb = (struct tosfs_superblock *) map;
    printf("Superblock :\n");
    printf("Magic number : 0x%X ", sb->magic);
    if (sb->magic == TOSFS_MAGIC) {
        printf("(Valide)\n");
    } else {
        printf("(INVALIDE !)\n");
    }
    printf("Bloc bitmap : " PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(sb->block_bitmap));
    printf("Inode bitmap : " PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(sb->inode_bitmap));
    printf("Taille d'un bloc : %u octets\n", sb->block_size);
    printf("Nombre de blocks : %u\n", sb->blocks);
    printf("Nombre d'inodes : %u\n", sb->inodes);
    printf("Numero de l'inode root : %u\n", sb->root_inode);

    struct tosfs_inode *inodes = (struct tosfs_inode *) ((char *)map + TOSFS_INODE_BLOCK * sb->block_size);
    printf("Inodes :\n");
    for (unsigned int i = 0; i < sb->inodes; i++) {
        if (sb->inode_bitmap & (1 << i)) { //On verifie que l'inode est utilisé
            struct tosfs_inode *ino = &inodes[i];
            printf("Inode %2u / Block: %2u / UID: %u / GID: %u / Mode: 0%o / Size: %u bytes\n",
                   ino->inode, ino->block_no, ino->uid, ino->gid, ino->mode, ino->size);
        }
    }

    struct tosfs_dentry *dentry = (struct tosfs_dentry *) ((char *)map + TOSFS_ROOT_BLOCK * sb->block_size);
    printf("Dentry :\n");
    unsigned int max_dentry = sb->block_size / sizeof(struct tosfs_dentry);
    for (unsigned int i = 0; i < max_dentry; i++) {
        if (dentry[i].inode != 0) { // Si l'inode associé est valide
            printf("Inode: %u -> Fichier : %s\n", dentry[i].inode, dentry[i].name);
        }
    }

    munmap(map, 40960);
    return 0;
}