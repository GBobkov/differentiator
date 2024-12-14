#include "tree_for_diff.h"
#include "read_data.h"
#include "write_data.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


// Создать узел с массивом data.
NODE* Create_Node(DATA_TYPE_CNST var_type, int data,  NODE* left, NODE* right)
{

    NODE* node = (NODE *) calloc(1, sizeof(NODE));
    node->type = var_type;
    node->data = data;
    node->left = left;
    node->right = right;

    return node;
}

// копирует узел с поддеревьями
NODE* Copy_Node(NODE* node)
{   
    if (!node) return NULL;
    return Create_Node(node->type, node->data, Copy_Node(node->left), Copy_Node(node->right));
}

// удалить конкретный узел
void Destroy_Node(NODE* node)
{
    assert(node != NULL);
    free(node); 
}

// рекурсивно удаляет дерево
void Destroy_Tree(NODE* head)
{
    if (!head) return;
    
    if (head->right) Destroy_Tree(head->right);
    if (head->left) Destroy_Tree(head->left);
    Destroy_Node(head);
}



