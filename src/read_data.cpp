#include "read_data.h"

#include "tree_for_diff.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>



static char user_line_expression[100] = "3*x^(x^2+2)/ln(sin(x)+5)-1";
static int user_line_pointer = 0;

static void Skip_Space()
{
    while (user_line_expression[user_line_pointer] == ' ') user_line_pointer += 1;
}


static void SyntaxErr(int pos, const char* message)
{
    printf("\n%s\n", user_line_expression);
    for (int i = 0; i < pos; i++) printf(" ");
    printf("^\n");

    printf("Unpredictable symbol at position %d\n", pos);
    printf("symbol=%c\n", user_line_expression[user_line_pointer]);
    printf("message:%s\n", message);
    abort();
}

// Обработать сумму.
NODE* Get_Sumsub(); // определяю для того, чтобы зациклить спуск.

// Обработать число.
static NODE* Get_Number()
{
    Skip_Space();
    int val = 0;
    int oldp = user_line_pointer;
    while (user_line_expression[user_line_pointer] >= '0' && user_line_expression[user_line_pointer] <= '9')
    {
        val = val * 10 + user_line_expression[user_line_pointer] - '0';
        user_line_pointer++;
        Skip_Space();
    }
    
    if (user_line_pointer == oldp)
        SyntaxErr(user_line_pointer, "user_line_pointer == oldp");
    
    return Create_Node(NUM_DATA, val, NULL, NULL);
}

// Обработать "ядро" - выражение в скобках, или переменная, или число.
static NODE* Get_Kernel()
{
    Skip_Space();
    if (user_line_expression[user_line_pointer] == '(')
    {
        user_line_pointer++;
        NODE* tree = Get_Sumsub();
        if (user_line_expression[user_line_pointer] != ')')
            SyntaxErr(user_line_pointer, "user_line_expression[user_line_pointer] != ')'");
        user_line_pointer++;
        return tree;
    }
    else
    {
        if (user_line_expression[user_line_pointer] == 'x') 
        {
            user_line_pointer++;
            return Create_Node(VAR_DATA, 'x', NULL, NULL);
        }
        else 
            {
                return Get_Number();
            }
    }
}
 


// Проверка на подстроку "name" в user_line_expression, возвращает код функции если нашлась, иначе NONEXISTENT_VAL.
#define CODEGEN(name)\
{\
    const char *funcname = #name;\
    int pointer = 0;\
    bool find_flag = true;\
    while (funcname[pointer] != '\0')\
    {\
        if (tolower(funcname[pointer]) != user_line_expression[user_line_pointer + pointer]) find_flag = false;\
        pointer++;\
    }\
    if (find_flag)\
    {\
        user_line_pointer += pointer;\
        return OP_##name;\
    }\
}

// Поиск всевозможных функций в подстроке. Возвращает код функции если нашлась, иначе NONEXISTENT_VAL.
static int Find_Func()
{
    #include "func_codegen.h"
    return NONEXISTENT_VAL;
}
#undef CODEGEN

static NODE* Get_Log_Base()
{
    if (user_line_expression[user_line_pointer] != '_')
        SyntaxErr(user_line_pointer, "Expected '_' in log base.");
    user_line_pointer++;

    return Get_Kernel();
}


// обработать функцию.
static NODE* Get_Func()
{
    Skip_Space();
    int opfunc_data = Find_Func();
    if (opfunc_data != NONEXISTENT_VAL)
    {
        if (opfunc_data == OP_LOG)
        {
            NODE *base_node = Get_Log_Base();
            return Create_Node(OP_DATA, OP_LOG, base_node, Get_Func());
        }
        return Create_Node(OP_DATA, opfunc_data, Create_Num_Node(NONEXISTENT_VAL), Get_Func());
    }

    return Get_Kernel();
}

// обработать возведение в степень.
static NODE* Get_Degree()
{
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Func();
    Skip_Space();
    if (user_line_expression[user_line_pointer] == '^')
    {
        user_line_pointer++;
        op_tree = Create_Node(OP_DATA, OP_DEG, left_tree, Get_Degree());
    }
   
    
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать умножение.
static NODE* Get_Muldiv()
{
   
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Degree();
    Skip_Space();
    if (user_line_expression[user_line_pointer] == '*' || user_line_expression[user_line_pointer] == '/')
        {
            char sign = (user_line_expression[user_line_pointer] == '*')? OP_MUL: OP_DIV;
            user_line_pointer++;
            op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Muldiv());
        }
   
    if (op_tree) return op_tree;
    return left_tree;
}

// Обработать сумму.
NODE* Get_Sumsub()
{
    
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Muldiv();
    Skip_Space();
    if (user_line_expression[user_line_pointer] == '+' || user_line_expression[user_line_pointer] == '-')
    {
        char sign = (user_line_expression[user_line_pointer] == '+')? OP_SUM: OP_SUB;
        user_line_pointer++;
        op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Sumsub());
    }
    
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать дерево 
static NODE* Get_Expression_Tree()
{
    
    NODE* tree = Get_Sumsub();
    if (user_line_expression[user_line_pointer] != '\0')
        SyntaxErr(user_line_pointer, "user_line_expression[user_line_pointer] != '\\0'");

    user_line_pointer++;

    return tree;
}


// работа с пользователем (читать из базы данных или из ввода)
NODE* Handle_Read_Request(void)
{
    //printf("Choose how to get input data:\n[1] read database_file_name=%s\n[2] read expression from consol.\n", database_file_name);
    char answer[10] = {};
    answer[0] = '2';
    while (answer[0] != '1' && answer[0] != '2')
    {
        printf("Input:");
        scanf("%[^\n]", answer);
        getchar(); // Достать '\n'
    }
    if (answer[0] == '1') return NULL;
    else 
    {
        printf("Enter expression:");
        scanf("%[^\n]", user_line_expression);
        return Get_Expression_Tree();
    }
}