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
    if (degree % 2 == 0) return pow(num * num, degree / 2);
    return num * pow(num, degree - 1);

}


// Вычислить значение операции.
static int result_value(int operation, int ld, int rd)
{
    switch (operation)
    {
    case OP_MUL:
        return ld * rd;
        break;
    case OP_DIV:
        if (rd == 0) 
            printf("ALARM DIV BY ZERO!\n");
        return ld / rd;
        break;
    case OP_SUM:
        return ld + rd;
        break;
    case OP_SUB:
        return ld - rd;
        break;
    case OP_DEG:
        return pow(ld, rd);
        break;

    default:
        printf("ALARM! DANGEROUS SITUATION! Idk what the operation=%d\n", operation);
        break;
    } 
    
    return 0;
}


//функция вычисляет численное значение дерева.
int Calculate_Tree(NODE* node, bool* isnum)
{
    if (node->type == NUM_DATA) return node->data;
    if (!(node->data & BIT_ISCALC_FUNC) || node->type == VAR_DATA) 
    {
        *isnum = false;
        return 1;
    }
    
    if (node->left->type == NUM_DATA && node->right->type == NUM_DATA) return result_value(node->data, node->left->data, node->right->data);
    
    return result_value(node->data, Calculate_Tree(node->left, isnum), Calculate_Tree(node->right, isnum));
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

        head->type = left->type;
        head->data = left->data;

        head->right = left->right;
        head->left = left->left;

        Destroy_Tree(left);
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
    #define CODEGEN(side1, side2)\
    if (side2->type == NUM_DATA && side2->data == 1)\
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
        else if (right->data == 1)
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
    if (!head || head->type == NUM_DATA || head->type == VAR_DATA) return false;
    if (!Is_Num(head)) return false;
    if (head->data & 1) return false;
    bool isnum = true;
    int value = Calculate_Tree(head, &isnum);
    if (isnum)
    {
        head->data = value;
        head->type = NUM_DATA;

        Destroy_Tree(head->left);
        Destroy_Tree(head->right);
        head->left = NULL;
        head->right = NULL;
    }

    return isnum;        
}
  


// функция оптимизирует граф (умножение на 0, 1 сложение/вычитание с 0).
void Optimization_Step(NODE* head, int* changes)
{
    if (!head) return;

    if (head->type == OP_DATA)
    {
        if          (Nums_optim(head))      {*changes += 1;}
        else if     (head->data == OP_MUL)
        {
             
            if      (One_mul_optim  (head)) {*changes += 1;}
            else if (Zero_mul_optim (head)) {*changes += 1;}
             
        }
        else if (head->data == OP_DIV)
        {
             
            if      (Zero_div_optim (head)) {*changes += 1;}
            else if (One_div_optim  (head))  {*changes += 1;}
             
        }
        else if (head->data == OP_SUM || head->data == OP_SUB)
        {   
             
            if (Zero_sumsub_optim    (head)) {*changes += 1;}
             
        }
        else if (head->data == OP_DEG)
        {
             
            if (Zero_One_deg_optim  (head)) {*changes += 1;}
             
        }
    }
    Optimization_Step(head->right, changes);
    Optimization_Step(head->left, changes);
}


void Optimizator(NODE* head)
{
    int changes = 0;
    do
    {
        changes = 0;
        Optimization_Step(head, &changes);
    } while (changes > 0);
    
}