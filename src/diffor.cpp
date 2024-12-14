#include "diffor.h"
#include "tree_for_diff.h"
#include "optimizator.h"
#include "write_data.h"

#include <stdio.h>
#include <assert.h>

// NODE* global_tree_head = NULL;

// Рекурсивная функция вычисления производной
static NODE* New_Step_Drvt(NODE* node)
{
    //printf("New_func_activated\n"); Tree_Dump("smotrim.dot", node); scanf("%c");

    if (node->type == NUM_DATA)
    {
        node->data = 0;
        //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
        return Create_Node(NUM_DATA, 0, NULL, NULL);
    }
    if (node->type == VAR_DATA)
    {
        node->type = NUM_DATA;
        node->data = 1;
        //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
        return Create_Node(NUM_DATA, 1, NULL, NULL);
    }
    if (node->type == OP_DATA)
    {
        
        if (node->data == '+' || node->data == '-')
        {
            //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
            node->left = New_Step_Drvt(node->left);
            node->right = New_Step_Drvt(node->right);
            //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
            return node;
        }
        if (node->data == '*')
        {
            //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
            node->data = '+';
            NODE* left = node->left;
            NODE* right = node->right;
            node->left = Create_Node(OP_DATA, '*', New_Step_Drvt(Copy_Node(left)), Copy_Node(right));
            node->right = Create_Node(OP_DATA, '*', Copy_Node(left), New_Step_Drvt(Copy_Node(right)));
            //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
            return Copy_Node(node);
        }
        if (node->data == '/')
        {
            node->data = '/';

            NODE* left = node->left;
            NODE* right = node->right;
            node->left = Create_Node(OP_DATA, '-', Create_Node(OP_DATA, '*', New_Step_Drvt(left), Copy_Node(right)), Create_Node(OP_DATA, '*', left, Copy_Node(right)));
            node->right = Create_Node(OP_DATA, '^', Copy_Node(right), Create_Node(NUM_DATA, 2, NULL, NULL));
            
            return Copy_Node(node);
        }
        if (node->data == 'l')
        {
            //printf("IT's 1lnDUMP!\n");Tree_Dump("smotrim.dot", node); scanf("%c");
            if (Is_Num(node->right)) return Create_Node(NUM_DATA, 0, NULL, NULL);
            
            node->data = '/';
            //printf("node->right->data=%c\n", node->right->data);
            node->left = New_Step_Drvt(Copy_Node(node->right));
            //printf("node->right->data=%c\n", node->right->data);
            //printf("IT's 2lnDUMP!\n");Tree_Dump("smotrim.dot", node); scanf("%c");
            return Copy_Node(node);
        }
        if (node->data == 's')
        {
            if (Is_Num(node->right)) return Create_Node(NUM_DATA, 0, NULL, NULL);

            //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
            node->data = '*';
            node->left = Create_Node(OP_DATA, 'c', Create_Node(NUM_DATA, 0, NULL, NULL), Copy_Node(node->right));
            node->right = New_Step_Drvt(node->right);
            //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
            return Copy_Node(node);
        }
        if (node->data == 'c')
        {
            if (Is_Num(node->right)) return Create_Node(NUM_DATA, 0, NULL, NULL);
            
            node->data = '*';
            node->left = Create_Node(NUM_DATA, -1, NULL, NULL);
            node->right = Create_Node(OP_DATA, '*', Create_Node(OP_DATA, 's', Create_Node(NUM_DATA, 1, NULL, NULL), New_Step_Drvt(node->right)), New_Step_Drvt(node->right));
            return Copy_Node(node);
        }
        if (node->data == '^')
        {
            if (Is_Num(node->left))
            {
                // константа
                if (Is_Num(node->right))
                {
                    node->type = NUM_DATA;
                    node->data = 0;
                    Destroy_Tree(node->left);
                    Destroy_Tree(node->right);
                    node->left = NULL;
                    node->right = NULL;
                }
                else // показательная функция
                {
                    node->left = Create_Node(OP_DATA, '*', Create_Node(OP_DATA, 'l', Create_Node(NUM_DATA, 0, NULL, NULL), Create_Node(NUM_DATA, node->left->data, NULL, NULL)), Copy_Node(node));
                    node->right = New_Step_Drvt(node->right);
                    node->data = '*';
                }
                
            }
            else if (!Is_Num(node->left) && Is_Num(node->right)) // степенная функция
            {
                node->data = '*';
                int degree = Calculate_Tree(node->right);
                node->right = Create_Node(OP_DATA, '*', Create_Node(OP_DATA, '^', Copy_Node(node->left), Create_Node(NUM_DATA, degree - 1, NULL, NULL)), New_Step_Drvt(node->left));
                node->left = Create_Node(NUM_DATA, degree, NULL, NULL);
                Destroy_Tree(node->right);
                
                
            }
            else if (!Is_Num(node->left) && !Is_Num(node->right)) // функция показательная с переменным основанием.
            {

                //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
                
                NODE* old_node = node->left;
                node->left = Copy_Node(node);
                Destroy_Tree(old_node);
                old_node = node->right;
                //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
                node->right = New_Step_Drvt(Create_Node(OP_DATA, '*', Create_Node(OP_DATA, 'l', Create_Node(NUM_DATA, 1, NULL, NULL), Copy_Node(node->left->left)), Copy_Node(node->right)));
                Destroy_Tree(old_node);
                node->data = '*';
                //printf("stepen' newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", node); scanf("%c");
            }
            
            return Copy_Node(node);
        }
    }
    return NULL;
}

static FILE* latex_ptr = NULL;
static const char *_dump_start_fname = "dump_start.dot";
static const char *_dump_nonoptim_dirt_fname = "dump_diritive.dot";


// Функция вычисляет произвоную.
void Calculate_Derivative(void)
{
    NODE* head = Create_Node(NONE_DATA, '\0', NULL, NULL);
    Handle_Read_Request(head);
    Open_LaTEX_File();
    //global_tree_head = head;
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

    Write_Data2Base(head);
    Write_Data2LaTEX(head);
    Tree_Dump("dump_end.dot", head);
    Close_LaTEX_File();
}
