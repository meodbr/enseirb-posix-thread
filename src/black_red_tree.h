#ifndef __BLACK_RED_TREE_H__
#define __BLACK_RED_TREE_H__

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/**
 * @brief Your Red-Black Tree type
*/
#define BRTREE(type) struct type##_BRTREE_STRUCT

/**
 * @brief Definition of your Red-Black Tree struct
 * Put it after your type definition
 * @param type The name of the struct your tree will store
*/
#define BRTREE_DEF(type) \
    struct type##_BRTREE_STRUCT \
    { \
        struct type *root_node; \
    }

/**
 * @brief Initialize your Red-Black Tree
 * 
 * @param tree Pointer to your Tree
*/
#define BRTREE_INITIALIZE(tree) \
    (tree)->root_node = NULL

/**
 * @brief Initialize your Red-Black Tree (alternative way)
*/
#define BRTREE_INITIALIZER {NULL}

/**
 * @brief Put an instance of this type in your type definition
*/
#define BRTREE_ENTRY(type) struct type##_BRTREE_ENTRY_STRUCT

/**
 * @brief Definition of your Red-Black Tree Entry struct
 * Put this before your type definition
 * @param type The name of the struct your tree will store
*/
#define BRTREE_ENTRY_DEF(type) \
    struct type##_BRTREE_ENTRY_STRUCT \
    { \
        struct type *left_node; \
        struct type *right_node; \
        struct type *parent_node; \
        int color; \
        long long key; \
    }

#define BRTREE_GET_ENTRY(name) (name)->brtree_entry

#define BRTREE_LEAF NULL
#define BRTREE_COLOR_RED 0
#define BRTREE_COLOR_BLACK 1

/**
 * @brief initialize entry of your object n
 * 
 * @param n Pointer to your object containing the entry
 * @param _key The key you want to give
*/
#define BRTREE_ENTRY_INITIALIZE(n, _key) \
    do { \
        (BRTREE_GET_ENTRY(n)).left_node = NULL; \
        (BRTREE_GET_ENTRY(n)).right_node = NULL; \
        (BRTREE_GET_ENTRY(n)).parent_node = NULL; \
        (BRTREE_GET_ENTRY(n)).color = BRTREE_COLOR_RED; \
        (BRTREE_GET_ENTRY(n)).key = _key; \
    } while (0)

#define BRTREE_EMPTY(tree) (BRTREE_ROOT(tree) == NULL)

/**
 * @brief The key of your object 
*/
#define BRTREE_KEY(object) (BRTREE_GET_ENTRY(object)).key

#define BRTREE_ROOT(tree) (tree)->root_node

#define BRTREE_PARENT(n) (BRTREE_GET_ENTRY(n)).parent_node
#define BRTREE_LCHILD(n) (BRTREE_GET_ENTRY(n)).left_node
#define BRTREE_RCHILD(n) (BRTREE_GET_ENTRY(n)).right_node
#define BRTREE_COLOR(n) (BRTREE_GET_ENTRY(n)).color

#define BRTREE_GRANDPARENT(n) (BRTREE_PARENT(n) != NULL ? BRTREE_PARENT(BRTREE_PARENT(n)) : NULL)

#define BRTREE_BROTHER(n) \
        (BRTREE_PARENT(n) == NULL ? \
            NULL : \
            ((n) == BRTREE_LCHILD(BRTREE_PARENT(n))) ? \
                BRTREE_RCHILD(BRTREE_PARENT(n)) : \
                BRTREE_LCHILD(BRTREE_PARENT(n)))

#define BRTREE_UNCLE(enfant) (BRTREE_GRANDPARENT(enfant) == NULL ? NULL : BRTREE_BROTHER(BRTREE_PARENT(enfant)))

#define BRTREE_CHANGE_CHILD_FROM_PARENT(current, new, tree) \
    do { \
        if (BRTREE_PARENT(current) == NULL) { \
            BRTREE_ROOT(tree) = new; \
        } else if (BRTREE_RCHILD(BRTREE_PARENT(current)) == current)  \
            BRTREE_RCHILD(BRTREE_PARENT(current)) = new; \
        else \
            BRTREE_LCHILD(BRTREE_PARENT(current)) = new; \
    } while (0)

#define BRTREE_NOT_LEAF_CHILD(parent) (BRTREE_RCHILD(parent) != BRTREE_LEAF ? BRTREE_RCHILD(parent) : BRTREE_LCHILD(parent))

#define BRTREE_IS_LEFT_CHILD(n) (BRTREE_LCHILD(BRTREE_PARENT(n)) == n)

#define BRTREE_IS_BLACK(n) (n == NULL ? 1 : BRTREE_COLOR(n) == BRTREE_COLOR_BLACK)

