#include "b_tree.h"

// TODO: validate key labels - should be equal to first one on the right, not last one on the left?

const uint32_t ID_SIZE = size_of_attribute(Row, id);              // 4
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);  // 32 + 1 for null char
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);        // 255 + 1 for null char
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;              // 4
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;     // 36
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;   // 291

/*
 * Common Node Header Layout
 */
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/*
 * Leaf Node Header Layout
 */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET =
    LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                       LEAF_NODE_NUM_CELLS_SIZE +
                                       LEAF_NODE_NEXT_LEAF_SIZE;

/*
 * Leaf Node Body Layout
 */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

/*
 * Internal Node Header Layout
 */
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                           INTERNAL_NODE_NUM_KEYS_SIZE +
                                           INTERNAL_NODE_RIGHT_CHILD_SIZE;

/*
 * Internal Node Body Layout
 */
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE =
    INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;
/* Keep this small for testing */
const uint32_t INTERNAL_NODE_MAX_CELLS = 3;
const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

bool is_node_root(void* node) {
  uint8_t value = *((uint8_t*)(node + IS_ROOT_OFFSET));
  return (bool)value;
}

void set_node_root(void* node, bool is_root) {
  uint8_t value = is_root;
  *((uint8_t*)(node + IS_ROOT_OFFSET)) = value;
}

/**
 * get pointer to the cell number count for this leaf node
 */
uint32_t* leaf_node_num_cells(void* node) {
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

/**
 * get pointer to the cell n on the leaf node
 */
void* leaf_node_cell(void* node, uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

/**
 * get pointer to the cell's key value (same location as the cell itself)
 */
uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num);
}

/**
 * get pointer to the cell's database entry
 */
