#include "diffor.h"
#include "tree_for_diff.h"

#include <stdio.h>
#include <assert.h>

static NODE* global_tree_head = NULL;


static NODE* Copy_Node(NODE* node)
{   
    if (!node) return NULL;
    return Create_Node(node->type, node->data, Copy_Node(node->left), Copy_Node(node->right));
}




// Рекурсивная функция вычисления производной
static NODE* New_Step_Drvt(NODE* node)
{

    if (node->type == NUM_DATA)
    {
        node->data = 0;
        return Create_Node(NUM_DATA, 0, NULL, NULL);
    }
    if (node->type == VAR_DATA)
    {
        node->type = NUM_DATA;
        node->data = 1;
        return Create_Node(NUM_DATA, 1, NULL, NULL);
    }
    if (node->type == OP_DATA)
    {
        
        if (node->data == '+' || node->data == '-')
        {
            node->left = New_Step_Drvt(node->left);
            node->right = New_Step_Drvt(node->right);
            return node;
        }
        if (node->data == '*')
        {
            node->data = '+';
            NODE* left = node->left;
            NODE* right = node->right;
            node->left = Create_Node(OP_DATA, '*', New_Step_Drvt(Copy_Node(left)), Copy_Node(right));
            node->right = Create_Node(OP_DATA, '*', Copy_Node(left), New_Step_Drvt(Copy_Node(right)));
            return Copy_Node(node);
        }
        if (node->data == '^')
        {
            if (node->left->type == NUM_DATA)
            {
                // константа
                if (node->right->type == NUM_DATA)
                {
                    node->type = NUM_DATA;
                    node->data = 0;
                    Destroy_Tree(node->left);
                    Destroy_Tree(node->right);
                    node->left = NULL;
                    node->right = NULL;
                    return Copy_Node(node);
                }
                else // показательная функция
                {
                    node->left = Create_Node(OP_DATA, '*', Create_Node(OP_DATA, 'l', NULL, Create_Node(NUM_DATA, node->left->data, NULL, NULL)), Copy_Node(node));
                    node->right = New_Step_Drvt(node->right);
                    node->data = '*';
                    return Copy_Node(node);
                }
                
            }
            else if (node->left->type == VAR_DATA && node->right->type == NUM_DATA) // степенная функция
            {
                node->data = '*';
                Destroy_Tree(node->left);
                node->left = Create_Node(NUM_DATA, node->right->data, NULL, NULL);
                Destroy_Tree(node->right);
                node->right = Create_Node(OP_DATA, '^', Create_Node(VAR_DATA, 'x', NULL, NULL), Create_Node(NUM_DATA, node->left->data - 1, NULL, NULL));
                return Copy_Node(node);
            }
        }
        
    }
    return NULL;
}

//функция решает сложение и вычитание с 0. возвращает true, если были изменения.
static bool Zero_sumsub_optim(NODE* head)
{
    if (head->right->type == NUM_DATA && head->right->data == 0)
    {
        // TODO: внимательно free(). 
        
        head->type = head->left->type;
        head->data = head->left->data;

        head->right = Copy_Node(head->left->right);
        head->left = Copy_Node(head->left->left);
        Destroy_Tree(head->left->right);
        Destroy_Tree(head->left->left);
        return true;
    }
    else if (head->left->type == NUM_DATA && head->left->data == 0)
    {
        
        head->type = head->right->type;
        head->data = head->right->data;
        
        head->left = Copy_Node(head->right->left);
        head->right = Copy_Node(head->right->right);
        Destroy_Tree(head->right->right);
        Destroy_Tree(head->right->left);
        return true;
    }
    return false;
}


