#include "write_data.h"

#include <stdio.h>

static FILE* latex_ptr = NULL;


// Открывает латеховский файл.
void Open_LaTEX_File()
{
    latex_ptr = fopen(_latex_fname, "w"); 
}

// Закрывает латеховский файл.
void Close_LaTEX_File()
{
    fclose(latex_ptr);
}



// запись итоговой производной в латех
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
int Write_Data2Base(NODE* node)
{
    FILE* file = fopen(database_file_name, "w");
    Write_New_Node(file, node);
    fclose(file);
    return 0;
}