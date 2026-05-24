#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include "Header.h" 

int ExprNode::globalIdCounter = 0;

Error::Error(ErrorType errorType, int errorPosition, std::string lexemText)
{
    type = errorType;
    position = errorPosition;
    invalidLexem = lexemText;
}

ExprNode::ExprNode()
{
    id = ++globalIdCounter;
    type = typeExprNode::con;
    value = 0.0f;
    coefficient = 1;
    hashValue = 0;
}

/**
 * @brief Перегруженный оператор "меньше" для сортировки узлов дерева.
 * @param other Константная ссылка на другой узел для сравнения.
 * @return true, если текущий узел меньше по приоритету или значению, иначе false.
 */
bool ExprNode::operator<(const ExprNode& other) const
{
    //Если типы узлов равны
    if (this->type == other.type)
    {
        //Если тип – переменная
        if (this->type == typeExprNode::var)
        {
            //Сравнить переменные
            return this->varName < other.varName;
        }

        //Если тип – константа
        if (this->type == typeExprNode::con)
        {
            //Сравнить константы
            return this->value < other.value;
        }

        //Иначе (одинаковые операции)
        //Если количество операндов не равно
        if (this->operands.size() != other.operands.size())
        {
            //Сравнить количество операндов
            return this->operands.size() < other.operands.size();
        }

        //Пройти в цикле по всем операндам
        for (size_t i = 0; i < this->operands.size(); ++i)
        {
            //Если текущий операнд this меньше
            if (*this->operands[i] < *other.operands[i])
            {
                return true;
            }
            //Если операнд other меньше текущего
            if (*other.operands[i] < *this->operands[i])
            {
                return false;
            }
        }

        //Если все операнды идентичны
        return false;
    }

    //Иначе (типы узлов разные) – сравниваем приоритеты типов
    return (int)this->type < (int)other.type;
}
















int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    return 0;
}