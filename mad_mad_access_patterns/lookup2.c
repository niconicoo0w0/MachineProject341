/**
 * partner: ruizhou3
 * mad_mad_access_patterns
 * CS 341 - Spring 2023
 */
#include "tree.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h>

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

BinaryTreeNode *binary_search(uint32_t offset, char *word, char *addr) {
  if (offset) {
    BinaryTreeNode* curr = (BinaryTreeNode*)(addr + offset);
    if (!strcmp(word, curr->word)) {
      return curr;
    }
    // Recursively search the left or right subtree depending on the word.
    if (MIN(strcmp(word, curr->word), 0)) {
      return binary_search(curr->left_child, word, addr);
    } else {
      return binary_search(curr->right_child, word, addr);
    }
  }
  return NULL;
}

int main(int argc, char **argv) {
  if (argc <= 2) {
    printArgumentUsage();
    exit(1);
  }

  int fp = open(argv[1], 00);
  struct stat status;
  if (!fp || fstat(fp, &status)) {
    openFail(argv[1]);
    exit(2);
  }
  //mmap
  char* addr = mmap(0, status.st_size, 0x1, 0x02, fp, 0);
  if (addr == MAP_FAILED) {
    mmapFail(argv[1]);
    close(fp);
    exit(2);
  }

  char root[5];
  memcpy(root, addr, 4);
  root[4] = '\0';
  if (strcmp(root, BINTREE_HEADER_STRING)) {
    formatFail(argv[1]);
    close(fp);
    exit(2);
  }

  for (int i = 2; i < argc; i++) {
    if (argv[i]) {
      BinaryTreeNode *result = binary_search(4, argv[i], addr);
      if (!result) {
        printNotFound(argv[i]);
      } else {
        printFound(result->word, result->count, result->price);
      }
    } else {
      close(fp);
      exit(1);
    }
  }
  
  close(fp);
  return 0;
  
}