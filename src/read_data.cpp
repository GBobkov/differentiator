#include "read_data.h"

#include "tree_for_diff.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>



static void Skip_Space(const char* user_line_expression, int* user_line_pointer)
{
    while (user_line_expression[*user_line_pointer] == ' ') *user_line_pointer += 1;

}


static void SyntaxErr(const char* user_line_expression, int* user_line_pointer, const char* message)
{
    printf("\n%s\n", user_line_expression);
    for (int i = 0; i < *user_line_pointer; i++) printf(" ");
    printf("^\n");

    printf("Unpredictable symbol at position %d\n", *user_line_pointer);
    printf("symbol=%c\n", user_line_expression[*user_line_pointer]);
    printf("message:%s\n", message);
    abort();
}

// Обработать сумму.
static NODE* Get_Sumsub(const char* user_line_expression, int* user_line_pointer); // определяю для того, чтобы зациклить спуск.

// Обработать число.
static NODE* Get_Number(const char* user_line_expression, int* user_line_pointer)
{
    Skip_Space(user_line_expression, user_line_pointer);
    int val = 0;
    int oldp = *user_line_pointer;
    while (user_line_expression[*user_line_pointer] >= '0' && user_line_expression[*user_line_pointer] <= '9')
    {
        val = val * 10 + user_line_expression[*user_line_pointer] - '0';
        *user_line_pointer += 1;
        Skip_Space(user_line_expression, user_line_pointer);
    }
    
    if (*user_line_pointer == oldp)
        SyntaxErr(user_line_expression, user_line_pointer, "user_line_pointer == oldp");
    
    return Create_Node(NUM_DATA, val, NULL, NULL);
}

// Обработать "ядро" - выражение в скобках, или переменная, или число.
static NODE* Get_Kernel(const char* user_line_expression, int* user_line_pointer)
{
    Skip_Space(user_line_expression, user_line_pointer);
    if (user_line_expression[*user_line_pointer] == '(')
    {
        *user_line_pointer += 1;
        NODE* tree = Get_Sumsub(user_line_expression, user_line_pointer);
        if (user_line_expression[*user_line_pointer] != ')')
            SyntaxErr(user_line_expression, user_line_pointer, "user_line_expression[user_line_pointer] != ')'");
        *user_line_pointer += 1;
        return tree;
    }
    else
    {
        if (user_line_expression[*user_line_pointer] == 'x') 
        {
            *user_line_pointer += 1;
            return Create_Node(VAR_DATA, 'x', NULL, NULL);
        }
        else 
            {
                return Get_Number(user_line_expression, user_line_pointer);
            }
    }
}
 


// Поиск функций в подстроке. Возвращает код функции если нашлась, иначе NONEXISTENT_VAL.
static int Find_Func(const char* user_line_expression, int* user_line_pointer)
{
    // Проверка на подстроку "name" в user_line_expression, возвращает код функции если нашлась, иначе NONEXISTENT_VAL.
    #define CODEGEN(name)\
    {\
        const char *funcname = #name;\
        int pointer = 0;\
        bool find_flag = true;\
        while (funcname[pointer] != '\0')\
        {\
            if (tolower(funcname[pointer]) != user_line_expression[*user_line_pointer + pointer]) {find_flag = false; break;}\
            pointer++;\
        }\
        if (find_flag)\
        {\
            *user_line_pointer += pointer;\
            return OP_##name;\
        }\
    }
    #include "func_codegen.h"
    #undef CODEGEN

    return NONEXISTENT_VAL;
}

static NODE* Get_Log_Base(const char* user_line_expression, int* user_line_pointer)
{
    if (user_line_expression[*user_line_pointer] != '_')
        SyntaxErr(user_line_expression, user_line_pointer, "Expected '_' in log base.");
    *user_line_pointer += 1;

    return Get_Kernel(user_line_expression, user_line_pointer);
}


// обработать функцию.
static NODE* Get_Func(const char* user_line_expression, int* user_line_pointer)
{
    Skip_Space(user_line_expression, user_line_pointer);
    int opfunc_data = Find_Func(user_line_expression, user_line_pointer);
    if (opfunc_data != NONEXISTENT_VAL)
    {
        if (opfunc_data == OP_LOG)
        {
            NODE *base_node = Get_Log_Base(user_line_expression, user_line_pointer);
            return Create_Node(OP_DATA, OP_LOG, base_node, Get_Func(user_line_expression, user_line_pointer));
        }
        return Create_Node(OP_DATA, opfunc_data, Create_Num_Node(NONEXISTENT_VAL), Get_Func(user_line_expression, user_line_pointer));
    }

    return Get_Kernel(user_line_expression, user_line_pointer);
}

// обработать возведение в степень.
static NODE* Get_Degree(const char* user_line_expression, int* user_line_pointer)
{
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Func(user_line_expression, user_line_pointer);
    Skip_Space(user_line_expression, user_line_pointer);
    if (user_line_expression[*user_line_pointer] == '^')
    {
        *user_line_pointer += 1;
        op_tree = Create_Node(OP_DATA, OP_DEG, left_tree, Get_Degree(user_line_expression, user_line_pointer));
    }
   
    
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать умножение.
static NODE* Get_Muldiv(const char* user_line_expression, int* user_line_pointer)
{
   
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Degree(user_line_expression, user_line_pointer);
    Skip_Space(user_line_expression, user_line_pointer);
    if (user_line_expression[*user_line_pointer] == '*' || user_line_expression[*user_line_pointer] == '/')
        {
            char sign = (user_line_expression[*user_line_pointer] == '*')? OP_MUL: OP_DIV;
            *user_line_pointer += 1;
            op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Muldiv(user_line_expression, user_line_pointer));
        }
   
    if (op_tree) return op_tree;
    return left_tree;
}

// Обработать сумму.
NODE* Get_Sumsub(const char* user_line_expression, int* user_line_pointer)
{
    
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Muldiv(user_line_expression, user_line_pointer);
    Skip_Space(user_line_expression, user_line_pointer);
    if (user_line_expression[*user_line_pointer] == '+' || user_line_expression[*user_line_pointer] == '-')
    {
        char sign = (user_line_expression[*user_line_pointer] == '+')? OP_SUM: OP_SUB;
        *user_line_pointer += 1;
        op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Sumsub(user_line_expression, user_line_pointer));
    }
    
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать дерево 
static NODE* Get_Expression_Tree(const char* user_line_expression)
{
    int user_line_pointer = 0;
    NODE* tree = Get_Sumsub(user_line_expression, &user_line_pointer);
    if (user_line_expression[user_line_pointer] != '\0')
        SyntaxErr(user_line_expression, &user_line_pointer, "user_line_expression[user_line_pointer] != '\\0'");

    return tree;
}


// работа с пользователем (читать из базы данных или из ввода)
NODE* Handle_Read_Request(char* user_line_expression)
{
    printf("Enter expression:");
    scanf("%[^\n]", user_line_expression);
    
    return Get_Expression_Tree(user_line_expression);
}