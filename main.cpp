#include "diffor.h"
#include "read_data.h"
#include "tree_dump.h"
#include "tree_for_diff.h"
#include "write_data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *_dump_start_fname = "build/dump_start.dot";
static const char* _dump_enddotfname = "build/dump_end.dot";


int main()
{
    char* user_line_expression = (char *) calloc(BUFSIZE, sizeof(char));
    NODE* head = Handle_Read_Request(user_line_expression);
    Open_LaTEX_File();
    Write_New_Line_To_LaTEX("\n(", head, ")^{'} = ");
    Tree_Dump(_dump_start_fname, head);
    head = Calculate_Derivative(head);
    Tree_Dump(_dump_enddotfname, head);
    Write_New_Line_To_LaTEX("", head, "\n");
    Close_LaTEX_File();
    Destroy_Tree(head);
    free(user_line_expression);
    printf("That's all!\n");
    return 0;
}

