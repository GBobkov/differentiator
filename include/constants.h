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


// операции
enum OPERATIONS
{
    SUM_OP='+',
    SUB_OP='-',
    MUL_OP='*',
    DIV_OP='/',
    LN_OP=1,
    COS_OP=2,
    SIN_OP=3,
    TAN_OP=4,
    COT_OP=5
};


#endif