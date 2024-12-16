#include "optimizator.h"

#include <stdio.h>

extern NODE* global_tree_head;

// проверка на то, что дерево - число. true если число, false если функция.
bool Is_Num(NODE* head)
{
    if (head->left == NULL && head->right == NULL)
        return (head->type == NUM_DATA);
    
    bool ans = true;
    if (head->left) ans = ans && Is_Num(head->left);
    if (head->right) ans = ans && Is_Num(head->right);

    return ans; 

}


// возводит в степень число
static int pow(int num, int degree)
{
    if (degree == 1) return num;
    return num * pow(num, degree - 1);

}


// Вычислить значение операции.
static int result_value(char operation, int ld, int rd)
{
    if (operation == '*') return ld * rd;
    if (operation == '/') {if (rd == 0) {printf("ALARM DIV BY ZERO!\n");} return ld / rd;}
    if (operation == '+') return ld + rd;
    if (operation == '^') return pow(ld, rd);

    printf("ALARM! DANGEROUS SITUATION!\n");
    return 0;
}


//функция вычисляет численное значение дерева.
int Calculate_Tree(NODE* node)
{
    if (node->type == NUM_DATA) return node->data;
    if (node->left->type == NUM_DATA && node->right->type == NUM_DATA) return result_value(node->data, node->left->data, node->right->data);
    
    return result_value(node->data, Calculate_Tree(node->left), Calculate_Tree(node->right));
} 


//функция решает сложение и вычитание с 0. возвращает true, если были изменения.
static bool Zero_sumsub_optim(NODE* head)
{
    if (!head) return false;

    NODE* left = head->left;
    NODE* right = head->right;

    #define CODEGEN(side1, side2) if (side2->type == NUM_DATA && side2->data == 0)\
    {\
        Destroy_Tree(side2);\
        head->type = side1->type;\
        head->data = side1->data;\
        head->right = side1->right;\
        head->left = side1->left;\
        Destroy_Node(side1);\
        return true;\
    }

    CODEGEN(left, right)
    CODEGEN(right, left)

    #undef CODEGEN
    return false;
}


// функция решает когда 0 делится на что то. возвращает true, если были изменения.
static bool Zero_div_optim(NODE* head)
{
    if (!head) return false;

    if (head->left->type == NUM_DATA && head->left->data == 0)
    {
        Destroy_Tree(head->left);
        Destroy_Tree(head->right);

        head->type = NUM_DATA;
        head->data = 0;
        head->left = NULL;
        head->right = NULL;
        return true;
    }
    return false;
}


// функция решает когда 1 делит что то. возвращает true, если были изменения.
static bool One_div_optim(NODE* head)
{
    if (!head) return false;

    NODE* left = head->left;
    NODE* right = head->right;
    
    if (right->type == NUM_DATA && right->data == 1)
    {
        Destroy_Tree(right);

        head->type = head->left->type;
        head->data = head->left->data;

        head->right = head->left->right;
        head->left = head->left->left;

        Destroy_Node(left);
        return true;
    }    
    return false;
}


// функция решает умножения на нуль. возвращает true, если были изменения.
static bool Zero_mul_optim(NODE* head)
{
    if (!head) return false;

    if ((head->right->type == NUM_DATA && head->right->data == 0) || (head->left->type == NUM_DATA && head->left->data == 0))
    {
        Destroy_Tree(head->left);
        Destroy_Tree(head->right);
        
        head->type = NUM_DATA;
        head->data = 0;
        head->left = NULL;
        head->right = NULL;
        return true;
    }
    return false;

}

// функция решает умножения на 1. возвращает true, если были изменения.
static bool One_mul_optim(NODE* head)
{
    if (!head) return false;
 
    NODE* left = head->left;
    NODE* right = head->right;
    #define CODEGEN(side1, side2) if (side2->type == NUM_DATA && side2->data == 1)\
    {\
        Destroy_Tree(side2);\
        head->type = side1->type;\
        head->data = side1->data;\
        head->right = side1->right;\
        head->left = side1->left;\
        Destroy_Node(side1);\
        return true;\
    }

    CODEGEN(left, right)
    CODEGEN(right, left)

    #undef CODEGEN
    return false;
}
 
// функция решает возведение в степени 0,1. возвращает true, если были изменения.
static bool Zero_One_deg_optim(NODE* head)
{
    if (!head) return false;

    NODE* left = head->left;
    NODE* right = head->right;
    if (right->type == NUM_DATA)
    {
        if (right->data == 0)
        {
            Destroy_Tree(right);
            Destroy_Tree(left);

            head->type = NUM_DATA;
            head->data = 1;
            
            head->right = NULL;
            head->left = NULL;
            return true;
        }
        else if (head->right->data == 1)
            {
                Destroy_Tree(right);

                head->type = left->type;
                head->data = left->data;
                head->right = left->right;
                head->left = left->left;
                
                Destroy_Node(left);
                
                return true;
            }
    }
    
    return false;
}


// Функция вычисляет значения операций с числами.
static bool Nums_optim(NODE* head)
{
    // printf("DUMP!!-----------\nnode[%p]\n node->type=%d\nnode->left=%p\nnode->right=%p\n", head, head->type, head->left, head->right);
    // {printf("YAAAAA UMER!!\n"); Tree_Dump("smotrim.dot", head);scanf("%c");*/}
    if (!head || head->type == NUM_DATA || head->type == VAR_DATA) return false;
    if (!Is_Num(head)) return false;
    if (head->data == '+' || head->data == '-' || head->data == '*' ||head->data == '/') return false;
    
    head->data = Calculate_Tree(head);
    head->type = NUM_DATA;
    head->left = NULL;
    head->right = NULL; 
    //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
    return true;
            
}



// функция оптимизирует граф (умножение на 0, 1 сложение/вычитание с 0).
void Optimization(NODE* head, int* changes)
{
    if (!head) return;

     //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
    if (head->type == OP_DATA)
    {
        //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        if (Nums_optim(head)) {*changes += 1; /*printf("dumppnumber0"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");*/}
        else if (head->data == '*')
        {
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if      (One_mul_optim    (head)) {*changes += 1;/*printf("dumppnumber1\n"); Tree_Dump("smotrim.dot", global_tree_head); scanf("%c");*/}
            else if (Zero_mul_optim   (head)) {*changes += 1;/*printf("dumppnumber2\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");*/}
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        }
        else if (head->data == '/')
        {
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if      (Zero_div_optim (head)) {*changes += 1;/*printf("dumppnumber3\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");*/}
            else if (One_div_optim  (head))  {*changes += 1;/*printf("dumppnumber4\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");*/}
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        }
        else if (head->data == '+' || head->data == '-')
        {   
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if (Zero_sumsub_optim    (head)) {*changes += 1;/*printf("dumppnumber5\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");*/}
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        }
        else if (head->data == '^')
        {
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if (Zero_One_deg_optim  (head)) {*changes += 1;/*printf("dumppnumber6\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");*/}
             //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        }
    }
    Optimization(head->right, changes);
    Optimization(head->left, changes);
}