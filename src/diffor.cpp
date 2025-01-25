#include "diffor.h"
#include "tree_for_diff.h"
#include "optimizator.h"
#include "write_data.h"
#include "read_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


static const char *_dump_nonoptim_dirt_fname = "build/dump_diritive.dot";


// Рекурсивная функция вычисления производной. Возвращает указатель на начальный узел. Также меняет подаваемый аргумент.
static NODE* New_Step_Drvt(NODE* node)
{
    if (node->type == NUM_DATA)
    {
        node->data = 0;
        return node;
    }
    else if (node->type == VAR_DATA)
    {
        node->type = NUM_DATA;
        node->data = 1;
        return node;
    }
    else if (node->type == OP_DATA)
    {   
        NODE* left = node->left;
        NODE* right = node->right;
        
        if (node->data == OP_SUM || node->data == OP_SUB)
        {
            node->left = New_Step_Drvt(left);
            node->right = New_Step_Drvt(right);

        }
        else if (node->data == OP_MUL)
        {   
            node->left = Create_Node(OP_DATA, OP_MUL, New_Step_Drvt(Copy_Node(left)), right);
            node->right = Create_Node(OP_DATA, OP_MUL, left, New_Step_Drvt(Copy_Node(right)));
            node->data = OP_SUM;

        }
        else if (node->data == OP_DIV)
        {
            node->left = Create_Node(OP_DATA, OP_SUB, Create_Node(OP_DATA, OP_MUL, New_Step_Drvt(Copy_Node(left)), right), Create_Node(OP_DATA, OP_MUL, left, New_Step_Drvt(Copy_Node(right))));
            node->right = Create_Node(OP_DATA, OP_DEG, Copy_Node(right), Create_Num_Node(2));
            node->data = OP_DIV;

        }
        else if (node->data == OP_LN || node->data == OP_LOG)
        {
            if (Is_Num(node->right)) return Rewrite_Node2Zero(node);

            node->left = New_Step_Drvt(Copy_Node(right));

            // если логарифм не натуральный
            if (node->data == OP_LOG)
            {
                NODE* new_left = Create_Node(OP_DATA, OP_LN, Create_Num_Node(NONEXISTENT_VAL), left);
                node->right = Create_Node(OP_DATA, OP_MUL, new_left, right);
            }
            else
                Destroy_Tree(left);

            node->data = OP_DIV;

        }
        else if (node->data == OP_SIN || node->data == OP_SINH || node->data == OP_COSH)
        {
            if (Is_Num(node->right)) Rewrite_Node2Zero(node);

            int diff_op = NONEXISTENT_VAL;
            if (node->data == OP_SIN) diff_op = OP_COS;
            else if (node->data == OP_SINH) diff_op = OP_COSH;
            else diff_op = node->data;

            node->left = Create_Node(OP_DATA, diff_op, left, right);
            node->right = New_Step_Drvt(Copy_Node(right));
            node->data = OP_MUL;            

        }
        else if (node->data == OP_COS)
        {
            if (Is_Num(node->right)) return Rewrite_Node2Zero(node);
        
            node->left = Create_Num_Node(-1);
            node->right = Create_Node(OP_DATA, OP_MUL, Create_Node(OP_DATA, OP_SIN, left, right), New_Step_Drvt(Copy_Node(right)));
            node->data = OP_MUL;

        }
        else if (node->data == OP_TAN || node->data == OP_TANH)
        {
            if (Is_Num(node->right)) return Rewrite_Node2Zero(node);

            node->left = New_Step_Drvt(Copy_Node(right));
            node->right = Create_Node(OP_DATA, OP_DEG, Create_Node(OP_DATA, (node->data == OP_TAN)? OP_COS: OP_COSH, left, right), Create_Num_Node(2));
            node->data = OP_DIV;    

        }
        else if (node->data == OP_COT || node->data == OP_COTH)
        {
            if (Is_Num(node->right)) return Rewrite_Node2Zero(node);
            
            node->left = Create_Node(OP_DATA, OP_MUL, Create_Num_Node(-1), New_Step_Drvt(Copy_Node(right)));
            node->right = Create_Node(OP_DATA, OP_DEG, Create_Node(OP_DATA, (node->data == OP_COT)? OP_SIN: OP_SINH, left, right), Create_Num_Node(2));
            node->data = OP_DIV;

        }
        else if (node->data == OP_DEG)
        {
            if (Is_Num(left))
            {
                // константа
                if (Is_Num(right))
                {
                    node->type = NUM_DATA;
                    node->data = 0;
                    Destroy_Tree(left);
                    Destroy_Tree(right);
                    node->left = NULL;
                    node->right = NULL;
                }
                else // показательная функция
                {
                    node->left = Create_Node(OP_DATA, OP_MUL, Create_Node(OP_DATA, OP_LN, Create_Node(NUM_DATA, 0, NULL, NULL), Create_Node(NUM_DATA, left->data, NULL, NULL)), Create_Node(OP_DATA, OP_DEG, left, right));
                    node->right = New_Step_Drvt(Copy_Node(right));
                    node->data = OP_MUL;
                }
                
            }
            else if (!Is_Num(left) && Is_Num(right)) // степенная функция
            {
                int degree = Calculate_Tree(right);
                node->left = Create_Node(NUM_DATA, degree, NULL, NULL);
                node->right = Create_Node(OP_DATA, OP_MUL, Create_Node(OP_DATA, OP_DEG, left, Create_Node(NUM_DATA, degree - 1, NULL, NULL)), New_Step_Drvt(Copy_Node(left)));
                node->data = OP_MUL;
                Destroy_Tree(right);
            }
            else if (!Is_Num(left) && !Is_Num(right)) // функция показательная с переменным основанием.
            {
                node->left = Create_Node(node->type, node->data, left, right);
                node->right = New_Step_Drvt(Create_Node(OP_DATA, OP_MUL, Create_Node(OP_DATA, OP_LN, Create_Node(NUM_DATA, 0, NULL, NULL), Copy_Node(left)), Copy_Node(node->right)));
                node->data = OP_MUL;
            }
        }
    }

    return node;
}


// Функция вычисляет произвоную.
NODE* Calculate_Derivative(NODE* head)
{
    head = New_Step_Drvt(head);
    Tree_Dump(_dump_nonoptim_dirt_fname, head);
    Optimizator(head);

    return head;
}
