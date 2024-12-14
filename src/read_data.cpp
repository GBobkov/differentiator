#include "read_data.h"

#include "tree_for_diff.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>


#define BUFSIZE 64

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




static char user_line_expression[100] = "3*x^(x^2+2)/l(s(x)+5)-1";
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
    
    int val = 0;
    int oldp = user_line_pointer;
    while (user_line_expression[user_line_pointer] >= '0' && user_line_expression[user_line_pointer] <= '9')
    {
        val = val * 10 + user_line_expression[user_line_pointer] - '0';
        user_line_pointer++;
    }
    
    if (user_line_pointer == oldp)
        SyntaxErr(user_line_pointer, "user_line_pointer == oldp");
    
    return Create_Node(NUM_DATA, val, NULL, NULL);
}

// Обработать "ядро" - выражение в скобках, или переменная, или число.
static NODE* Get_Kernel()
{

    
    if (user_line_expression[user_line_pointer] == '(')
    {
        user_line_pointer++;
        NODE* tree = Get_Sumsub();
        if (user_line_expression[user_line_pointer] != ')')
            SyntaxErr(user_line_pointer, "user_line_expression[user_line_pointer] != ')'");
        user_line_pointer++;
        
        return tree;
    }
    else
    {
        if (user_line_expression[user_line_pointer] == 'x') 
        {
            user_line_pointer++;
            
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

    if (user_line_expression[user_line_pointer] == 'c' || user_line_expression[user_line_pointer] == 's' || user_line_expression[user_line_pointer] == 'l')
    {
        char sign = user_line_expression[user_line_pointer];
        user_line_pointer++;
        return Create_Node(OP_DATA, sign, Create_Node(NUM_DATA, 0, NULL, NULL), Get_Func());
    }
    else return Get_Kernel();
   
}

// обработать возведение в степень.
static NODE* Get_Degree()
{
   
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Func();
    if (user_line_expression[user_line_pointer] == '^')
    {
        char sign = user_line_expression[user_line_pointer];
        user_line_pointer++;
        op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Degree());
    }
   
    
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать умножение.
static NODE* Get_Muldiv()
{
   
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Degree();
    if (user_line_expression[user_line_pointer] == '*' || user_line_expression[user_line_pointer] == '/')
        {
            char sign = user_line_expression[user_line_pointer];
            user_line_pointer++;
            op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Muldiv());
        }
   
    if (op_tree) return op_tree;
    return left_tree;
}

// Обработать сумму.
NODE* Get_Sumsub()
{
    
    NODE* op_tree = NULL;
    NODE* left_tree = Get_Muldiv();
    if (user_line_expression[user_line_pointer] == '+' || user_line_expression[user_line_pointer] == '-')
    {
        char sign = user_line_expression[user_line_pointer];
        user_line_pointer++;
        op_tree = Create_Node(OP_DATA, sign, left_tree, Get_Sumsub());
    }
    
    if (op_tree) return op_tree;
    return left_tree;
}

// обработать дерево 
static NODE* Get_Expression_Tree(void)
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
    char answer[10] = {};
    while (answer[0] != '1' && answer[0] != '2')
    {
        printf("Input:");
        scanf("%c", answer);
        getchar(); // Достать '\n'
    }

    if (answer[0] == '1') Read_Data(head);
    else 
    {
        printf("Enter expression:");
        scanf("%s", user_line_expression);
        NODE* global_tree = Get_Expression_Tree();

        head->type = global_tree->type;
        head->data = global_tree->data;
        head->left = global_tree->left;
        head->right = global_tree->right;
    }
}