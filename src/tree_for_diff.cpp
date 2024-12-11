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


// инициализация узлов в dot-file
static void Nodes_Init_2Dump(FILE* dump_file, NODE* node)
{
    assert(dump_file != NULL);
    assert(node != NULL);

    #define TITLE_COLOR "\"lightblue\""
    if (node->type == OP_DATA || node->type == VAR_DATA)
        fprintf (dump_file, "NODE_0x%p[label = \"%c\", fillcolor =" TITLE_COLOR "];\n", node, node->data, node->type);
    else if (node->type == NUM_DATA)
        fprintf (dump_file, "NODE_0x%p[label = \"%d\", fillcolor =" TITLE_COLOR "];\n", node, node->data);
    else
    {
        printf("Error at %s:%d(%s)", __FILE__, __LINE__, __FUNCTION__);
        abort();
    }
    
    if (node->left) Nodes_Init_2Dump(dump_file, node->left);
    if (node->right) Nodes_Init_2Dump(dump_file, node->right);
    #undef TITLE_COLOR
}


// соеденить стрелками элементы дерева
static void Write_Connections_2Dump(FILE* dump_file, NODE* node)
{
    assert(dump_file != NULL);
    assert(node != NULL);

    if (node->left)
    {
        fprintf(dump_file, "NODE_0x%p->NODE_0x%p [weight = 0, color = deeppink]\n", node, node->left);
        Write_Connections_2Dump(dump_file, node->left);
    }
    if (node->right)
    {
        fprintf(dump_file, "NODE_0x%p->NODE_0x%p [weight = 0, color = deeppink]\n", node, node->right);
        Write_Connections_2Dump(dump_file, node->right);
    }
}



// Сформировать dot-file и png.
void Tree_Dump(const char* dump_fname, NODE* node)
{

    FILE* dump_file = fopen(dump_fname, "w");

    assert(dump_file != NULL);
    assert(node != NULL);

    #define FREE_COLOR  "\"lightgreen\""
    #define BUSY_COLOR  "\"coral\""


    fprintf (dump_file, "digraph G\n");
    fprintf (dump_file, "{\n");
    fprintf (dump_file, "splines=line;\n");
    fprintf (dump_file, "nodesep=2  ;\n"); // расстояние между ячейками
    fprintf (dump_file, "node[shape=\"oval\", style=\"rounded, filled\"];\n\n");

    Nodes_Init_2Dump(dump_file, node);
    fprintf (dump_file, "\n");

    Write_Connections_2Dump(dump_file, node);
    fprintf (dump_file, "\n");

    fprintf (dump_file, "}\n");

    fclose(dump_file);
    #undef FREE_COLOR
    #undef BUSY_COLOR

    char create_png_cmd[BUFSIZE] = {};
    char png_fname[BUFSIZE] = {};
    strcpy(png_fname, dump_fname);
    char *dot_ptr = strchr(png_fname, '.');
    strcpy(dot_ptr, ".png");

    sprintf(create_png_cmd, "dot %s -Tpng -o %s", dump_fname, png_fname); 
    //printf("cmd=%s\n", create_png_cmd);
    system(create_png_cmd);
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
    case '&': // знак композиции
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


// Считать данные с файла и создать дерево.
int Read_Data(NODE* node)
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


// Рекурсивно записываем дерево
static void 
Write_New_Node(FILE* file, NODE* node)
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