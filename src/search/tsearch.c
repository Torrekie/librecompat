/* Copyright (C) 1995-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Tree search for red/black trees.
   The algorithm for adding nodes is taken from one of the many "Algorithms"
   books by Robert Sedgewick, although the implementation differs.
   The algorithm for deleting nodes can probably be found in a book named
   "Introduction to Algorithms" by Cormen/Leiserson/Rivest.  At least that's
   the book that my professor took most algorithms from during the "Data
   Structures" course...

   Totally public domain.  */

/* Red/black trees are binary trees in which the edges are colored either red
   or black.  They have the following properties:
   1. The number of black edges on every path from the root to a leaf is
      constant.
   2. No two red edges are adjacent.
   Therefore there is an upper bound on the length of every path, it's
   O(log n) where n is the number of nodes in the tree.  No path can be longer
   than 1+2*P where P is the length of the shortest path in the tree.
   Useful for the implementation:
   3. If one of the children of a node is NULL, then the other one is red
      (if it exists).

   In the implementation, not the edges are colored, but the nodes.  The color
   interpreted as the color of the edge leading to this node.  The color is
   meaningless for the root node, but we color the root node black for
   convenience.  All added nodes are red initially.

   Adding to a red/black tree is rather easy.  The right place is searched
   with a usual binary tree search.  Additionally, whenever a node N is
   reached that has two red successors, the successors are colored black and
   the node itself colored red.  This moves red edges up the tree where they
   pose less of a problem once we get to really insert the new node.  Changing
   N's color to red may violate rule 2, however, so rotations may become
   necessary to restore the invariants.  Adding a new red leaf may violate
   the same rule, so afterwards an additional check is run and the tree
   possibly rotated.

   Deleting is hairy.  There are mainly two nodes involved: the node to be
   deleted (n1), and another node that is to be unchained from the tree (n2).
   If n1 has a successor (the node with a smallest key that is larger than
   n1), then the successor becomes n2 and its contents are copied into n1,
   otherwise n1 becomes n2.
   Unchaining a node may violate rule 1: if n2 is black, one subtree is
   missing one black edge afterwards.  The algorithm must try to move this
   error upwards towards the root, so that the subtree that does not have
   enough black edges becomes the whole tree.  Once that happens, the error
   has disappeared.  It may not be necessary to go all the way up, since it
   is possible that rotations and recoloring can fix the error before that.

   Although the deletion algorithm must walk upwards through the tree, we
   do not store parent pointers in the nodes.  Instead, delete allocates a
   small array of parent pointers and fills it while descending the tree.
   Since we know that the length of a path is O(log n), where n is the number
   of nodes, this is likely to use less memory.  */

/* Tree rotations look like this:
      A                C
     / \              / \
    B   C            A   G
   / \ / \  -->     / \
   D E F G         B   F
                  / \
                 D   E

   In this case, A has been rotated left.  This preserves the ordering of the
   binary tree.  */

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>

/* Assume malloc returns naturally aligned (alignof (max_align_t))
   pointers so we can use the low bits to store some extra info.  This
   works for the left/right node pointers since they are not user
   visible and always allocated by malloc.  The user provides the key
   pointer and so that can point anywhere and doesn't have to be
   aligned.  */
#define USE_MALLOC_LOW_BIT 1

typedef void (*__free_fn_t) (void *__nodep);
typedef void (*__action_fn_t) (const void *__nodep, VISIT __value, int __level);

#ifndef USE_MALLOC_LOW_BIT
typedef struct node_t
{
  /* Callers expect this to be the first element in the structure - do not
     move!  */
  const void *key;
  struct node_t *left_node;
  struct node_t *right_node;
  unsigned int is_red:1;
} *node;

#define LEFT(N) (N)->left_node
#define RIGHT(N) (N)->right_node

#else /* USE_MALLOC_LOW_BIT */

typedef struct node_t
{
  /* Callers expect this to be the first element in the structure - do not
     move!  */
  const void *key;
  uintptr_t left_node; /* Includes whether the node is red in low-bit. */
  uintptr_t right_node;
} *node;

#define LEFT(N) (node)((N)->left_node & ~((uintptr_t) 0x1))
#define RIGHT(N) (node)((N)->right_node)

#endif /* USE_MALLOC_LOW_BIT */
typedef const struct node_t *const_node;

#undef DEBUGGING

#ifdef DEBUGGING

/* Routines to check tree invariants.  */

#define CHECK_TREE(a) check_tree(a)

static void
check_tree_recurse (node p, int d_sofar, int d_total)
{
  if (p == NULL)
    {
      assert (d_sofar == d_total);
      return;
    }

  check_tree_recurse (LEFT(p), d_sofar + (LEFT(p) && !RED(LEFT(p))),
		      d_total);
  check_tree_recurse (RIGHT(p), d_sofar + (RIGHT(p) && !RED(RIGHT(p))),
		      d_total);
  if (LEFT(p))
    assert (!(RED(LEFT(p)) && RED(p)));
  if (RIGHT(p))
    assert (!(RED(RIGHT(p)) && RED(p)));
}

static void
check_tree (node root)
{
  int cnt = 0;
  node p;
  if (root == NULL)
    return;
  SETBLACK(root);
  for(p = LEFT(root); p; p = LEFT(p))
    cnt += !RED(p);
  check_tree_recurse (root, 0, cnt);
}

#else

#define CHECK_TREE(a)

#endif

/* twalk_r is the same as twalk, but with a closure parameter instead
   of the level.  */
static void
trecurse_r (const void *vroot, void (*action) (const void *, VISIT, void *),
	    void *closure)
{
  const_node root = (const_node) vroot;

  if (LEFT(root) == NULL && RIGHT(root) == NULL)
    (*action) (root, leaf, closure);
  else
    {
      (*action) (root, preorder, closure);
      if (LEFT(root) != NULL)
	trecurse_r (LEFT(root), action, closure);
      (*action) (root, postorder, closure);
      if (RIGHT(root) != NULL)
	trecurse_r (RIGHT(root), action, closure);
      (*action) (root, endorder, closure);
    }
}

void
__twalk_r (const void *vroot, void (*action) (const void *, VISIT, void *),
	   void *closure)
{
  const_node root = (const_node) vroot;

  CHECK_TREE ((node) root);

  if (root != NULL && action != NULL)
    trecurse_r (root, action, closure);
}
void
twalk_r (const void *vroot, void (*action) (const void *, VISIT, void *),
	 void *closure)
{
  __twalk_r (vroot, action, closure);
}

/* The standardized functions miss an important functionality: the
   tree cannot be removed easily.  We provide a function to do this.  */
static void
tdestroy_recurse (node root, __free_fn_t freefct)
{
  if (LEFT(root) != NULL)
    tdestroy_recurse (LEFT(root), freefct);
  if (RIGHT(root) != NULL)
    tdestroy_recurse (RIGHT(root), freefct);
  (*freefct) ((void *) root->key);
  /* Free the node itself.  */
  free (root);
}

void
__tdestroy (void *vroot, __free_fn_t freefct)
{
  node root = (node) vroot;

  CHECK_TREE (root);

  if (root != NULL)
    tdestroy_recurse (root, freefct);
}
void
tdestroy (void *vroot, __free_fn_t freefct)
{
  __tdestroy (vroot, freefct);
}
