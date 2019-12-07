#include "amount_set.h"
#include <stdlib.h>
#include <assert.h>

#define ERROR -1

typedef struct node_t {
  ASElement element;
  double amount;
  struct node_t *next;
} *Node;
struct AmountSet_t {
  CopyASElement user_copy_function;
  FreeASElement user_free_function;
  CompareASElements user_compare_function;
  Node head; // the start of a linked list. 'head' is a dummy.
  Node iterator;
};
static Node getElementNodePtr(AmountSet set, ASElement element);

AmountSet asCreate(CopyASElement copyElement,
                   FreeASElement freeElement,
                   CompareASElements compareElements) {
  if (copyElement == NULL || freeElement == NULL || compareElements == NULL) {
    return NULL;
  }
  AmountSet new_set = malloc(sizeof(*new_set));
  if (new_set == NULL) {
    return NULL;
  }
  // initializing all fields
  new_set->user_compare_function = compareElements;
  new_set->user_free_function = freeElement;
  new_set->user_copy_function = copyElement;
  new_set->head = malloc(sizeof(*(new_set->head)));
  if (new_set->head == NULL) {
    free(new_set);
    return NULL;
  }
  // first node in linked list is a dummy
  new_set->head->next = NULL;
  new_set->head->element = NULL;
  new_set->head->amount = 0;
  return new_set;
}

void asDestroy(AmountSet set) {
  if (set == NULL) {
    return;
  }
  /* going through every the list's node, making sure it isn't NULL,
   * using the user's free function, freeing the element and then the node. */
  Node prior_node = NULL;
  Node current_node = set->head->next;
  while (current_node != NULL) {
    assert(current_node != set->head);
    set->user_free_function(current_node->element);
    prior_node = current_node;
    current_node = current_node->next;
    free(prior_node);
  }
  /* eventually, freeing the dummy node and the set itself.
   * the internal iterator may point somewhere, but all the nodes are
   * already freed so no need to free the iterator as well.*/
  free(set->head);
  free(set);
}

bool asContains(AmountSet set, ASElement element) {
  if (set == NULL || set->head == NULL || element == NULL) {
    return false;
  }
  Node node_ptr;
  /* going through all the nodes in the list until we find the element, using
   * the user's compare function, or until the list is over. */
  for (node_ptr = set->head->next; node_ptr != NULL;
       node_ptr = node_ptr->next) {
    if (node_ptr->element == NULL) {
      // there shouldn't be a NULL element in the middle of the list .
      return false;
    }
    if (set->user_compare_function(element, node_ptr->element) == 0) {
      return true;
    }
  }
  return false;
}

int asGetSize(AmountSet set) {
  if (set == NULL) {
    return ERROR; //error value
  }
  int size = 0;
  /* going through all the linked list, promoting the size value until list
   is over */
  Node node_ptr = set->head->next;
  while (node_ptr != NULL) {
    node_ptr = node_ptr->next;
    size++;
  }
  return size;
}

AmountSetResult asGetAmount(AmountSet set, ASElement element, double
*outAmount) {
  Node node_ptr;
  if (set == NULL || element == NULL || outAmount == NULL) {
    return AS_NULL_ARGUMENT;
  }
  if (asContains(set, element) == false) {
    return AS_ITEM_DOES_NOT_EXIST;
  }
  // extracting the element's amount
  node_ptr = getElementNodePtr(set, element);
  *outAmount = node_ptr->amount;
  return AS_SUCCESS;
}

AmountSet asCopy(AmountSet set) {
  if (set == NULL) {
    return NULL;
  }
  // creating a new empty AS with the given set's functions.
  AmountSet new_set = asCreate(set->user_copy_function,
                               set->user_free_function,
                               set->user_compare_function);
  if (new_set == NULL) {
    return NULL;
  }
  Node node_ptr_copy_from = set->head->next;
  /* a pointer the the first node in AS's linked list.
   * will be promoted later */
  if (node_ptr_copy_from == NULL) {
    // if AS is empty, return an empty one as well.
    return new_set;
  }
  while (node_ptr_copy_from != NULL) {
    AmountSetResult result = asRegister(new_set,
                                        node_ptr_copy_from->element);
    // adding the element from node_ptr_copy_from to the new AS.
    if (result == AS_NULL_ARGUMENT) {
      // if failed, we must free the allocated memory.
      asDestroy(new_set);
      return NULL;
    }
    // we need a pointer to the nodes who holds the element
    Node node_ptr_copy_to =
        getElementNodePtr(new_set,
                          node_ptr_copy_from->element);

    assert(node_ptr_copy_to != NULL && node_ptr_copy_to->next == NULL);

    // assigning all fields.
    node_ptr_copy_to->amount = node_ptr_copy_from->amount;
    node_ptr_copy_from = node_ptr_copy_from->next;
  }
  Node ptr_to_first_element =
      getElementNodePtr(new_set, set->head->next->element);
  // we need a pointer to first node to connect the new set to the linked list.
  new_set->head->next = ptr_to_first_element;
  return new_set;
}

