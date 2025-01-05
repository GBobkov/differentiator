#include "diffor.h"
#include "tree_for_diff.h"
#include "optimizator.h"
#include "write_data.h"
#include "read_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


static int step_counter = 1;

static NODE* Rewrite_Node2Zero(NODE* node)
{
    node->type = NUM_DATA;
    Destroy_Tree(node->left);
    Destroy_Tree(node->right);
    node->data = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Рекурсивная функция вычисления производной
static NODE* New_Step_Drvt(NODE* node)
{

    if (node->type == NUM_DATA)
    {
        node->data = 0;
        
        return node;
    }
    if (node->type == VAR_DATA)
    {
        node->type = NUM_DATA;
        node->data = 1;
        
        return node;
    }
    if (node->type == OP_DATA)
    {   
        NODE* left = node->left;
        NODE* right = node->right;
        if (node->data == '+' || node->data == '-')
        {
            node->left = New_Step_Drvt(left);
            node->right = New_Step_Drvt(right);
            
            return node;
        }
        if (node->data == '*')
        {   
            node->data = '+';   
            node->left = Create_Node(OP_DATA, '*', New_Step_Drvt(Copy_Node(left)), right);
            node->right = Create_Node(OP_DATA, '*', left, New_Step_Drvt(Copy_Node(right)));
            
            return node;
        }
        if (node->data == '/')
        {
            node->data = '/';

            node->left = Create_Node(OP_DATA, '-', Create_Node(OP_DATA, '*', New_Step_Drvt(Copy_Node(left)), right), Create_Node(OP_DATA, '*', left, New_Step_Drvt(Copy_Node(right))));
            node->right = Create_Node(OP_DATA, '^', Copy_Node(right), Create_Node(NUM_DATA, 2, NULL, NULL));
            
            
            return node;
        }
        if (node->data == 'l')
        {
            if (Is_Num(node->right)) return Rewrite_Node2Zero(node);

            node->data = '/';
            node->left = New_Step_Drvt(Copy_Node(right));
            Destroy_Node(left);
            return node;
        }
        if (node->data == 's')
        {
            if (Is_Num(node->right)) Rewrite_Node2Zero(node);
            node->data = '*';
            node->left = Create_Node(OP_DATA, 'c', left, right);
            node->right = New_Step_Drvt(Copy_Node(right));
            
            return node;
        }
        if (node->data == 'c')
        {
            if (Is_Num(node->right)) return Rewrite_Node2Zero(node);
            
            node->data = '*';
            node->left = Create_Node(NUM_DATA, -1, NULL, NULL);
            node->right = Create_Node(OP_DATA, '*', Create_Node(OP_DATA, 's', left, right), New_Step_Drvt(Copy_Node(right)));
            return node;
        }
        if (node->data == '^')
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
                    printf("I'mhere\n");
                    node->data = '*';
                    node->left = Create_Node(OP_DATA, '*', Create_Node(OP_DATA, 'l', Create_Node(NUM_DATA, 0, NULL, NULL), Create_Node(NUM_DATA, left->data, NULL, NULL)), Create_Node(OP_DATA, '^', left, right));
                    node->right = New_Step_Drvt(Copy_Node(right));
                }
                
            }
            else if (!Is_Num(left) && Is_Num(right)) // степенная функция
            {
                node->data = '*';
                int degree = Calculate_Tree(right);
                node->left = Create_Node(NUM_DATA, degree, NULL, NULL);
                node->right = Create_Node(OP_DATA, '*', Create_Node(OP_DATA, '^', left, Create_Node(NUM_DATA, degree - 1, NULL, NULL)), New_Step_Drvt(Copy_Node(left)));
                Destroy_Tree(right);
            }
            else if (!Is_Num(left) && !Is_Num(right)) // функция показательная с переменным основанием.
            {
                node->data = '*';
                node->left = Create_Node(node->type, node->data, left, right);
                node->right = New_Step_Drvt(Create_Node(OP_DATA, '*', Create_Node(OP_DATA, 'l', Create_Node(NUM_DATA, 0, NULL, NULL), Copy_Node(left)), Copy_Node(node->right)));
            }
            
            return node;
        }
    }
    return NULL;
}

static const char *_dump_start_fname = "build/dump_start.dot";
static const char *_dump_nonoptim_dirt_fname = "build/dump_diritive.dot";


// Функция вычисляет произвоную.
void Calculate_Derivative(void)
{
    NODE* head = Handle_Read_Request();
    Open_LaTEX_File();
    Write_New_Line_To_LaTEX("\n\\[\n(", head, ")^{'} = ");
    Tree_Dump(_dump_start_fname, head);
    head = New_Step_Drvt(head);
    Tree_Dump(_dump_nonoptim_dirt_fname, head);
    int changes = 0;
    do
    {
        changes = 0;
        Optimization(head, &changes);
        //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
    } while (changes > 0);

    Write_New_Line_To_LaTEX("\n", head, "\n\\]\n");
    Write_Data2Base(head);
    Tree_Dump("build/dump_end.dot", head);
    //Close_LaTEX_File();
    Destroy_Tree(head);
}
