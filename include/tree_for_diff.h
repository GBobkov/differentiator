#ifndef TREE_FOR_DIFF_H
#define TREE_FOR_DIFF_H

#include "tree_dump.h"
#include "constants.h"


// Создать узел с массивом data.
NODE* Create_Node(DATA_TYPE_CNST var_type, int data,  NODE* left, NODE* right);

// копирует узел с поддеревьями
NODE* Copy_Node(NODE* node);

// рекурсивно удаляет дерево
void Destroy_Tree(NODE* head);

// удалить конкретный узел
void Destroy_Node(NODE* node);


#endif