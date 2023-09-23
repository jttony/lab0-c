#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list)
        q_release_element(entry);

    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    element->value = strdup(s);
    if (!element->value) {
        free(element);
        return false;
    }

    list_add(&element->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    element->value = strdup(s);
    if (!element->value) {
        free(element);
        return false;
    }

    list_add_tail(&element->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *elem = list_first_entry(head, element_t, list);
    list_del(&elem->list);

    if (sp) {
        for (char *i = elem->value; bufsize > 1 && *i; sp++, i++, bufsize--)
            *sp = *i;
        *sp = '\0';
    }

    return elem;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *elem = list_last_entry(head, element_t, list);
    list_del(&elem->list);

    if (sp) {
        for (char *i = elem->value; bufsize > 1 && *i; sp++, i++, bufsize--)
            *sp = *i;
        *sp = '\0';
    }

    return elem;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int size = 0;
    struct list_head *node;
    list_for_each (node, head)
        size++;

    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *slow, *fast;
    slow = fast = head->next;
    for (; fast != head && (fast = fast->next) != head; fast = fast->next)
        slow = slow->next;

    element_t *elem = list_entry(slow, element_t, list);
    list_del(&elem->list);
    q_release_element(elem);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;

    bool dup = false;
    element_t *prev = NULL, *curr;
    list_for_each_entry (curr, head, list) {
        bool equal = prev && !strcmp(prev->value, curr->value);
        if (equal || dup) {
            list_del(&prev->list);
            q_release_element(prev);
            dup = equal;
        }
        prev = curr;
    }
    if (dup) {
        list_del(&prev->list);
        q_release_element(prev);
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head) {
            break;
        }

        list_move(node, node->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head) 
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;

    struct list_head *node = head->next;
    for (;;) {
        struct list_head *safe = node, *start = node->prev;
        for (int i = 0; i < k; i++, node = node->next) {
            if (node == head)
                return;
        }
        node = safe;
        safe = node->next;
        for (int i = 0; i < k; i++, node = safe, safe = safe->next)
            list_move(node, start);
    }
}


static int q_merge_two(struct list_head *first, struct list_head *second, bool descend)
{
    if (!first || !second)
        return 0;

    int size = 0;
    LIST_HEAD(tmp);
    while (!list_empty(first) && !list_empty(second)) {
        element_t *f = list_first_entry(first, element_t, list);
        element_t *s = list_first_entry(second, element_t, list);
        if ((descend && strcmp(f->value, s->value) > 0) || (!descend && strcmp(f->value, s->value) <= 0))
            list_move_tail(&f->list, &tmp);
        else
            list_move_tail(&s->list, &tmp);
        size++;
    }

    size += q_size(first);
    list_splice_tail_init(first, &tmp);
    
    size += q_size(second);
    list_splice_tail_init(second, &tmp);
    list_splice(&tmp, first);

    return size;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) 
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return; // Nothing to sort
    }

    struct list_head *slow = head, *fast = head->next;
    for (; fast != head && fast->next != head; fast = fast->next->next)
        slow = slow->next;
    struct list_head left;
    list_cut_position(&left, head, slow);
    q_sort(&left, descend);
    q_sort(head, descend);
    q_merge_two(head, &left, descend);

}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    element_t *entry = NULL;
    int size = 0;
    list_for_each_entry (entry, head, list) {
        struct list_head *prev = entry->list.prev, *safe = prev->prev;
        for (; prev != head; prev = safe, safe = safe->prev) {
            element_t *prev_entry = list_entry(prev, element_t, list);
            if (strcmp(prev_entry->value, entry->value) <= 0)
                break;
            size--;
            list_del(prev);
            q_release_element(prev_entry);
        }
        size++;
    }
    return size;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    element_t *entry = NULL;
    int size = 0;
    list_for_each_entry (entry, head, list) {
        struct list_head *prev = entry->list.prev, *safe = prev->prev;
        for (; prev != head; prev = safe, safe = safe->prev) {
            element_t *prev_entry = list_entry(prev, element_t, list);
            if (strcmp(prev_entry->value, entry->value) >= 0)
                break;
            size--;
            list_del(prev);
            q_release_element(prev_entry);
        }
        size++;
    }
    return size;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);

    int queue_size = 0;
    queue_contex_t *first, *second;
    first = list_first_entry(head, queue_contex_t, chain),
    second = list_entry(first->chain.next, queue_contex_t, chain);
    while (second->q) {
        queue_size = q_merge_two(first->q, second->q, descend);
        second->q = NULL;
        list_move_tail(&second->chain, head);
        second = list_entry(first->chain.next, queue_contex_t, chain);
    }
    return queue_size;
}