#define BRTREE_IS_IN_TREE(n, tree) (BRTREE_PARENT(n) != NULL || BRTREE_ROOT(tree) == n)

#define BRTREE_LEFT_ROTATION(origin, tree, type) \
    do { \
        struct type *x = origin; \
        struct type *y = BRTREE_RCHILD(x); \
        BRTREE_RCHILD(x) = BRTREE_LCHILD(y); \
        if (BRTREE_LCHILD(y) != BRTREE_LEAF) \
            BRTREE_PARENT(BRTREE_LCHILD(y)) = (x); \
        BRTREE_PARENT(y) = BRTREE_PARENT(x); \
        if (BRTREE_PARENT(x) == NULL) \
            BRTREE_ROOT(tree) = (y); \
        else if ((x) == BRTREE_LCHILD(BRTREE_PARENT(x))) \
            BRTREE_LCHILD(BRTREE_PARENT(x)) = y; \
        else \
            BRTREE_RCHILD(BRTREE_PARENT(x)) = y; \
        BRTREE_LCHILD(y) = (x); \
        BRTREE_PARENT(x) = (y); \
    } while (0)

#define BRTREE_RIGHT_ROTATION(origin, tree, type) \
    do { \
        struct type *x = origin; \
        struct type *y = BRTREE_LCHILD(x); \
        BRTREE_LCHILD(x) = BRTREE_RCHILD(y); \
        if (BRTREE_RCHILD(y) != BRTREE_LEAF) \
            BRTREE_PARENT(BRTREE_RCHILD(y)) = (x); \
        BRTREE_PARENT(y) = BRTREE_PARENT(x); \
        if (BRTREE_PARENT(x) == NULL) \
            BRTREE_ROOT(tree) = y; \
        else if ((x) == BRTREE_RCHILD(BRTREE_PARENT(x))) \
            BRTREE_RCHILD(BRTREE_PARENT(x)) = y; \
        else \
            BRTREE_LCHILD(BRTREE_PARENT(x)) = y; \
        BRTREE_RCHILD(y) = x; \
        BRTREE_PARENT(x) = y; \
    } while (0)

#define BRTREE_INSERT_CAS5(n, tree, type) \
    do { \
        struct type *p = BRTREE_PARENT(n); \
        struct type *g = BRTREE_GRANDPARENT(n); \
        if ((n) == BRTREE_LCHILD(p)) { \
            BRTREE_RIGHT_ROTATION(g, tree, type); \
        } else { \
            BRTREE_LEFT_ROTATION(g, tree, type); \
        } \
        BRTREE_COLOR(p) = BRTREE_COLOR_BLACK; \
        BRTREE_COLOR(g) = BRTREE_COLOR_RED; \
    } while (0)

#define BRTREE_INSERT_CAS4(n, tree, type) \
    do { \
        struct type *p = BRTREE_PARENT(n); \
        struct type *g = BRTREE_GRANDPARENT(n); \
        if (BRTREE_LCHILD(g) != NULL) \
            if ((n) == BRTREE_RCHILD(BRTREE_LCHILD(g))) { \
                BRTREE_LEFT_ROTATION(p, tree, type); \
                (n) = BRTREE_LCHILD(n); \
            } \
        if (BRTREE_RCHILD(g) != NULL) \
            if ((n) == BRTREE_LCHILD(BRTREE_RCHILD(g))) { \
                BRTREE_RIGHT_ROTATION(p, tree, type); \
                (n) = BRTREE_RCHILD(n); \
            } \
        BRTREE_INSERT_CAS5(n, tree, type); \
    } while (0)

#define BRTREE_INSERT_REPAIR_TREE(node, tree, type) \
    do { \
        struct type *n = node; \
        while (n != NULL) { \
            if (BRTREE_PARENT(n) == NULL) {\
                BRTREE_COLOR(n) = BRTREE_COLOR_BLACK; \
                BRTREE_ROOT(tree) = n; \
                break; \
            } \
            else if (BRTREE_COLOR(BRTREE_PARENT(n)) == BRTREE_COLOR_BLACK) \
                break; \
            else { \
                if (BRTREE_UNCLE(n) != NULL) \
                    if (BRTREE_COLOR(BRTREE_UNCLE(n)) == BRTREE_COLOR_RED) { \
                        BRTREE_COLOR(BRTREE_PARENT(n)) = BRTREE_COLOR_BLACK; \
                        BRTREE_COLOR(BRTREE_UNCLE(n)) = BRTREE_COLOR_BLACK; \
                        n = BRTREE_GRANDPARENT(n); \
                        BRTREE_COLOR(n) = BRTREE_COLOR_RED; \
                        continue; \
                    } \
                BRTREE_INSERT_CAS4(n, tree, type); \
                break; \
            } \
        } \
    } while (0)

