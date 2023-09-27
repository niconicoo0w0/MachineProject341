/**
 * partner: ruizhou3
 * mad_mad_access_patterns
 * CS 341 - Spring 2023
 */
#include "tree.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

void binary_search(FILE *fp, char *word, unsigned long struct_size) {
  fseek(fp, BINTREE_ROOT_NODE_OFFSET, SEEK_SET);
  BinaryTreeNode *curr = calloc(1, struct_size);
  fread(curr, struct_size, 1, fp);
  int flag = 1;
  while (flag) {
    char offset = 1;
    while (fgetc(fp)) {
      offset++;
    }
    fseek(fp, 0 - offset, 1);
    char *to_compare = calloc(1, offset);
    fread(to_compare, 1, offset, fp);
    if (!strcmp(word, to_compare)) {
      printFound(word, curr->count, curr->price);
      flag = 0;
    }
    uint32_t value;
    if (MAX(strcmp(word, to_compare), 0)) {
      value = curr->right_child;
    } else {
      value = curr->left_child;
    }
    if (!value) {
      printNotFound(word);
      flag = 0;
    } else {
      fseek(fp, value, SEEK_SET);
      fread(curr, struct_size, 1, fp);
    }
    if (to_compare) { free(to_compare); }
  }
  if (curr) { free(curr); }
}

int main(int argc, char **argv) {
    if (argc <= 2) {
      printArgumentUsage();
      exit(1);
    }
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
      openFail(argv[1]);
      exit(2);
    }
    char root[5];
    fread(root, 1, 4, fp);
    root[4] = '\0';
    // Any file which does not start with these 4 bytes is an invalid data file
    if (strcmp(root, BINTREE_HEADER_STRING)) {
      formatFail(argv[1]);
      exit(2);
    }
    for (int i = 2; i < argc; i++) {
      if (fp && argv[i]) {
        binary_search(fp, argv[i], sizeof(BinaryTreeNode));
      }
    }
    fclose(fp);
    return 0;
}