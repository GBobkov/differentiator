#ifndef TREE_FOR_DIFF_H
#define TREE_FOR_DIFF_H


// Константы типа переменной.
enum DATA_TYPE_CNST
{
    NONE_DATA    =0,
    OP_DATA      =1,
    NUM_DATA     =2,
    VAR_DATA     =4
};



// Структура узла
struct NODE
{
    DATA_TYPE_CNST type;
    int data;
    NODE* left;
    NODE* right;
};



// Считать данные с файла и создать дерево.
int Read_Data(NODE* node);



// Сохранить данные
int Write_Data(NODE* node);



// Сформировать dot-file и png.
void Tree_Dump(const char* dump_fname, NODE* node);

// Создать узел с массивом data.
NODE* Create_Node(DATA_TYPE_CNST var_type, int data,  NODE* left, NODE* right);

// рекурсивно удаляет дерево
void Destroy_Tree(NODE* head);

// удалить конкретный узел
void Destroy_Node(NODE* node);


#endif