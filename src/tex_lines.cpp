#include "tex_lines.h"
#include "write_data.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>



TEX_LINE* Create_Tex_Line(const char* phrase_beg, NODE* head, const char* phrase_end)
{
    TEX_LINE* line = (TEX_LINE *) calloc(1, sizeof(TEX_LINE));
    line->head = head;
    line->phrase_beg = phrase_beg;
    line->phrase_end = phrase_end;
    return line;
}


void Destroy_Tex_Line(TEX_LINE* line)
{
    free(line);
}


TEX_OUTPUT* Create_Tex_Output(unsigned capacity)
{
    TEX_LINE** lines = (TEX_LINE **) calloc(capacity, sizeof(TEX_LINE*));
    TEX_OUTPUT* latex =  (TEX_OUTPUT *) calloc(1, sizeof(TEX_OUTPUT));
    latex->capacity = capacity;
    latex->size = 0;
    latex->lines = lines;
    return latex;
}


void Destroy_Tex_Output(TEX_OUTPUT* latex)
{
    free(latex->lines);
    free(latex);
}

void Add_New_Line(TEX_OUTPUT* latex, TEX_LINE* line)
{
    assert(latex);
    
    if (latex->size >= (int) latex->capacity)
    {
        printf("WARNING! Overload capacity LATEX_OUTPUT.\n");
        return;
    }
    latex->lines[latex->size] = line;
    latex->size += 1;
}

// Создаёт теховский файл, и пдф файл.
void Make_Pdf(TEX_OUTPUT* lines)
{
    // Open_LaTEX_File();
    // for (int i = 0; i < lines->size; i++)
    //     Write_New_Line_To_LaTEX(lines->lines[i]);
    Close_LaTEX_File();
}