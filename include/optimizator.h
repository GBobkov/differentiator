#ifndef OPTIMIZATOR_H
#define OPTIMIZATOR_H

#include "tree_for_diff.h"


// проверка на то, что дерево - число. true если число, false если функция.
bool Is_Num(NODE* head);

//функция вычисляет численное значение дерева.
int Calculate_Tree(NODE* node);

// функция оптимизирует граф (умножение на 0, 1 сложение/вычитание с 0). 
void Optimization(NODE* head, int* changes);

#endif