#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) {
        return -1;
    }

    int inodes_per_sector = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int sector_offset = (inumber - 1) / inodes_per_sector;
    int sector_index = (inumber - 1) % inodes_per_sector;

    int sector_num = INODE_START_SECTOR + sector_offset;
    struct inode inodes[DISKIMG_SECTOR_SIZE / sizeof(struct inode)];
    int bytes_read = diskimg_readsector(fs->dfd, sector_num, inodes);

    if (bytes_read != DISKIMG_SECTOR_SIZE) {
        return -1;
    }
    *inp = inodes[sector_index];

    return 0;
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {

    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum < 8) {
            return inp->i_addr[blockNum];
        } else {
            return -1; // fuera de rango
        }
    }

    if (blockNum < 7 * 256) {
        // Indirecto simple
        int indir_block = inp->i_addr[blockNum / 256];
        if (indir_block == 0) return -1;

        uint16_t buf[256];
        if (diskimg_readsector(fs->dfd, indir_block, buf) != DISKIMG_SECTOR_SIZE) {
            return -1;
        }
        return buf[blockNum % 256];
    }

    // Doble indirecto
    blockNum -= 7 * 256;
    if (blockNum >= 256 * 256) return -1;

    int doubly_indirect_block = inp->i_addr[7];
    if (doubly_indirect_block == 0) return -1;

    uint16_t level1[256];
    if (diskimg_readsector(fs->dfd, doubly_indirect_block, level1) != DISKIMG_SECTOR_SIZE) {
        return -1;
    }

    int level1_index = blockNum / 256;
    int level2_index = blockNum % 256;

    int level2_block = level1[level1_index];
    if (level2_block == 0) return -1;

    uint16_t level2[256];
    if (diskimg_readsector(fs->dfd, level2_block, level2) != DISKIMG_SECTOR_SIZE) {
        return -1;
    }

    return level2[level2_index];
}


int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1);
}