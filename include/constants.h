#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BUFSIZE 512


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
    OP_LN  =  2,
    OP_LOG =  4,
    OP_COS =  6,
    OP_SIN =  8,
    OP_TAN =  10,
    OP_COT =  12,
    OP_SINH  =  14, 
    OP_COSH  =  16, 
    OP_TANH  =  18,
    OP_COTH =  20,
    OP_SUM =  1,
    OP_SUB =  3,
    OP_MUL =  5,
    OP_DIV =  7,
    OP_DEG =  9

};

enum BIT_VALUES
{
    BIT_ISCALC_FUNC = 1
};

enum ERROR_FLAGS
{
    NONEXISTENT_VAL = 666 
};

#endif