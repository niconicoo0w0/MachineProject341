/**
 * finding_filesystems
 * CS 341 - Spring 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

static unsigned long block_size = sizeof(data_block);
static unsigned long block_num_size = sizeof(data_block_number);

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);


// Helper Functions!!
size_t get_index_offset(size_t size, int flag) {
    if (flag == 0) {
        return size / block_size;
    }
    return size % block_size;
}

void update_ctim(inode* curr) {
    clock_gettime(CLOCK_REALTIME, &(curr->ctim));
    return;
}

void update_atim(inode* curr) {
    clock_gettime(CLOCK_REALTIME, &(curr->atim));
    return;
}

void update_mtim(inode* curr) {
    clock_gettime(CLOCK_REALTIME, &(curr->mtim));
    return;
}

data_block* help_datablock(file_system* fs, inode* curr, uint64_t index){
    data_block_number* block;
    if (index >= 11) {
        block = (data_block_number*)(fs->data_root + curr->indirect);
        index -= 11;
        return fs->data_root + block[index];
    }
    block = curr->direct;
    return fs->data_root + block[index];
}

size_t get_num_unused_block(file_system *fs) {
    char *map = GET_DATA_MAP(fs->meta);
    size_t i = 0;
    size_t returnit = 0;
    while (i < fs->meta->dblock_count) {
        if (!map[i]) {
            returnit++;
        }
        i++;
    }
    return returnit;
}

// Actual Functions!!
int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    inode* curr = get_inode(fs, path);
    // check existence
    if (!curr) {
        errno = ENOENT;
        return -1;
    }
    curr->mode = ((curr->mode >> RWX_BITS_NUMBER) << RWX_BITS_NUMBER) | new_permissions;
    update_ctim(curr);
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* curr = get_inode(fs, path);
    // check existence
    if (!curr) {
        errno = ENOENT;
        return -1;
    }
    if (owner == ((uid_t)-1)) {
        curr->uid = curr->uid;
    } else {
        curr->uid = owner;
    }
    if (group == ((gid_t)-1)) {
        curr->gid = curr->gid;
    } else {
        curr->gid = group;
    }
    update_ctim(curr);
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode* node = get_inode(fs, path);
    if (!node) {
        const char* filename;
        inode* parent = parent_directory(fs, path, &filename);
        if (!valid_filename(filename)) {
            return NULL;
        }
        if (!parent || !is_directory(parent) || parent->size >= NUM_DIRECT_BLOCKS * block_size) {
            return NULL;
        }

        size_t index = get_index_offset(parent->size, 0);
        size_t offset = get_index_offset(parent->size, 1);
        inode_number unused = first_unused_inode(fs);
        // there are no more inodes in the system
        if (unused == -1) {
            return NULL;
        }

        if (index >= NUM_DIRECT_BLOCKS && parent->indirect == -1) {
            // number of data blocks is full
            if (add_single_indirect_block(fs, parent) == -1) {
                return NULL;
            }
        }

        inode* returnit = fs->inode_root + unused;
        init_inode(parent, returnit);

        minixfs_dirent dirent;
        dirent.inode_num = unused;
        dirent.name = strdup(filename);

        char* s = (char*)help_datablock(fs, parent, index) + offset;
        memset(s, 0, FILE_NAME_ENTRY);
        make_string_from_dirent(s, dirent);
        parent->size += 256;
        return returnit;
    }
    return NULL;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        size_t unused_block = get_num_unused_block(fs);
        // used block
        char* content = block_info_string(fs->meta->dblock_count - unused_block);
        long len = (long)strlen(content);
        if (*off >= len) {
            return 0;
        }
        size_t remain = len - *off;
        size_t size = MIN(remain, count);
        memcpy(buf, content + *off, size);
        *off += size;
        return size;
    }
    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    size_t max_size = (NUM_DIRECT_BLOCKS + block_size / block_num_size) * block_size;
    if (count + *off > max_size) {
        errno = ENOSPC;
        return -1;
    }
    inode* curr = get_inode(fs, path);
    if (!curr) {
        curr = minixfs_create_inode_for_path(fs, path);
        if (!curr) {
            errno = ENOSPC;
            return -1;
        }
    }

    size_t required_block = get_index_offset((*off + count), 0);
    size_t remain = get_index_offset((*off + count), 1);
    if (remain != 0) {
        required_block++;
    }
    if (minixfs_min_blockcount(fs, path, required_block) == -1) {
        errno = ENOSPC;
        return -1;
    }

    size_t index = get_index_offset(*off, 0);
    size_t offset = get_index_offset(*off, 1);
    size_t returnit = 0;

    for (size_t bytes_done = 0; bytes_done < count; index++) {
        data_block* block = help_datablock(fs, curr, index) + offset;
        size_t size = MIN(count - bytes_done, block_size - offset);
        memcpy(block->data + offset, buf + bytes_done, size);
        bytes_done += size;
        *off += size;
        // Move to next block and update return value
        offset = 0;
        returnit = bytes_done;
    }
    curr->size += returnit;
    update_atim(curr);
    update_mtim(curr);
    return returnit;
}

ssize_t minixfs_read(file_system* fs, const char* path, void* buf, size_t count, off_t* off) {
    const char* virtual_path = is_virtual_path(path);
    if (virtual_path) {
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    }
    inode* curr = get_inode(fs, path);
    if (!curr) {
        errno = ENOENT;
        return -1;
    }
    if (curr->size - *off == 0) {
        return 0;
    }
    size_t total_bytes = MIN(curr->size - *off, count);

    size_t index = get_index_offset(*off, 0);
    size_t offset = get_index_offset(*off, 1);
    size_t returnit = 0;

    for (size_t bytes_done = 0; bytes_done < total_bytes; index++) {
        data_block* block = help_datablock(fs, curr, index) + offset;
        size_t size = MIN(total_bytes - bytes_done, block_size - offset);
        memcpy(buf + bytes_done, block, size);
        bytes_done += size;
        *off += size;
        // Move to next block and update return value
        offset = 0;
        returnit = bytes_done;
    }
    update_atim(curr);
    return returnit;
}

