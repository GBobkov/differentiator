#ifndef TEX_LINES_H
#define TEX_LINES_H

#include "tree_for_diff.h"

#define AMOUNT_OF_TEX_LINES 100

struct TEX_LINE
{
    const char* phrase_beg;
    NODE* head;
    const char* phrase_end;
};

struct TEX_OUTPUT
{
    TEX_LINE** lines;
    int size;
    unsigned capacity;
};



TEX_LINE* Create_Tex_Line(const char* phrase_beg, NODE* head, const char* phrase_end);
void Destroy_Tex_Line(TEX_LINE* line);

// Классичестий конструктор.
TEX_OUTPUT* Create_Tex_Output(unsigned capacity);

// Классический дестройер.
void Destroy_Tex_Output(TEX_OUTPUT* latex);

// Создаёт теховский файл, и пдф файл.
void Make_Pdf(TEX_OUTPUT* lines); 

// Добавит новую линию в конструктор.
void Add_New_Line(TEX_OUTPUT* latex, TEX_LINE* line);


#endif