/**
 * @brief Insert an object in your tree
 * 
 * @param n Pointer to the object to insert
 * @param tree Pointer to the tree to insert in
 * @param type The type of your object n
*/
#define BRTREE_INSERT(n, tree, type) \
    do { \
        /*printf("BRTREE_INSERT : n=%p\n", n);*/ \
        if (BRTREE_IS_IN_TREE(n, tree)) break; \
        struct type *racine_ins = BRTREE_ROOT(tree); \
        int is_already_in = 0; \
        /*BRTREE_COLOR(n) = BRTREE_COLOR_RED;*/ \
        while (racine_ins != NULL) { \
            if (n == racine_ins) { \
                is_already_in = 1; \
                break; \
            } \
            if (BRTREE_KEY(n) < BRTREE_KEY(racine_ins)) { \
                if (BRTREE_LCHILD(racine_ins) != BRTREE_LEAF) { \
                    racine_ins = BRTREE_LCHILD(racine_ins); \
                    continue; \
                } else { \
                    BRTREE_LCHILD(racine_ins) = (n); \
                    break; \
                } \
            } else { \
                if (BRTREE_RCHILD(racine_ins) != BRTREE_LEAF) { \
                    racine_ins = BRTREE_RCHILD(racine_ins); \
                    continue; \
                } else { \
                    BRTREE_RCHILD(racine_ins) = (n); \
                    break; \
                } \
            } \
        } \
        if (!is_already_in) { \
            BRTREE_PARENT(n) = (racine_ins); \
            BRTREE_LCHILD(n) = BRTREE_LEAF; \
            BRTREE_RCHILD(n) = BRTREE_LEAF; \
            BRTREE_COLOR(n) = BRTREE_COLOR_RED; \
            BRTREE_INSERT_REPAIR_TREE(n, tree, type); \
        } \
    } while (0)

#define BRTREE_MIN(root, res) \
    do { \
        res = root; \
        while (BRTREE_LCHILD(res) != NULL) { \
            res = BRTREE_LCHILD(res); \
        } \
    } while (0)

#define BRTREE_ERASE_CAS_DOUBLE_NOIR(n, n_parent, tree, type) \
    do { \
        struct type *x = n; \
        struct type *p = n_parent; \
        while (p != NULL) { /*cas 0*/ \
            struct type *b; \
            struct type *opp, *adj; \
            int is_left_child; \
            if (x == NULL) is_left_child = (BRTREE_LCHILD(p) == x); \
            else is_left_child = BRTREE_IS_LEFT_CHILD(x); \
            if (is_left_child) { \
                b = BRTREE_RCHILD(p); \
                adj = BRTREE_LCHILD(b); \
                opp = BRTREE_RCHILD(b); \
            } else { \
                b = BRTREE_LCHILD(p); \
                adj = BRTREE_RCHILD(b); \
                opp = BRTREE_LCHILD(b); \
            } \
            if (BRTREE_IS_BLACK(b)) { \
                if (!BRTREE_IS_BLACK(opp)) { /*cas 1b*/ \
                    if (is_left_child) BRTREE_LEFT_ROTATION(p, tree, type); \
                    else BRTREE_RIGHT_ROTATION(p, tree, type); \
                    BRTREE_COLOR(b) = BRTREE_COLOR(p); \
                    BRTREE_COLOR(p) = BRTREE_COLOR_BLACK; \
                    BRTREE_COLOR(opp) = BRTREE_COLOR_BLACK; \
                    break; \
                } else if (!BRTREE_IS_BLACK(adj)) { /*cas 1c*/ \
                    if (is_left_child) BRTREE_RIGHT_ROTATION(b, tree, type); \
                    else BRTREE_LEFT_ROTATION(b, tree, type); \
                    BRTREE_COLOR(b) = BRTREE_COLOR_RED; \
                    BRTREE_COLOR(adj) = BRTREE_COLOR_BLACK; \
                    if (is_left_child) BRTREE_LEFT_ROTATION(p, tree, type); \
                    else BRTREE_RIGHT_ROTATION(p, tree, type); \
                    BRTREE_COLOR(b) = BRTREE_COLOR(p); \
                    break; \
                } else { /*cas 1a*/\
                    BRTREE_COLOR(b) = BRTREE_COLOR_RED; \
                    if (BRTREE_IS_BLACK(p)) { \
                        x = p; \
                        p = BRTREE_PARENT(x); \
                        continue; \
                    } else { \
                        BRTREE_COLOR(p) = BRTREE_COLOR_BLACK; \
                        break; \
                    } \
                } \
            } else { /*cas 2*/ \
                if (is_left_child) BRTREE_LEFT_ROTATION(p, tree, type); \
                else BRTREE_RIGHT_ROTATION(p, tree, type); \
                BRTREE_COLOR(p) = BRTREE_COLOR_RED; \
                BRTREE_COLOR(b) = BRTREE_COLOR_BLACK; \
                continue; \
            } \
        } \
    } while (0)

