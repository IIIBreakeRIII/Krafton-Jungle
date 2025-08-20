#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>

rbtree *new_rbtree(void) {
  
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *nilNode = (node_t *)calloc(1, sizeof(node_t));

  nilNode->color = RBTREE_BLACK;
  p->nil = nilNode;
  p->root = nilNode;

  return p;
}

void right_rotation(rbtree *tree, node_t *x) {
  
  node_t *y;

  y = x->left;
  x->left = y->right;
  
  if (y->right != tree->nil) {
    y->right->parent = x;
  }

  y->parent = x->parent;

  if (x->parent == tree->nil) {
    tree->root = y;
  }
  else if (x == x->parent->left) {
    x->parent->left = y;
  }
  else {
    x->parent->right = y;
  }

  y->right = x;
  x->parent = y;
}

void left_rotation(rbtree *tree, node_t *x) {
  node_t *y;

  y = x->right;
  x->right = y->left;
  
  if (y->left != tree->nil) {
    y->left->parent = x;
  }

  y->parent = x->parent;

  if (x->parent == tree->nil) {
    tree->root = y;
  }
  else if (x == x->parent->left) {
    x->parent->left = y;
  }
  else {
    x->parent->right = y;
  }

  y->left = x;
  x->parent = y;
}

void free_node(rbtree *t, node_t *x) {

  if (x->left != t->nil) {
    free_node(t, x->left);
  }

  if (x->right != t->nil) {
    free_node(t, x->right);
  }

  free(x);

  x = NULL;
}

void delete_rbtree(rbtree *t) {

  if (t->root != t->nil) {
    free_node(t, t->root);
  }

  free(t->nil);
  free(t);
}

void rbtree_insert_fixup(rbtree *t, node_t *z) {
  node_t *y;

  while (z->parent->color == RBTREE_RED) {

    if (z->parent == z->parent->parent->left) {
      y = z->parent->parent->right;

      if (y->color == RBTREE_RED) {
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;

        z->parent->parent->color = RBTREE_RED;
        
        z = z->parent->parent;
      }
      else {
        if (z == z->parent->right) {
          z = z->parent;
          left_rotation(t, z);
        }

        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;

        right_rotation(t, z->parent->parent);
      }
    }
    else {
      y = z->parent->parent->left;

      if (y->color == RBTREE_RED) {
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;

        z->parent->parent->color = RBTREE_RED;

        z = z->parent->parent;
      }
      else {
        if (z == z->parent->left) {
          z = z->parent;
          right_rotation(t, z);
        }

        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;

        left_rotation(t, z->parent->parent);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  
  node_t *y = t->nil;
  node_t *x = t->root;
  node_t *z = (node_t *)calloc(1, sizeof(node_t));

  z->key = key;

  while (x != t->nil) {
    y = x;
    if (z->key < x->key) {
      x = x->left;
    }
    else {
      x = x->right;
    }
  }

  z->parent = y;

  if (y == t->nil) {
    t->root = z;
  }
  else if (z->key < y->key) {
    y->left = z;
  }
  else {
    y->right = z;
  }

  z->left = t->nil;
  z->right = t->nil;
  z->color = RBTREE_RED;

  rbtree_insert_fixup(t, z);

  return z;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  
  node_t *current = t->root;

  while (current != t->nil) {

    if(current->key == key) {
      return current;
    }

    if (current->key < key) {
      current = current->right;
    }
    else {
      current = current->left;
    }
  }

  return NULL;
}

node_t *rbtree_min(const rbtree *t) {

  if (t->root == t->nil) {
    return NULL;
  }
  
  node_t *curr = t->root;

  while (curr->left != t->nil) {
    curr = curr->left;
  }

  return curr;
}

node_t *rbtree_max(const rbtree *t) {
  
  if (t->root == t->nil) {
    return NULL;
  }

  node_t * curr = t->root;

  while (curr->right != t->nil) {
    curr = curr->right;
  }

  return curr;
}

void rbtree_transplant(rbtree *t, node_t *u, node_t *v) {

  if (u->parent == t->nil) {
    t->root = v;
  }
  else if (u == u->parent->left) {
    u->parent->left = v;
  }
  else {
    u->parent->right = v;
  }

  v->parent = u->parent;
}

void rbtree_delete_fixup(rbtree *t, node_t *x) {
  
  while (x != t->root && x->color == RBTREE_BLACK) {
    
    if (x == x->parent->left){
      node_t *w = x->parent->right;
      
      if (w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        
        left_rotation(t, x->parent);
        
        w = x->parent->right;
      }

      if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK) {
        w->color = RBTREE_RED;
        x = x->parent;
      }

      else { 
        
        if (w->right->color == RBTREE_BLACK) {
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          
          right_rotation(t, w);
          
          w = x->parent->right;
        }

        w->color = x->parent->color;
        
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        
        left_rotation(t, x->parent);
        
        x = t->root;
      }
    }
    else {
      node_t *w = x->parent->left;

      if (w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        
        right_rotation(t, x->parent);
        
        w = x->parent->left;
      }

      if (w->right->color == RBTREE_BLACK && w->left->color == RBTREE_BLACK) {
        w->color = RBTREE_RED;
        x = x->parent;
      }

      else 
      {
        if (w->left->color == RBTREE_BLACK) {
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          
          left_rotation(t, w);
          
          w = x->parent->left;
        }

        w->color = x->parent->color;
        
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        
        right_rotation(t, x->parent);
        
        x = t->root;
      }
    }
  }
  x->color = RBTREE_BLACK;
}

int rbtree_erase(rbtree *t, node_t *p) {
  
  node_t *y;
  node_t *x;
  color_t yOriginalColor;

  y = p;
  yOriginalColor = y->color;

  if (p->left == t->nil) {
    x = p->right;
    
    rbtree_transplant(t, p, p->right);
  }
  else if (p->right == t->nil) {
    x = p->left;
    
    rbtree_transplant(t, p, p->left);
  }
  else {
    y = p->right;
    
    while(y->left != t->nil){
      y = y->left;
    }
    
    yOriginalColor = y->color;
    x = y->right;

    if (y->parent == p) {
      x->parent = y;
    } 
    else {
      rbtree_transplant(t, y, y->right);
      
      y->right = p->right;
      y->right->parent = y;
    }

    rbtree_transplant(t, p, y);
    
    y->left = p->left;
    y->left->parent = y;
    y->color = p->color;
  }

  if (yOriginalColor == RBTREE_BLACK) {
    rbtree_delete_fixup(t, x);
  }

  free(p);

  return 0;
}

void subtree_to_array(const rbtree *t, node_t *curr, key_t *arr, size_t n, size_t *count) {
  
  if (curr == t->nil) {
    return;
  }
  
  subtree_to_array(t, curr->left, arr, n, count);
  
  if (*count < n) {
    arr[(*count)++] = curr->key;
  }
  
  else return;
  
  subtree_to_array(t, curr->right, arr, n, count);
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  
  if (t->root == t->nil) {
    return 0;
  }

  size_t cnt = 0;
  
  subtree_to_array(t, t->root, arr, n, &cnt); 
  
  return 0;
}
