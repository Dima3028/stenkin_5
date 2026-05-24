#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
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

std::string Error::generateErrorMessage() const
{
    switch (type)
    {
    case ErrorType::NoError:
        return "Ошибок нет.";
    case ErrorType::FileNotExist:
        return "Указанный входной файл не существует или к нему нет доступа.";
    case ErrorType::OutFileCreateFail:
        return "Невозможно создать указанный выходной файл.";
    case ErrorType::FileEmpty:
        return "Указанный файл для входных данных пуст.";
    case ErrorType::MultipleRelations:
        return "Во входных данных больше одного знака равенства или неравенства.";
    case ErrorType::InvalidVarName:
        return "Неверное написание имени переменной.";
    case ErrorType::OutOfRange:
        return "Введенные числа (константы) выходят за допустимый диапазон [-1000; 1000].";
    case ErrorType::UnsupportedChar:
        return "Встретилась неподдерживаемая последовательность символов.";
    case ErrorType::NotEnoughOperands:
        return "Недостаточное количество операндов для операции.";
    case ErrorType::NotEnoughOperators:
        return "Недостаточное количество операций (лишние операнды в стеке).";
    case ErrorType::InvalidOperands:
        return "Недопустимые операнды в выражении (например, переменная в показателе степени).";
    case ErrorType::DivideByZero:
        return "Попытка деления на ноль.";
    case ErrorType::NegativePower:
        return "Показатель степени является отрицательным числом.";
    case ErrorType::FractionalPower:
        return "Показатель степени является дробным числом.";
    case ErrorType::MissingRelation:
        return "Во входных данных отсутствует знак равенства или неравенства.";
    default:
        return "Неизвестная ошибка.";
    }
}

std::string formatAllErrors(const std::vector<Error>& errorList)
{
    if (errorList.empty())
    {
        return "Проверка завершена. Ошибок не обнаружено.\n";
    }

    std::string fullReport = "========================================\n";
    fullReport += "ВНИМАНИЕ! Обнаружено ошибок: " + std::to_string(errorList.size()) + "\n";
    fullReport += "========================================\n";

    for (const auto& currentError : errorList)
    {
        fullReport += "Лексема: '" + currentError.invalidLexem + "'\n";
        fullReport += "Позиция: " + std::to_string(currentError.position) + "\n";
        fullReport += "Детали:  " + currentError.generateErrorMessage() + "\n";
        fullReport += "----------------------------------------\n";
    }

    return fullReport;
}

bool areTreesEqual(const ExprNode* node1, const ExprNode* node2)
{
    if (node1 == nullptr && node2 == nullptr)
    {
        return true;
    }
    if (node1 == nullptr || node2 == nullptr)
    {
        return false;
    }

    if (node1->type != node2->type ||
        node1->value != node2->value ||
        node1->varName != node2->varName ||
        node1->coefficient != node2->coefficient)
    {
        return false;
    }

    if (node1->operands.size() != node2->operands.size())
    {
        return false;
    }

    for (size_t i = 0; i < node1->operands.size(); ++i)
    {
        if (!areTreesEqual(node1->operands[i], node2->operands[i]))
        {
            return false;
        }
    }

    return true;
}

void generateDotParams(const ExprNode* node, std::ofstream& outFile)
{
    if (!node)
    {
        return;
    }

    std::string label;
    if (node->type == typeExprNode::var)
    {
        label = node->varName;
    }
    else if (node->type == typeExprNode::con)
    {
        label = std::to_string(node->value);
    }
    else
    {
        label = "Op(" + std::to_string((int)node->type) + ")";
    }

    outFile << "    node" << node->id << " [label=\"" << label << "\"];\n";

    for (const ExprNode* child : node->operands)
    {
        if (child)
        {
            outFile << "    node" << node->id << " -> node" << child->id << ";\n";
            generateDotParams(child, outFile);
        }
    }
}

void generateDotFile(const ExprNode* root, const std::string& filename)
{
    if (!root)
    {
        return;
    }

    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
        return;
    }

    outFile << "digraph ExpressionTree {\n";
    generateDotParams(root, outFile);
    outFile << "}\n";
    outFile.close();
}

bool buildTree(const std::string& rpnString, ExprNode*& root, std::vector<Error>& errors)
{
    // реализовать
    return true;
}

void transformTree(ExprNode* node)
{
    // реализовать
}

void simplifyTree(ExprNode*& node)
{
    // реализовать
}












int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    return 0;
}