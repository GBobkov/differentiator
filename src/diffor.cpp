#include "diffor.h"
#include "tree_for_diff.h"

#include <stdio.h>
#include <assert.h>

static NODE* global_tree_head = NULL;
static int global_counter = 1;
// копирует узел
static NODE* Copy_Node(NODE* node)
{   
    if (!node) return NULL;
    return Create_Node(node->type, node->data, Copy_Node(node->left), Copy_Node(node->right));
}


// проверка на то, что дерево - число. true если число, false если функция.
static bool Is_Num(NODE* head)
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
static int Calculate_Tree(NODE* node)
{
    if (node->type == NUM_DATA) return node->data;
    if (node->left->type == NUM_DATA && node->right->type == NUM_DATA) return result_value(node->data, node->left->data, node->right->data);

    return result_value(node->data, Calculate_Tree(node->left), Calculate_Tree(node->right));
} 


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

//функция решает сложение и вычитание с 0. возвращает true, если были изменения.
static bool Zero_sumsub_optim(NODE* head)
{
    if (!head) return false;

    if (head->right->type == NUM_DATA && head->right->data == 0)
    {
        // TODO: внимательно free(). 
        
        head->type = head->left->type;
        head->data = head->left->data;
        
        
        head->right = Copy_Node(head->left->right);
        head->left = Copy_Node(head->left->left);
            
        // Destroy_Tree(head->left->right);
        // Destroy_Tree(head->left->left);
        return true;
    }
    else if (head->left->type == NUM_DATA && head->left->data == 0)
    {
        
        head->type = head->right->type;
        head->data = head->right->data;
        
        head->left = Copy_Node(head->right->left);
        head->right = Copy_Node(head->right->right);
        // Destroy_Tree(head->right->right);
        // Destroy_Tree(head->right->left);
        return true;
    }
    return false;
}


// функция решает когда 0 делится на что то. возвращает true, если были изменения.
static bool Zero_div_optim(NODE* head)
{
    if (!head) return false;

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
    if (!head) return false;

    if (head->right->type == NUM_DATA && head->right->data == 1)
    {
        
        head->type = head->left->type;
        head->data = head->left->data;

        head->right = Copy_Node(head->left->right);
        head->left = Copy_Node(head->left->left);
        // Destroy_Tree(head->left->right);
        // Destroy_Tree(head->left->left);
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
    if (!head) return false;

    //TODO: Destroy ненужной памяти.
    if ((head->right->type == NUM_DATA) && (head->right->data == 1))
    {

        head->type = head->left->type;
        head->data = head->left->data;

        head->right = Copy_Node(head->left->right);
        head->left = Copy_Node(head->left->left);
        return true;
    }
    if ((head->left->type == NUM_DATA) && (head->left->data == 1))
    {

        head->type = head->right->type;
        head->data = head->right->data;

        head->left = Copy_Node(head->right->left);
        head->right = Copy_Node(head->right->right);
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
                
                //Destroy_Tree(head->right);
                NODE* lft_nd = head->left;
                
                head->right = head->left->right;
                head->left = head->left->left;
                
                //Destroy_Node(lft_nd);
                
                return true;
            }
    }
    
    return false;
}


// Функция вычисляет значения операций с числами.
static bool Nums_op(NODE* head)
{
    printf("DUMP!!-----------\nnode[%p]\n node->type=%d\nnode->left=%p\nnode->right=%p\n", head, head->type, head->left, head->right);
    {printf("YAAAAA UMER!!\n"); Tree_Dump("smotrim.dot", head);scanf("%c");}
    if (!head || head->type == NUM_DATA || head->type == VAR_DATA) return false;
    if (head->left->type != NUM_DATA || head->right->type != NUM_DATA) return false;

    head->type = NUM_DATA;
    printf("HOH!\n");
    head->data = result_value(head->data, head->left->data, head->right->data);
    //Destroy_Tree(head->left);
    //Destroy_Tree(head->right);
    head->left = NULL;
    head->right = NULL; 
    printf("HOH!\n");
    return true;
            
}



// функция оптимизирует граф (умножение на 0, 1 сложение/вычитание с 0). // FIXME: Не работает оптимизатор!!
static void Optimization(NODE* head, int* changes)
{
    if (!head) return;

    printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
    if (head->type == OP_DATA)
    {
        printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        if (Nums_op(head)) {*changes += 1; printf("dumppnumber0\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
        else if (head->data == '*')
        {
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if      (One_mul_optim    (head)) {*changes += 1;printf("dumppnumber1\n"); Tree_Dump("smotrim.dot", global_tree_head); scanf("%c");}
            else if (Zero_mul_optim   (head)) {*changes += 1;printf("dumppnumber2\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        }
        else if (head->data == '/')
        {
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if      (Zero_div_optim (head)) {*changes += 1;printf("dumppnumber3\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            else if (One_div_optim  (head))  {*changes += 1;printf("dumppnumber4\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        }
        else if (head->data == '+' || head->data == '-')
        {   
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if (Zero_sumsub_optim    (head)) {*changes += 1;printf("dumppnumber5\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
        }
        else if (head->data == '^')
        {
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
            if (Zero_One_deg_optim  (head)) {*changes += 1;printf("dumppnumber6\n"); Tree_Dump("smotrim.dot", global_tree_head);scanf("%c");}
            printf("%d\n", __LINE__); //printf("newstep %s:%d(%s)\n", __FILE__, __LINE__, __FUNCTION__); Tree_Dump("smotrim.dot", head); scanf("%c");
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
    global_tree_head = head;
    Tree_Dump("dump_start.dot", head);
    head = New_Step_Drvt(head);
    Tree_Dump("dump_diritive.dot", head);
    int changes = 0;
    do
    {
        printf("I'm rfd!\n");
        changes = 0;
        Optimization(head, &changes);
    } while (changes > 0);
    printf("I DID IT !!!!");
    //Write_Data(head);
    Tree_Dump("dump2.dot", head);
}
