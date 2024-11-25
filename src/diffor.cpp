#include "diffor.h"
#include "tree_for_diff.h"

#include <stdio.h>


// Рекурсивная функция вычисления производной
static NODE* New_Step_Drvt(NODE* node)
{

    if (node->type = NUM_DATA)
    {
        node->data = 0;
        return node;
    }
    if (node->type = OP_DATA)
    {
        switch (node->data)
        {
        case '+':
        case '-':
            node->left = New_Step_Drvt(node->left);
            node->right = New_Step_Drvt(node->right);
            return node;
        
        case '*':
            {
                node->data = '+';
                NODE* left = node->left;
                NODE* right = node->right;
                node->left = Create_Node(OP_DATA, '*', node, New_Step_Drvt(left), right);
                node->right = Create_Node(OP_DATA, '*', node, left, New_Step_Drvt(right));
            }
        case '^':
            if (node->left->type == NUM_DATA)
            {
                node->type = NUM_DATA;
                node->data = 0;
                Destroy_Tree(node->left);
                Destroy_Tree(node->right);
                node->left = NULL;
                node->right = NULL;
            }
            else if (node->left->type == VAR_DATA && node->right->type == NUM_DATA)
            {
                node->data = '*';
                Destroy_Tree(node->left);
                node->left = Create_Node(NUM_DATA, node->right->data, node, NULL, NULL);
                Destroy_Tree(node->right);
                node->right = Create_Node(OP_DATA, '*', node, Create_Node(VAR_DATA, 'x', NULL, NULL, NULL), Create_Node(NUM_DATA, node->left->data, NULL, NULL, NULL));
            }
        default:
            break;
        }
    }
    
}


// Функция вычисляет произвоную.
void Calculate_Derivative(void)
{
    NODE* head = Create_Node(NONE_DATA, '\0', NULL, NULL, NULL);
    Read_Data(head);
    New_Step_Drvt(head);
    Write_Data(head);
    Node_Dump("dump.dot", head);
}