// функция решает когда 0 делится на что то. возвращает true, если были изменения.
static bool Zero_div_optim(NODE* head)
{
    if (head->left->type == NUM_DATA && head->left->data == 0)
    {
        
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
    if (head->right->type == NUM_DATA && head->right->data == 1)
    {
        
        head->type = head->left->type;
        head->data = head->left->data;

        head->right = Copy_Node(head->left->right);
        head->left = Copy_Node(head->left->left);
        Destroy_Tree(head->left->right);
        Destroy_Tree(head->left->left);
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
        
        head->type = NUM_DATA;
        head->data = 0;
        Destroy_Tree(head->left);
        Destroy_Tree(head->right);
        head->left = NULL;
        head->right = NULL;
        return true;
    }
    return false;

}

// функция решает умножения на 1. возвращает true, если были изменения.
static bool One_mul_optim(NODE* head)
{
    if ((head->right->type == NUM_DATA) && (head->right->data == 1))
    {

        head->type = head->left->type;
        head->data = head->left->data;

        head->right = Copy_Node(head->left->right);
        head->left = Copy_Node(head->left->left);
        Destroy_Tree(head->left->right);
        Destroy_Tree(head->left->left);
        return true;
    }
    if ((head->left->type == NUM_DATA) && (head->left->data == 1))
    {

        head->type = head->right->type;
        head->data = head->right->data;

        head->left = Copy_Node(head->right->left);
        head->right = Copy_Node(head->right->right);
        Destroy_Tree(head->right->right);
        Destroy_Tree(head->right->left);
        return true;
    }
    return false;
}
 
// функция решает возведение в степени 0,1. возвращает true, если были изменения.
static bool Zero_One_deg_optim(NODE* head)
{
    if (!head) return false;

    if (head->right->type == NUM_DATA)
    {
        if (head->right->data == 0)
        {
            head->type = NUM_DATA;
            head->data = 1;
            // Destroy_Tree(head->right); // TODO: free (Destroy node)сигфолдится хз почему
            // Destroy_Tree(head->left);
            
            head->right = NULL;
            head->left = NULL;
            return true;
        }
        else if (head->right->data == 1)
            {
                
                head->type = head->left->type;
                head->data = head->left->data;
                
                Destroy_Tree(head->right);
                NODE* lft_nd = head->left;
                
                head->right = head->left->right;
                head->left = head->left->left;
                
                Destroy_Node(lft_nd);
                
                return true;
            }
    }
    
    return false;
}

// возводит в степень число
static int pow(int num, int degree)
{
    if (degree == 1) return num;
    return num * pow(num, degree - 1);

}


static int result_value(char operation, int ld, int rd)
{
    if (operation == '*') return ld * rd;
    if (operation == '/') {if (rd == 0) {printf("ALARM DIV BY ZERO!\n");} return ld / rd;}
    if (operation == '+') return ld + rd;
    if (operation == '^') return pow(ld, rd);
    printf("ALARM! DANGEROUS SITUATION!\n");
    return 0;
}

// Функция вычисляет значения операций с числами.
static bool Nums_op(NODE* head)
{
    if (head->left->type != NUM_DATA || head->right->type != NUM_DATA) return false;

    head->type = NUM_DATA;
    head->data = result_value(head->data, head->left->data,  head->right->data );
    Destroy_Tree(head->left);
    Destroy_Tree(head->right);
    head->left = NULL;
    head->right = NULL; 
    
    return true;
            
}



// функция оптимизирует граф (умножение на 0, 1 сложение/вычитание с 0). // FIXME: Не работает оптимизатор!!
static void Optimization(NODE* head, int* changes)
{
    if (!head) return;

    if (head->type == OP_DATA)
    {
        if (Nums_op(head)) {*changes += 1; printf("dumppnumber0"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            
        if (head->data == '*')
        {
            if      (One_mul_optim    (head)) {*changes += 1;printf("dumppnumber1"); Tree_Dump("smotrim.dot", global_tree_head); scanf("%c");}
            else if (Zero_mul_optim   (head)) {*changes += 1;printf("dumppnumber2"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            
        }
        else if (head->data == '\\')
        {
            if      (Zero_div_optim (head)) {*changes += 1;printf("dumppnumber3"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            else if (One_div_optim  (head))  {*changes += 1;printf("dumppnumber4"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
        }
        else if (head->data == '+' || head->data == '-')
        {   
            if (Zero_sumsub_optim    (head)) {*changes += 1;printf("dumppnumber5"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
        }
        else if (head->data == '^')
        {
            
            if (Zero_One_deg_optim  (head)) {*changes += 1;printf("dumppnumber6"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
        }
    }
    Optimization(head->right, changes);
    Optimization(head->left, changes);
}


// Функция вычисляет произвоную.
void Calculate_Derivative(void)
{
    NODE* head = Create_Node(NONE_DATA, '\0', NULL, NULL);
    Read_Data(head);
    Tree_Dump("dump_start.dot", head);
    head = New_Step_Drvt(head);
    Tree_Dump("dump_diritive.dot", head);
    int changes = 0;
    global_tree_head = head;
    do
    {
        changes = 0;
        Optimization(head, &changes);
    } while (changes > 0);
    printf("I DID IT !!!!");
    //Write_Data(head);
    Tree_Dump("dump2.dot", head);
}