Row* leaf_node_value(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

/**
 * get pointer to the page number of the next leaf
 */
uint32_t* leaf_node_next_leaf(void* node) {
  return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}

NodeType get_node_type(void* node) {
  uint8_t value = *((uint8_t*)(node + NODE_TYPE_OFFSET));
  return (NodeType)value;
}

void set_node_type(void* node, NodeType type) {
  uint8_t value = type;
  *((uint8_t*)(node + NODE_TYPE_OFFSET)) = value;
}

/**
 * create an empty leaf node at the given page location
 * note: sets right leaf node page number at 0 (root)...
 * ... does not handle links to parent/siblings
 */
void initialize_leaf_node(void* new_leaf_node) {
  set_node_type(new_leaf_node, NODE_LEAF);
  set_node_root(new_leaf_node, false);
  *leaf_node_num_cells(new_leaf_node) = 0;
  *leaf_node_next_leaf(new_leaf_node) = 0;  // 0 represents no sibling (points to the root)
}

/**
 * get pointer to the number of keys set on the internal node
 */
uint32_t* internal_node_num_keys(void* node) {
  return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

/**
 * create an empty internal node at the given page location
 * note: does not handle links to parent/siblings
 */
void initialize_internal_node(void* new_internal_node) {
  set_node_type(new_internal_node, NODE_INTERNAL);
  set_node_root(new_internal_node, false);
  *internal_node_num_keys(new_internal_node) = 0;
}

/**
 * get pointer to page number of rightmost leaf node belonging to the internal node
 */
uint32_t* internal_node_right_child(void* node) {
  return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}

/**
 * get pointer to internal node's cell child
 * note: internal nodes have child pointer before their key
 */
uint32_t* internal_node_cell(void* node, uint32_t cell_num) {
  return node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}

/**
 * get pointer to the page number for node's child
 */
uint32_t* internal_node_child(void* node, uint32_t child_num) {
  uint32_t num_keys = *internal_node_num_keys(node);
  if (child_num > num_keys) {
    printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
    exit(EXIT_FAILURE);
  } else if (child_num == num_keys) {
    return internal_node_right_child(node);
  } else {
    return internal_node_cell(node, child_num);
  }
}

/**
 * get pointer to internal node's key
 */
uint32_t* internal_node_key(void* node, uint32_t key_num) {
  return internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE / sizeof(uint32_t);
}

/**
 * get value of node's max key
 */
uint32_t get_node_max_key(void* node) {
  NodeType node_type = get_node_type(node);

  if (node_type == NODE_INTERNAL) {
    uint32_t num_keys = *internal_node_num_keys(node);
    return *internal_node_key(node, num_keys - 1);
  } else {
    uint32_t num_cells = *leaf_node_num_cells(node);
    return *leaf_node_key(node, num_cells - 1);
  }
}

/*
 * Return the index of the child which should contain the given key.
 */
uint32_t internal_node_find_child(void* node, uint32_t key) {

  uint32_t num_keys = *internal_node_num_keys(node);

  /* Binary search */
  uint32_t min_index = 0;
  uint32_t max_index = num_keys; /* there is one more child than key */

  while (min_index != max_index) {
    uint32_t index = (min_index + max_index) / 2;
    uint32_t key_to_right = *internal_node_key(node, index);
    if (key_to_right >= key) {
      max_index = index;
    } else {
      min_index = index + 1;
    }
  }
  return min_index;
}

/**
 * get pointer to node parent's page number
 */
uint32_t* node_parent(void* node) { return node + PARENT_POINTER_OFFSET; }

/*
Until we start recycling free pages, new pages will always
go onto the end of the database file
*/
uint32_t get_unused_page_num(Pager* pager) { return pager->num_pages; }

/**
 * split the root!
 * all the contents of the old root are copied into a new left_child
 * new page number passed in to become the right child
 * 
 * @param table pointer to the table
 * @param right_child_page_num (unused) page number for the new right child
 */
void create_new_root(Table* table, uint32_t right_child_page_num) {

  /*
  Handle splitting the root.
  Old root copied to new page, becomes left child.
  Address of right child passed in.
  Re-initialize root page to contain the new root node.
  New root node points to two children.
  */

  void* root = get_page(table->pager, table->root_page_num);
  void* right_child = get_page(table->pager, right_child_page_num);
  uint32_t left_child_page_num = get_unused_page_num(table->pager);
  void* left_child = get_page(table->pager, left_child_page_num);

  /* Left child has data copied from old root */
  memcpy(left_child, root, PAGE_SIZE);
  set_node_root(left_child, false);

  /* Root node is a new internal node with one key and two children */
  initialize_internal_node(root);
  set_node_root(root, true);
  *internal_node_num_keys(root) = 1;
  *internal_node_child(root, 0) = left_child_page_num;
  uint32_t left_child_max_key = get_node_max_key(left_child);
  *internal_node_key(root, 0) = left_child_max_key;
  *internal_node_right_child(root) = right_child_page_num;

  *node_parent(left_child) = table->root_page_num;
  *node_parent(right_child) = table->root_page_num;
}

/**
 * split the internal node and insert
 * 
 * @param table pointer to the table
 * @param old_node_page_num old internal node's page number
 * @param insert_child_page_num new inserted leaf's page number
 *
 */
void internal_node_split_and_insert(Table* table, uint32_t old_node_page_num, uint32_t insert_child_page_num) {
// TODO: give page numbers a type alias

  void* old_node = get_page(table->pager, old_node_page_num);
  void* insert_child = get_page(table->pager, insert_child_page_num);

  // previous max + the right child + the new insert
  uint32_t num_children = INTERNAL_NODE_MAX_CELLS + 2;

  uint32_t* temp_pages = (uint32_t*) malloc(num_children * sizeof(uint32_t));
  // last one doesn't need a key because it can be the right child of the second internal node
  uint32_t* temp_keys = (uint32_t*) malloc((num_children - 1) * sizeof(uint32_t));

  // line up all the keys and pointers in the temporary structure
  for (int32_t i = 0, j = 0; i < num_children; i++, j++) {
    uint32_t insert_child_max_key = get_node_max_key(insert_child);
    uint32_t node_child_key = *internal_node_key(old_node, j);

    // scenario 1: you insert the new leaf into position
    if (insert_child_max_key < node_child_key) {
      temp_keys[i] = insert_child_max_key;
      temp_pages[i] = insert_child_page_num;
      j--; // decrement J because you haven't moved anything from the old_node in this round of the loop
    }
    // scenario 2: you are at the end, and you add the new leaf as the right child pointer
    else if (i == num_children - 1) {
      temp_pages[i] = insert_child_page_num;
    }
    // scenario 3: you are dealing with the old node's right side pointer
    else if (j == INTERNAL_NODE_MAX_CELLS) {
      uint32_t page = *internal_node_right_child(old_node);
      void* node = get_page(table->pager, page);
      temp_keys[i] = get_node_max_key(node);
      temp_pages[i] = page;
    }
    // scenario 4: you insert the old leaf into position
    else {
      uint32_t page = *internal_node_child(old_node, j);
      void* node = get_page(table->pager, page);
      temp_keys[i] = get_node_max_key(node);
      temp_pages[i] = page;
    }
  }

  int32_t new_node_page_num = get_unused_page_num(table->pager);
  void* new_node = get_page(table->pager, new_node_page_num);

  int32_t slice_index = num_children / 2; // 2

  // first half goes to old node
  *(internal_node_num_keys(old_node)) = 0;
  for (int32_t i = 0; i < slice_index; i++) {
    // either it's one of the child-key cells...
    if (i < slice_index - 1) {
      *(internal_node_cell(old_node, i)) = temp_pages[i];
      *(internal_node_key(old_node, i)) = temp_keys[i];
      *(internal_node_num_keys(old_node)) = *internal_node_num_keys(old_node) + 1;
    }
    // ...or it's the right child
    else {
      *(internal_node_right_child(old_node)) = temp_pages[i];
    }
  }

  // second half goes to new node
  *(internal_node_num_keys(new_node)) = 0;
  for (int32_t j = slice_index, k = 0; j < num_children; j++, k++) {
    if (j < num_children - 1) {
      *(internal_node_cell(new_node, k)) = temp_pages[j];
      *(internal_node_key(new_node, k)) = temp_keys[j];
      *(internal_node_num_keys(new_node)) = *internal_node_num_keys(new_node) + 1;
    } else {
      *(internal_node_right_child(new_node)) = temp_pages[j];
    }
  }

  // Last but not least - attach the new internal node onto the parent
  if (old_node_page_num == 0) {
    // if the old node was the root, then you split it and add the new node as its right child
    create_new_root(table, new_node_page_num);
  } else {
    // otherwise you add the new node onto the parent
    int32_t parent_page_num = *node_parent(old_node);
    internal_node_insert(table, parent_page_num, new_node_page_num);
  }
}

void internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num) {
  /*
  Add a new child/key pair to parent that corresponds to child
  */

  void* parent = get_page(table->pager, parent_page_num);
  void* child = get_page(table->pager, child_page_num);
  uint32_t child_max_key = get_node_max_key(child);
  uint32_t index = internal_node_find_child(parent, child_max_key);

  uint32_t original_num_keys = *internal_node_num_keys(parent);
  *internal_node_num_keys(parent) = original_num_keys + 1;
  uint32_t right_child_page_num = *internal_node_right_child(parent);
  void* right_child = get_page(table->pager, right_child_page_num);

  if (original_num_keys >= INTERNAL_NODE_MAX_CELLS) {

    internal_node_split_and_insert(table, parent_page_num, child_page_num);

  } else if (child_max_key > get_node_max_key(right_child)) {
    /* Replace right child */
    *internal_node_child(parent, original_num_keys) = right_child_page_num;
    *internal_node_key(parent, original_num_keys) =
        get_node_max_key(right_child);
    *internal_node_right_child(parent) = child_page_num;
  } else {
    /* Make room for the new cell */
    for (uint32_t i = original_num_keys; i > index; i--) {
      void* destination = internal_node_cell(parent, i);
      void* source = internal_node_cell(parent, i - 1);
      memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
    }
    *internal_node_child(parent, index) = child_page_num;
    *internal_node_key(parent, index) = child_max_key;
  }
}


void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key) {
  uint32_t old_child_index = internal_node_find_child(node, old_key);
  *internal_node_key(node, old_child_index) = new_key;
}

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value) {
  /*
  Create a new node and move half the cells over.
  Insert the new value in one of the two nodes.
  Update parent or create a new parent.
  */

  void* old_node = get_page(cursor->table->pager, cursor->page_num);
  uint32_t old_max = get_node_max_key(old_node);
  uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
  void* new_node = get_page(cursor->table->pager, new_page_num);
  initialize_leaf_node(new_node);
  *node_parent(new_node) = *node_parent(old_node);
  *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
  *leaf_node_next_leaf(old_node) = new_page_num;

  /*
  All existing keys plus new key should be divided
  evenly between old (left) and new (right) nodes.
  Starting from the right, move each key to correct position.
  */
  for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
    void* destination_node;
    if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
      destination_node = new_node;
    } else {
      destination_node = old_node;
    }
    uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
    void* destination = leaf_node_cell(destination_node, index_within_node);

    if (i == cursor->cell_num) {
      serialize_row(value, leaf_node_value(destination_node, index_within_node));
      *leaf_node_key(destination_node, index_within_node) = key;
    } else if (i > cursor->cell_num) {
      memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
    } else {
      memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }
  }

  /* Update cell count on both leaf nodes */
  *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
  *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

  if (is_node_root(old_node)) {
    return create_new_root(cursor->table, new_page_num);
  } else {
    uint32_t parent_page_num = *node_parent(old_node);
    uint32_t new_max = get_node_max_key(old_node);
    void* parent = get_page(cursor->table->pager, parent_page_num);

    update_internal_node_key(parent, old_max, new_max);
    internal_node_insert(cursor->table, parent_page_num, new_page_num);
    return;
  }
}


void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value) {
  void* node = get_page(cursor->table->pager, cursor->page_num);

  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS) {
    // Node full
    leaf_node_split_and_insert(cursor, key, value);
    return;
  }

  if (cursor->cell_num < num_cells) {
    // Make room for new cell
    for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
             LEAF_NODE_CELL_SIZE);
    }
  }

  *(leaf_node_num_cells(node)) += 1;
  *(leaf_node_key(node, cursor->cell_num)) = key;
  serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;

  // Binary search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    uint32_t index = (min_index + one_past_max_index) / 2;
    uint32_t key_at_index = *leaf_node_key(node, index);
    if (key == key_at_index) {
      cursor->cell_num = index;
      return cursor;
    }
    if (key < key_at_index) {
      one_past_max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  cursor->cell_num = min_index;
  return cursor;
}

Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);

  uint32_t child_index = internal_node_find_child(node, key);
  uint32_t child_num = *internal_node_child(node, child_index);
  void* child = get_page(table->pager, child_num);

  if (get_node_type(child) == NODE_INTERNAL) {
    return internal_node_find(table, child_num, key);
  } else {
    return leaf_node_find(table, child_num, key);
  }
}
