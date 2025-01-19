#include "write_data.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

static const char *_latex_fname = "latex/diritivate.tex";
static FILE* latex_ptr = NULL;


static char *String2Lower(char* str)
{
    int ptr = 0;
    while (str[ptr] != '\0')
    {
        str[ptr] = (char) tolower(str[ptr]);
        ptr++;
    }
    return str;
}


// Рекурсивно записываем дерево
static void Write_New_Node(FILE* file, NODE* node)
{
    assert(file);
    assert(node);

    fprintf(file, "{\n");
    if (node->type == OP_DATA || node->type == VAR_DATA)
        fprintf(file,"%c\n", node->data);
    else if (node->type == NUM_DATA)
        fprintf(file,"%d\n", node->data);
    if (node->right) Write_New_Node(file, node->right);
    if (node->left) Write_New_Node(file, node->left);
    fprintf(file, "}\n");
}


// начало файла
static int Begin_TexDump(FILE* tex_dump)
{
    assert(tex_dump);

    if (tex_dump == NULL)
    {
        printf("No such file\n");
        return FILE_NOT_OPEN;
    }
    fprintf(tex_dump, "\\documentclass{article}\n");
    fprintf(tex_dump, "\\usepackage{ucs}\n");
    fprintf(tex_dump, "\\usepackage[utf8x]{inputenc}\n");
    fprintf(tex_dump, "\\usepackage[russian]{babel}\n");
    fprintf(tex_dump, "\\usepackage{microtype}\n");
    fprintf(tex_dump, "\\usepackage{amsmath}\n");
    fprintf(tex_dump, "\\usepackage{ragged2e}\n");
    fprintf(tex_dump, "\\usepackage[l2tabu,orthodox]{nag}\n");
    fprintf(tex_dump, "\\usepackage[a4paper, left= 25mm, right=25mm, top=2cm, bottom=2cm]{geometry}\n\n");

    fprintf(tex_dump, "\\begin{document}\n\n");
    fprintf(tex_dump, "\\begin{*equation}\n");
    return NO_ERROR;
}

static int End_TexDump(FILE* tex_dump)
{
    assert(tex_dump);

    if (tex_dump == NULL){
        printf("No such file\n");
        return FILE_NOT_OPEN;
    }

    fprintf(tex_dump, "\\end{*equation}\n");
    fprintf(tex_dump, "\n\\end{document}\n");

    return NO_ERROR;
}


// Открывает латеховский файл.
void Open_LaTEX_File()
{
    latex_ptr = fopen(_latex_fname, "w"); 
    Begin_TexDump(latex_ptr);
}


// Закрывает латеховский файл.
void Close_LaTEX_File()
{
    End_TexDump(latex_ptr);
    fclose(latex_ptr);
    char creat_cmd[BUFSIZ] = {};
    sprintf(creat_cmd, "xelatex -output-directory=latex -interaction=nonstopmode %s > latex/message.txt", _latex_fname);
    system(creat_cmd);
}


// запись итоговой производной в латех-файл
void Write_Data2LaTEX(NODE* head)
{
    assert(head);

    if (head->type == NUM_DATA)
    {
        if (head->data < 0)
            fprintf(latex_ptr, "(%d)", head->data);
        else
            fprintf(latex_ptr, "%d", head->data);
    }
    else if (head->type == VAR_DATA)
        fprintf(latex_ptr, "%c", head->data);
    else if (head->type == OP_DATA)
    {
        
        #define CODEGEN(func)\
        if (head->data == OP_##func)\
        {\
            char tmp_str[BUFSIZ] = {};\
            strcpy(tmp_str, #func);\
            fprintf(latex_ptr, "\\%s(", String2Lower(tmp_str));\
            Write_Data2LaTEX(head->right);\
            fprintf(latex_ptr, ")");\
        }\
        else
        
        if (head->data == OP_SUM || head->data == OP_SUB)
        {
            Write_Data2LaTEX(head->left);
            fprintf(latex_ptr, "%c", (head->data == OP_SUM)? '+': '-');
            Write_Data2LaTEX(head->right);
        }
        else if (head->data == OP_DIV)
        {
            fprintf(latex_ptr, "\\frac{");
            Write_Data2LaTEX(head->left);
            fprintf(latex_ptr, "}{");
            Write_Data2LaTEX(head->right);
            fprintf(latex_ptr, "}");
        }
        else if (head->data == OP_MUL)
        {
            bool isopenbrckt = false;
            if (head->left->type == OP_DATA && (head->left->data == OP_SUM || head->left->data == OP_SUB))
            {
                isopenbrckt = true;
                fprintf(latex_ptr, "(");
            }
            Write_Data2LaTEX(head->left);
            if (isopenbrckt)
            {
                isopenbrckt = false;
                fprintf(latex_ptr, ")");
            }

            fprintf(latex_ptr, " \\cdot ");

            if (head->right->type == OP_DATA && (head->right->data == OP_SUM || head->right->data == OP_SUB))
            {
                isopenbrckt = true;
                fprintf(latex_ptr, "(");
            }
            Write_Data2LaTEX(head->right);
            if (isopenbrckt)
            {
                isopenbrckt = false;
                fprintf(latex_ptr, ")");
            }
        }
        else if (head->data == OP_DEG)
        {
            bool isopenbrckt = false;
            if (head->left->type == OP_DATA && (head->left->data == OP_SUM || head->left->data == OP_SUB))
            {
                isopenbrckt = true;
                fprintf(latex_ptr, "(");
            }
            Write_Data2LaTEX(head->left);
            if (isopenbrckt)
            {
                isopenbrckt = false;
                fprintf(latex_ptr, ")");
            }

            fprintf(latex_ptr, "^");

            if (head->right->type == OP_DATA)
            {
                isopenbrckt = true;
                fprintf(latex_ptr, "{(");
            }
            Write_Data2LaTEX(head->right);
            if (isopenbrckt)
            {
                isopenbrckt = false;
                fprintf(latex_ptr, ")}");
            }
        }
        else 
        #include "func_codegen.h"
        // else
        {
            printf("Can't find function. head->data=%d\n", head->data);
            printf("ERROR. %s:%d\n", __FILE__, __LINE__);
            abort();
        }
        #undef CODEGEN
    }
}



// Добавляет ещё строчку в латех.
void Write_New_Line_To_LaTEX(const char* phrase_beg, NODE* head, const char* phrase_end)
{
    assert(phrase_beg);
    assert(phrase_end);

    fprintf(latex_ptr, "%s", phrase_beg);
    if (head) Write_Data2LaTEX(head);
    fprintf(latex_ptr, "%s", phrase_end);
}