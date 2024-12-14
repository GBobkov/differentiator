#include "tree_for_diff.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BUFSIZE 64

static const char* database_file_name = "database.txt";


// Создать узел с массивом data.
NODE* Create_Node(DATA_TYPE_CNST var_type, int data,  NODE* left, NODE* right)
{

    NODE* node = (NODE *) calloc(1, sizeof(NODE));
    node->type = var_type;
    node->data = data;
    node->left = left;
    node->right = right;

    return node;
}

// копирует узел с поддеревьями
NODE* Copy_Node(NODE* node)
{   
    if (!node) return NULL;
    return Create_Node(node->type, node->data, Copy_Node(node->left), Copy_Node(node->right));
}

// удалить конкретный узел
void Destroy_Node(NODE* node)
{
    assert(node != NULL);
    free(node); 
}

// рекурсивно удаляет дерево
void Destroy_Tree(NODE* head)
{
    if (!head) return;
    
    if (head->right) Destroy_Tree(head->right);
    if (head->left) Destroy_Tree(head->left);
    Destroy_Node(head);
}



// Убирает \n в конце строки
static void remove_newline(char *str) {
    char *pos;
    // Находим символ новой строки в строке
    if ((pos = strchr(str, '\n')) != NULL) {
        *pos = '\0'; // Заменяем символ новой строки на окончание строки
    }
}


// Считать новую строку с консоли
static void Get_New_Line(FILE* file, char *answer)
{   
    
    fgets(answer, BUFSIZE, file);
    
    remove_newline(answer);
    
}


// Строка - число ?
static bool Isnum(const char *str)
{
    while (*str != '\0')
    {
        if (str[0] < '0' || str[0] > '9') return false;
        str++;
    }
    return true;
}


// Строка - знак операции ?
static bool Isop(const char *str)
{
    if (*(str + 1) != '\0') return false;

    switch (*str)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case 'l':
    case 'c':
    case 's': 
        return true;
    
    default:
        return false;
    }
}

// Строка - имя переменной ?
static bool Isvar(const char *str)
{
    if (*(str + 1) != '\0') return false;
    return isalpha(*str);
}

// Обработает новую строку и запишет в ноду тип и data.
static void Handle_New_Line(FILE* file, NODE* node)
{
    assert(node);
    
    char line[BUFSIZE] = {};
    
    Get_New_Line(file, line);
    
    // TODO: Возмжно кодген или массив функций
    if (Isnum(line))
    {
        node->type = NUM_DATA;
        node->data = atoi(line);
    }
    else if (Isop(line))
    {
        node->type = OP_DATA;
        node->data = line[0];
    }
    else if (Isvar(line))
    {
        node->type = VAR_DATA;
        node->data = line[0];
    }
    else if (line[0] == '\0')
    {
        node->type = NONE_DATA;
        node->data = '\0';
    }
    else
    {
        printf("Unpredictable data!\ndata=\"%s\"\n", line);
        abort();
    }
    
}

// Считать очередной узел из файла
static void Read_New_Node(FILE* file, NODE* node)
{
     
    Handle_New_Line(file, node);
    char bracket = '\0';
    bracket = fgetc(file); // Достаём скобку.
    fgetc(file);    // Достаём \n.
    
    if (bracket == '{')
    {
        NODE* right_son = Create_Node(NONE_DATA, 0, NULL, NULL);
        node->right = right_son;
        Read_New_Node(file, right_son);
    }
    
    if (bracket == '{')
    {
        bracket = fgetc(file);  // Достаём скобку.
        fgetc(file); // Достаём \n.
    }
    
    if (bracket == '{')
    {
        NODE* left_son = Create_Node(NONE_DATA, 0, NULL, NULL);
        node->left = left_son;
        Read_New_Node(file, left_son);
        fgetc(file);    // Достаём скобку ('}').
        fgetc(file);    // Достаём \n.
    }

}




static char user_line_expression[100] = "x^sx";
static int user_line_pointer = 0;


static void SyntaxErr(int line, const char* message)
{
    printf("Unpredictable symbol at position %d\n", line);
    printf("symbol=%c\n", user_line_expression[user_line_pointer]);
    printf("message:%s\n", message);
    abort();
}

// Обработать сумму.
NODE* Get_Sumsub();

