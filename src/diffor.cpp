#include "diffor.h"
#include "tree_for_diff.h"

#include <stdio.h>


#FIXME: Исправить фигню со связями в графе. аккуратнее с возвращающим узлом и созданием узла (проследить за уникальностью узла.) 
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
        node->data = 0;
        return node;
    }
    if (node->type == OP_DATA)
    {
        printf("node->data=%d, node->type=%d\n", node->data, node->type);
        if (node->data == '+' || node->data == '-')
        {
            node->left = New_Step_Drvt(node->left);
            node->right = New_Step_Drvt(node->right);
            return node;
        }
        if (node->data == '*')
        {
            printf("Ti pidoras!\n");
            node->data = '+';
            NODE* left = node->left;
            NODE* right = node->right;
            node->left = Create_Node(OP_DATA, '*', node, New_Step_Drvt(left), right);
            node->right = Create_Node(OP_DATA, '*', node, left, New_Step_Drvt(right));
            printf("Suuuda nahui!\n");
            return Create_Node(node->type, node->data, node->parent, node->left, node->right);
        }
        if (node->data == '^')
        {
            if (node->left->type == NUM_DATA)
            {
                node->type = NUM_DATA;
                node->data = 0;
                Destroy_Tree(node->left);
                Destroy_Tree(node->right);
                node->left = NULL;
                node->right = NULL;
                return Create_Node(node->type, node->data, node->parent, node->left, node->right);
            }
            else if (node->left->type == VAR_DATA && node->right->type == NUM_DATA)
            {
                node->data = '*';
                Destroy_Tree(node->left);
                node->left = Create_Node(NUM_DATA, node->right->data, node, NULL, NULL);
                Destroy_Tree(node->right);
                node->right = Create_Node(OP_DATA, '*', node, Create_Node(VAR_DATA, 'x', NULL, NULL, NULL), Create_Node(NUM_DATA, node->left->data - 1, NULL, NULL, NULL));
                return Create_Node(node->type, node->data, node->parent, node->left, node->right);
            }
        }
        
    }
    
}


// Функция вычисляет произвоную.
void Calculate_Derivative(void)
{
    NODE* head = Create_Node(NONE_DATA, '\0', NULL, NULL, NULL);
    Read_Data(head);
    Node_Dump("dump.dot", head);
    head = New_Step_Drvt(head);
    Write_Data(head);
    Node_Dump("dump.dot", head);
}
