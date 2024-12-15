#ifndef WRITE_DATA_H
#define WRITE_DATA_H

#include "constants.h"

// Открывает латеховский файл.
void Open_LaTEX_File();

// Закрывает латеховский файл.
void Close_LaTEX_File();

// запись итоговой производной в латех
void Write_Data2LaTEX(NODE* head);

// Сохранить данные в базу данных
int Write_Data2Base(NODE* node);

// ошибки
enum WRITE_DATA_ERRORS
{
    NO_ERROR=0,
    FILE_NOT_OPEN=1
};

#endif