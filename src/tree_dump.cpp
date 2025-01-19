#include "tree_dump.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define CMD_SIZE 30



#define CODEGEN(name)\
if (op == OP_##name) return #name;

// Функция возвращает char операцию.
static const char * What_Oper(NODE* node)
{
    if (node->type != OP_DATA)
    {
        printf("not op_data. %s:%d\n", __FILE__, __LINE__);
        abort();
    }
    int op = node->data;
    #include "func_codegen.h"
    if (op == OP_SUM) return "+";
    if (op == OP_SUB) return "-";
    if (op == OP_MUL) return "*";
    if (op == OP_DIV) return "/";
    if (op == OP_DEG) return "^";
    return "error";
}
#undef CODEGEN

// инициализация узлов в dot-file
static void Nodes_Init_2Dump(FILE* dump_file, NODE* node)
{
    assert(dump_file != NULL);
    assert(node != NULL);

    #define TITLE_COLOR "\"lightblue\""
    if (node->type == VAR_DATA)
        fprintf (dump_file, "NODE_0x%p[label = \"%c\",  fillcolor = " TITLE_COLOR "];\n", node, node->data);
    else if (node->type == NUM_DATA)
        fprintf (dump_file, "NODE_0x%p[label = \"%d\",  fillcolor = " TITLE_COLOR "];\n", node, node->data);
    else if (node->type == OP_DATA)
        fprintf (dump_file, "NODE_0x%p[label = \"%s\",  fillcolor = " TITLE_COLOR "];\n", node, What_Oper(node));
    else
    {
        printf("Unpredictable node->type=%d\n", node->type);
        printf("Dump_Node[%p]\n", node);
        printf("node->data=%d\nnode->left=%p\nnode->right=%p\n", node->data, node->left, node->right);
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
    fprintf (dump_file, "rankdir=TB;\n");
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

    char png_fname[CMD_SIZE] = {};
    if (strlen(dump_fname) > CMD_SIZE - strlen(".png"))
    {
        printf("Very big dumpfname. Can't dump to %s\n", dump_fname);
        return;
    }
    strcpy(png_fname, dump_fname);
    char *dot_ptr = strchr(png_fname, '.');
    if (!dot_ptr)
    {
        printf("Can't dump. Incorrect dump_fname=%s\n", dump_fname);
        return;
    }
    strcpy(dot_ptr, ".png");

    char create_png_cmd[CMD_SIZE * 3] = {}; 
    sprintf(create_png_cmd, "dot %s -Tpng -o %s", dump_fname, png_fname); 
    system(create_png_cmd);
}