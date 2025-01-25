#ifndef CONSTANTS_H
#define CONSTANTS_H
// Константы типа переменной.
enum DATA_TYPE_CNST
{
    NONE_DATA    = 0,
    OP_DATA      = 1,
    NUM_DATA     = 2,
    VAR_DATA     = 4
};



// Структура узла
struct NODE
{
    DATA_TYPE_CNST type;
    int data;
    NODE* left;
    NODE* right;
};


// операции
enum OPERATIONS
{
    OP_LN  =  1,
    OP_LOG =  2,
    OP_COS =  3,
    OP_SIN =  4,
    OP_TAN =  5,
    OP_COT =  6,
    OP_SINH  =  7, 
    OP_COSH  =  8, 
    OP_TANH  =  9,
    OP_COTH =  10,
    OP_SUM =  11,
    OP_SUB =  12,
    OP_MUL =  13,
    OP_DIV =  14,
    OP_DEG =  15

};


enum ERROR_FLAGS
{
    NONEXISTENT_VAL = 666 
};

#endif