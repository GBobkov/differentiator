#ifndef CONSTANTS_H
#define CONSTANTS_H
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

#endif