#ifndef WRITE_DATA_H
#define WRITE_DATA_H

#include "constants.h"

// Открывает латеховский файл.
void Open_LaTEX_File();

// Закрывает латеховский файл.
void Close_LaTEX_File();

// запись итоговой производной в латех
void Write_Data2LaTEX(NODE* head);

#endif