AmountSetResult asRegister(AmountSet set, ASElement element) {
  if (set == NULL || element == NULL) {
    return AS_NULL_ARGUMENT;
  }
  if (asContains(set, element)) {
    return AS_ITEM_ALREADY_EXISTS;
  }
  Node node_before = set->head;
  Node node_after = set->head->next;
  // will be used to connect the linked list properly.

  while (node_after != NULL) {
    if (set->user_compare_function(element, node_after->element) < 0) {
      break;
      /*loop runs until it reaches a bigger element (by user_compare_function)
       * or the end of the list (NULL) */
    }
    node_before = node_after;
    node_after = node_after->next;
  }
  Node new_node = malloc(sizeof(*new_node));
  // the node which will hold the element.
  if (new_node == NULL) {
    return AS_OUT_OF_MEMORY;
  }
  // assigning all field.
  new_node->element = set->user_copy_function(element);
  new_node->amount = 0;
  new_node->next = node_after != NULL ? node_after : NULL;
  node_before->next = new_node;
  return AS_SUCCESS;
}

AmountSetResult asDelete(AmountSet set, ASElement element) {
  if (set == NULL || element == NULL) {
    return AS_NULL_ARGUMENT;
  }
  if (!asContains(set, element)) {
    return AS_ITEM_DOES_NOT_EXIST;
  }
  Node node_before = set->head;
  Node node_to_delete = set->head->next;
  // will be used to connect the linked list properly.

  // the loop rubs until the element is found or the linked list is over
  while (node_to_delete != NULL) {
    if (set->user_compare_function(element, node_to_delete->element) == 0) {
      break;
    }
    node_before = node_to_delete;
    node_to_delete = node_to_delete->next;
  }
  if (node_to_delete == NULL) {
    // if linked list is over and the element isn't found.
    return AS_ITEM_DOES_NOT_EXIST;
  }
  // freeing the element and connecting the nodes properly.
  set->user_free_function(node_to_delete->element);
  node_before->next = node_to_delete->next;
  free(node_to_delete);
  return AS_SUCCESS;
}

AmountSetResult asClear(AmountSet set) {
  if (set == NULL) {
    return AS_NULL_ARGUMENT;
  }
  // a pointer to the first node in linked list, will be promoted later.
  Node next_node = set->head->next;
  Node prior_node = NULL;
  while (next_node != NULL) {
    // as long as the linked list isn't over
    prior_node = next_node;
    set->user_free_function(prior_node->element);
    next_node = next_node->next;
    free(prior_node);
  }
  set->head->next = NULL;
  return AS_SUCCESS;
}

AmountSetResult asChangeAmount(AmountSet set, ASElement element, const double
amount) {
  if (set == NULL || element == NULL) {
    return AS_NULL_ARGUMENT;
  }
  if (asContains(set, element) == false) {
    return AS_ITEM_DOES_NOT_EXIST;
  }
  // getting the node that holds the element
  Node node_of_element = getElementNodePtr(set, element);
  assert(node_of_element != NULL);
  if (node_of_element->amount + amount < 0) {
    return AS_INSUFFICIENT_AMOUNT;
  }
  node_of_element->amount = node_of_element->amount + amount;
  return AS_SUCCESS;
}

ASElement asGetFirst(AmountSet set) {
  if (set == NULL || set->head->next == NULL) {
    //if the list is empty
    return NULL;
  }
  // the head is dummy. elements start at head->next .
  set->iterator = set->head->next;
  return set->iterator->element;
}

ASElement asGetNext(AmountSet set) {
  if (set == NULL || set->head->next == NULL) {
    // if the list is empty
    return NULL;
  }
  set->iterator = set->iterator->next;
  if (set->iterator == NULL) {
    return NULL;
  }
  return set->iterator->element;
}


/* the function receives the AS and a wanted element,
 * and going through the linked list until element is found
 * (if exists) and returning a pointer to the node that holds
 * the wanted element. */
static Node getElementNodePtr(AmountSet set, ASElement element) {
  if (set == NULL || element == NULL) {
    return NULL;
  }
  Node node_ptr = set->head->next;
  while (node_ptr != NULL) {
    if (set->user_compare_function(node_ptr->element, element) == 0) {
      return node_ptr;
    }
    node_ptr = node_ptr->next;
  }
  return NULL;
}