// Обработать число.
static NODE* Get_Number()
{
    printf("(%s)\tenter\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    int val = 0;
    int oldp = user_line_pointer;
    while (user_line_expression[user_line_pointer] >= '0' && user_line_expression[user_line_pointer] <= '9')
    {
        val = val * 10 + user_line_expression[user_line_pointer] - '0';
        user_line_pointer++;
    }
    
    if (user_line_pointer == oldp)
        SyntaxErr(user_line_pointer, "user_line_pointer == oldp");
    printf("(%s)\texit\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    return Create_Node(NUM_DATA, val, NULL, NULL);
}

// Обработать "ядро" - выражение в скобках, или переменная, или число.
static NODE* Get_Kernel()
{

    printf("(%s)\tenter\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    if (user_line_expression[user_line_pointer] == '(')
    {
        user_line_pointer++;
        NODE* tree = Get_Sumsub();
        if (user_line_expression[user_line_pointer] != ')')
            SyntaxErr(user_line_pointer, "user_line_expression[user_line_pointer] != ')'");
        user_line_pointer++;
        printf("(%s)\texit\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
        return tree;
    }
    else
    {
        if (user_line_expression[user_line_pointer] == 'x') 
        {
            user_line_pointer++;
            printf("(%s)\texit\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
            return Create_Node(VAR_DATA, 'x', NULL, NULL);
        }
        else 
            {
                
                return Get_Number();
            }
    }
}
 
// обработать функцию.
static NODE* Get_Func()
{
   printf("(%s)\tenter\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    NODE* op_tree = NULL;
    if (user_line_expression[user_line_pointer] == 'c' || user_line_expression[user_line_pointer] == 's' || user_line_expression[user_line_pointer] == 'l')
    {
        char sign = user_line_expression[user_line_pointer];
        user_line_pointer++;
        op_tree = Create_Node(OP_DATA, sign, Create_Node(NUM_DATA, 0, NULL, NULL), Get_Func());
    }
    NODE* right_tree = Get_Kernel();
   
    printf("(%s)\texit\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    if (op_tree) return op_tree;
    return right_tree;
}

// обработать возведение в степень.
static NODE* Get_Degree()
{
   printf("(%s)\tenter\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Func();
    if (user_line_expression[user_line_pointer] == '^')
    {
        char sign = user_line_expression[user_line_pointer];
        user_line_pointer++;
        op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Degree());
    }
   
    printf("(%s)\texit\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать умножение.
static NODE* Get_Muldiv()
{
   printf("(%s)\tenter\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Degree();
    if (user_line_expression[user_line_pointer] == '*' || user_line_expression[user_line_pointer] == '/')
        {
            char sign = user_line_expression[user_line_pointer];
            user_line_pointer++;
            op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Muldiv());
        }
   printf("(%s)\texit\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    if (op_tree) return op_tree;
    return left_tree;
}

// Обработать сумму.
NODE* Get_Sumsub()
{
    printf("(%s)\tenter\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Muldiv();
    if (user_line_expression[user_line_pointer] == '+' || user_line_expression[user_line_pointer] == '-')
    {
        char sign = user_line_expression[user_line_pointer];
        user_line_pointer++;
        op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Sumsub());
    }
    printf("(%s)\texit\tptr=%d.symbol=%c\n", __FUNCTION__, user_line_pointer, user_line_expression[user_line_pointer]);
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать дерево 
static NODE* Get_Expression_Tree()
{
    NODE* tree = Get_Sumsub();
    if (user_line_expression[user_line_pointer] != '\0')
        SyntaxErr(user_line_pointer, "user_line_expression[user_line_pointer] != '\\0'");

    user_line_pointer++;
    
    return tree;
}


// Считать данные с файла и создать дерево.
static int Read_Data(NODE* node)
{
    FILE* file = fopen(database_file_name, "r");

    assert(node != NULL);
    assert(file != NULL);
    
    fgetc(file);    // Убираем первую '{'
    fgetc(file);

    Read_New_Node(file, node);
    
    fclose(file);

    return 0;
}


// работа с пользователем (читать из базы данных или из ввода)
void Handle_Read_Request(NODE* head)
{
    printf("Choose how to get input data:\n[1] read database_file_name=%s\n[2] read expression from consol.\n", database_file_name);
    char answer = '\0';
    while (answer != '1' && answer != '2')
    {
        printf("Input:");
        scanf("%c", &answer);
        getchar(); // Достать '\n'
    }

    if (answer == '1') Read_Data(head);
    else 
    {
        printf("Enter expression:%s\n", user_line_expression);
        //scanf("%s", user_line_expression);
        NODE* global_tree = Get_Expression_Tree();

        head->type = global_tree->type;
        head->data = global_tree->data;
        head->left = global_tree->left;
        head->right = global_tree->right;
    }
}

// Рекурсивно записываем дерево
static void Write_New_Node(FILE* file, NODE* node)
{
    fprintf(file, "{\n");
    if (node->type == OP_DATA || node->type == VAR_DATA)
        fprintf(file,"%c\n", node->data);
    else if (node->type == NUM_DATA)
        fprintf(file,"%d\n", node->data);
    if (node->right) Write_New_Node(file, node->right);
    if (node->left) Write_New_Node(file, node->left);
    fprintf(file, "}\n");
}   


// Сохранить данные
int Write_Data(NODE* node)
{
    FILE* file = fopen(database_file_name, "w");
    Write_New_Node(file, node);
    fclose(file);
    return 0;
}

