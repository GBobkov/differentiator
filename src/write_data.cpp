#include "write_data.h"

#include <stdio.h>

static FILE* latex_ptr = NULL;


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
// Сохранить данные в базу данных
int Write_Data2Base(NODE* node)
{
    FILE* file = fopen(database_file_name, "w");
    Write_New_Node(file, node);
    fclose(file);
    return 0;
}

// начало файла
static int Begin_TexDump(FILE* tex_dump){
    if (tex_dump == NULL){
        printf("No such file\n");
        return FILE_NOT_OPEN;
    }

    fprintf(tex_dump, "\\documentclass{article}\n");
    fprintf(tex_dump, "\\usepackage{ucs}\n");
    fprintf(tex_dump, "\\usepackage[utf8x]{inputenc}\n");
    fprintf(tex_dump, "\\usepackage[russian]{babel}\n");
    fprintf(tex_dump, "\\usepackage{amsmath}\n");

    fprintf(tex_dump, "\\begin{document}\n\n");

    return NO_ERROR;
}

static int End_TexDump(FILE* tex_dump){
    if (tex_dump == NULL){
        printf("No such file\n");
        return FILE_NOT_OPEN;
    }

    fprintf(tex_dump, "\\end{document}\n");

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
}



// запись итоговой производной в латех-файл
void Write_Data2LaTEX(NODE* head)
{

    if (head->type == NUM_DATA)
        fprintf(latex_ptr, "%d", head->data);
    else if (head->type == VAR_DATA)
        fprintf(latex_ptr, "%c", head->data);
    else if (head->type == OP_DATA)
    {
        if (head->data == '+' || head->data == '-')
        {
            Write_Data2LaTEX(head->left);
            fprintf(latex_ptr, "%c", head->data);
            Write_Data2LaTEX(head->right);
        }
        else if (head->data == '/')
        {
            fprintf(latex_ptr, "\\frac{");
            Write_Data2LaTEX(head->left);
            fprintf(latex_ptr, "}{");
            Write_Data2LaTEX(head->right);
            fprintf(latex_ptr, "}");
        }
        else if (head->data == '*')
        {
            bool isopenbrckt = false;
            if (head->left->type == OP_DATA && (head->left->data == '+' || head->left->data == '-'))
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

            fprintf(latex_ptr, "*");

            if (head->right->type == OP_DATA && (head->right->data == '+' || head->right->data == '-'))
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
        else if (head->data == '^')
        {
            bool isopenbrckt = false;
            if (head->left->type == OP_DATA && (head->left->data == '+' || head->left->data == '-'))
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
        else if (head->data == 's')
        {
            fprintf(latex_ptr, "sin{(");
            Write_Data2LaTEX(head->right);
            fprintf(latex_ptr, ")}");
        }
        else if (head->data == 'c')
        {
            fprintf(latex_ptr, "cos{(");
            Write_Data2LaTEX(head->right);
            fprintf(latex_ptr, ")}");
        }
        else if (head->data == 'l')
        {
            fprintf(latex_ptr, "ln{(");
            Write_Data2LaTEX(head->right);
            fprintf(latex_ptr, ")}");
        }
        else if (head->data == 't')
        {
            fprintf(latex_ptr, "tan{(");
            Write_Data2LaTEX(head->right);
            fprintf(latex_ptr, ")}");
        }
    }
}