#define BRTREE_ERASE_SWITCH(to_supp, to_switch, tree) \
    do { \
        BRTREE_PARENT(to_switch) = BRTREE_PARENT(to_supp); \
        BRTREE_LCHILD(to_switch) = BRTREE_LCHILD(to_supp); \
        BRTREE_RCHILD(to_switch) = BRTREE_RCHILD(to_supp); \
        BRTREE_COLOR(to_switch) = BRTREE_COLOR(to_supp); \
        BRTREE_CHANGE_CHILD_FROM_PARENT(to_supp, to_switch, tree); \
        if (BRTREE_RCHILD(to_switch) != NULL) BRTREE_PARENT(BRTREE_RCHILD(to_switch)) = to_switch; \
        if (BRTREE_LCHILD(to_switch) != NULL) BRTREE_PARENT(BRTREE_LCHILD(to_switch)) = to_switch; \
    } while (0)

/**
 * @brief remove an object from your tree
 * 
 * @param node Pointer to the object to erase
 * @param tree Pointer to the tree to erase from
 * @param type The type of your object node
*/
#define BRTREE_ERASE(node, tree, type) \
    do { \
        struct type *to_del = node; \
        struct type *n = node; \
        if (BRTREE_LCHILD(to_del) != NULL && BRTREE_RCHILD(to_del) != NULL) { \
            BRTREE_MIN(BRTREE_RCHILD(to_del), n); \
        } \
        struct type *f = BRTREE_NOT_LEAF_CHILD(n); \
        \
        if (BRTREE_COLOR(n) == BRTREE_COLOR_RED) { \
            BRTREE_CHANGE_CHILD_FROM_PARENT(n, f, tree); \
            if (f != NULL) BRTREE_PARENT(f) = BRTREE_PARENT(n); \
        } else if (f == NULL) { \
            BRTREE_CHANGE_CHILD_FROM_PARENT(n, f, tree); \
            BRTREE_ERASE_CAS_DOUBLE_NOIR(f, BRTREE_PARENT(n), tree, type); \
        } else { \
            if (BRTREE_COLOR(f) == BRTREE_COLOR_RED) { \
                BRTREE_CHANGE_CHILD_FROM_PARENT(n, f, tree); \
                BRTREE_PARENT(f) = BRTREE_PARENT(n); \
                BRTREE_COLOR(f) = BRTREE_COLOR_BLACK; \
            } else { \
                BRTREE_CHANGE_CHILD_FROM_PARENT(n, f, tree); \
                BRTREE_PARENT(f) = BRTREE_PARENT(n); \
                BRTREE_ERASE_CAS_DOUBLE_NOIR(f, BRTREE_PARENT(f), tree, type); \
            } \
        } \
        if (to_del != n) { \
            BRTREE_ERASE_SWITCH(to_del, n, tree); \
            if (to_del == BRTREE_ROOT(tree)) BRTREE_ROOT(tree) = n; \
        } \
        BRTREE_PARENT(to_del) = NULL; \
        BRTREE_LCHILD(to_del) = NULL; \
        BRTREE_RCHILD(to_del) = NULL; \
    } while(0)

/**
 * @brief Gives you the object in the tree that has the smaller key
 * (i.e.) Gives you the leftmost object in the tree
 * @param tree Pointer to the tree
 * @param res Pre-allocated pointer to be filled with the result
*/
#define BRTREE_GET_SMALLER_KEY(tree, res) BRTREE_MIN(BRTREE_ROOT(tree), res)

/**
 * @brief Moves the node n if his key is not matching his tree position
 * 
 * @param n Pointer to your object to be reordered
 * @param tree Pointer to the tree
 * @param type The type of your object node
*/
#define BRTREE_REORDER(n, tree, type) \
    BRTREE_ERASE(n, tree, type); \
    BRTREE_INSERT(n, tree, type);

/**
 * @brief Modifies the key value for object n
 * 
 * @param n Pointer to your object
 * @param _key The value you want to give to the key
 * @param tree Pointer to the tree
 * @param type The type of your object node
*/
#define BRTREE_SET_KEY(n, _key, tree, type) \
    BRTREE_KEY(n) = _key; \
    BRTREE_REORDER(n, tree, type);

#endif // __BLACK_RED_TREE_H__