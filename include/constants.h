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
    SUM_OP=1,
    SUB_OP=2,
    MUL_OP=3,
    DIV_OP=4,
    LN_OP=5,
    COS_OP=6,
    SIN_OP=7,
    TAN_OP=8,
    COT_OP=9
};

static const char* database_file_name = "database.txt";
static const char *_latex_fname = "diritivate.tex";


#endif