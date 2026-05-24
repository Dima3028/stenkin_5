#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <stack>
#include <cctype>
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

//Вспомогательная функция перевода номера операции в строку 
std::string opToString(typeExprNode type)
{
    switch (type)
    {
    case typeExprNode::plus:   return "+";
    case typeExprNode::minus:  return "-";
    case typeExprNode::mul:    return "*";
    case typeExprNode::div:    return "/";
    case typeExprNode::pow:    return "^";
    case typeExprNode::u_minus:return "_-";
    case typeExprNode::eq:     return "=";
    case typeExprNode::lt:     return "<";
    case typeExprNode::gt:     return ">";
    case typeExprNode::le:     return "<=";
    case typeExprNode::ge:     return ">=";
    default:                   return "Unknown";
    }
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
        label = opToString(node->type);
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

std::vector<std::string> tokenizeRPN(const std::string& rpnString)
{
    // 1. Инициализировать пустой массив строк tokens
    std::vector<std::string> tokens;

    // 2. Инициализировать поток чтения строки из rpnString
    std::istringstream stream(rpnString);

    // 3. Инициализировать временную строку temp
    std::string temp;

    // 4. В цикле, пока успешно считывается строка temp из потока:
    while (stream >> temp)
    {
        // 4.1. Если считанная строка temp не является пустой:
        if (!temp.empty())
        {
            // 4.1.1. Добавить temp в конец массива tokens
            tokens.push_back(temp);
        }
    }

    // 5. Вернуть массив tokens
    return tokens;
}

//Вспомогательная функция проверки: является ли строка переменной
bool isVar(const std::string& token)
{
    return token.length() == 1 && isalpha(token[0]) && islower(token[0]);
}

//Вспомогательная функция проверки: является ли строка константой (целое число)
bool isCon(const std::string& token, int& val)
{
    if (token.empty()) return false;
    size_t start = 0;
    if (token[0] == '-' || token[0] == '+')
    {
        if (token.length() == 1) return false;
        start = 1;
    }
    for (size_t i = start; i < token.length(); ++i)
    {
        if (!isdigit(token[i])) return false;
    }
    try
    {
        val = std::stoi(token);
    }
    catch (...)
    {
        return false;
    }
    return true;
}


void freeTree(ExprNode* node)
{
    if (node == nullptr)
    {
        return;
    }

    for (ExprNode* child : node->operands)
    {
        freeTree(child);
    }

    node->operands.clear();
    delete node;
}

bool buildTree(const std::string& rpnString, ExprNode*& root, std::vector<Error>& errors)
{
    if (rpnString.length() > 500)
    {
        errors.push_back(Error(ErrorType::UnsupportedChar, 500, "Строка > 500"));
        return false;
    }

    // 1. Вызвать tokenizeRPN и получить массив из строки
    std::vector<std::string> tokens = tokenizeRPN(rpnString);

    // 2. Если последняя лексема в массиве НЕ является знаком сравнения
    if (!tokens.empty())
    {
        const std::string& lastToken = tokens.back();
        if (lastToken != "=" && lastToken != "<" && lastToken != ">" &&
            lastToken != "<=" && lastToken != ">=")
        {
            // 2.1. Зафиксировать ошибку MissingRelation
            errors.push_back(Error(ErrorType::MissingRelation, (int) (rpnString.length()), ""));
        }
    }
    else
    {
        errors.push_back(Error(ErrorType::MissingRelation, 0, ""));
    }

    // 3. Инициализировать пустой стек указателей на ExprNode
    std::stack<ExprNode*> st;

    // Лямбда функция очистки стека
    auto clearStack = [&st]() 
        {
        while (!st.empty()) 
        {
            freeTree(st.top());
            st.pop();
        }
        };

    int currentPos = 0;
    int relationCount = 0; // Для проверки: только один знак равенства/неравенства
    int opCount = 0;       // Для проверки: максимум 100 операций

    // 4. В цикле по массиву лексем с начала до конца:
    for (const std::string& token : tokens)
    {
        currentPos = rpnString.find(token, currentPos);
        int pos = (int) currentPos;
        currentPos += token.length();

        int conVal = 0;

        // 4.1. Если лексема – переменная или константа:
        if (isVar(token) || isCon(token, conVal))
        {
            if (isCon(token, conVal))
            {
                if (conVal < -1000 || conVal > 1000)
                {
                    errors.push_back(Error(ErrorType::OutOfRange, pos, token));
                    clearStack(); 
                    return false;
                }
            }

            // 4.1.1. Создать узел и поместить в стек
            ExprNode* node = new ExprNode();
            if (isVar(token))
            {
                node->type = typeExprNode::var;
                node->varName = token;
            }
            else
            {
                node->type = typeExprNode::con;
                node->value = (float)conVal;
            }
            st.push(node);
        }
        // 4.2. Иначе если лексема – унарный минус:
        else if (token == "_-")
        {
            opCount++; // Считаем операцию

            // 4.2.1. Если стек пуст:
            if (st.empty())
            {
                errors.push_back(Error(ErrorType::NotEnoughOperands, pos, token));
                clearStack(); 
                return false;
            }
            // 4.2.2. Достать один элемент из стека
            ExprNode* operand = st.top();
            st.pop();

            // 4.2.3. Создать узел
            ExprNode* node = new ExprNode();
            node->type = typeExprNode::u_minus;
            node->operands.push_back(operand);
            st.push(node);
        }
        // 4.3. Иначе (лексема – бинарная операция или знак сравнения):
        else if (token == "+" || token == "-" || token == "*" || token == "/" || token == "^" ||
            token == "=" || token == "<" || token == ">" || token == "<=" || token == ">=")
        {
            opCount++; // Считаем операцию

            if (token == "=" || token == "<" || token == ">" || token == "<=" || token == ">=")
            {
                relationCount++;
                // ОГРАНИЧЕНИЕ: один оператор сравнения
                if (relationCount > 1)
                {
                    errors.push_back(Error(ErrorType::MultipleRelations, pos, token));
                    clearStack(); 
                    return false;
                }
            }

            // 4.3.1. Если в стеке меньше 2 элементов:
            if (st.size() < 2)
            {
                errors.push_back(Error(ErrorType::NotEnoughOperands, pos, token));
                clearStack(); 
                return false;
            }

            // Извлекаем правый, затем левый операнд
            ExprNode* right = st.top(); st.pop();
            ExprNode* left = st.top(); st.pop();

            
            // Защита от деления на ноль
            if (token == "/")
            {
                if (right->type == typeExprNode::con && right->value == 0.0f)
                {
                    errors.push_back(Error(ErrorType::DivideByZero, pos, token));
                    freeTree(left); 
                    freeTree(right); 
                    clearStack(); 
                    return false;
                }
            }
            // Строгие ограничения для возведения в степень
            else if (token == "^")
            {
                // ОГРАНИЧЕНИЕ: База не поддерживается для не одиночных переменных или не констант
                if (left->type != typeExprNode::var && left->type != typeExprNode::con)
                {
                    errors.push_back(Error(ErrorType::InvalidOperands, pos, token));
                    freeTree(left); 
                    freeTree(right); 
                    clearStack(); 
                    return false;
                }

                // ОГРАНИЧЕНИЕ: Показатель степени - константа (натуральное число <= 100)
                if (right->type != typeExprNode::con)
                {
                    errors.push_back(Error(ErrorType::InvalidOperands, pos, token));
                    freeTree(left); 
                    freeTree(right); 
                    clearStack(); 
                    return false;
                }

                int powerVal = (int)right->value;

                // ОГРАНИЧЕНИЕ: Показатель степени меньше или равен 0
                if (powerVal < 0)
                {
                    errors.push_back(Error(ErrorType::NegativePower, pos, token));
                    freeTree(left); 
                    freeTree(right); 
                    clearStack(); 
                    return false;
                }
                if (powerVal == 0) 
                {
                    errors.push_back(Error(ErrorType::InvalidOperands, pos, token));
                    freeTree(left); 
                    freeTree(right); 
                    clearStack(); 
                    return false;
                }
                // ОГРАНИЧЕНИЕ: Показатель степени больше 100
                if (powerVal > 100)
                {
                    errors.push_back(Error(ErrorType::OutOfRange, pos, token));
                    freeTree(left); 
                    freeTree(right); 
                    clearStack();
                    return false;
                }
            }

            // 4.3.2. Создать новый узел бинарной операции
            ExprNode* node = new ExprNode();
            if (token == "+") node->type = typeExprNode::plus;
            else if (token == "-") node->type = typeExprNode::minus;
            else if (token == "*") node->type = typeExprNode::mul;
            else if (token == "/") node->type = typeExprNode::div;
            else if (token == "^") node->type = typeExprNode::pow;
            else if (token == "=") node->type = typeExprNode::eq;
            else if (token == "<") node->type = typeExprNode::lt;
            else if (token == ">") node->type = typeExprNode::gt;
            else if (token == "<=") node->type = typeExprNode::le;
            else if (token == ">=") node->type = typeExprNode::ge;

            // 4.3.5. Добавить первый операнд, а затем второй операнд
            node->operands.push_back(left);
            node->operands.push_back(right);

            // 4.3.6. Поместить созданный узел в стек
            st.push(node);
        }
        else
        {
            // Обработка неверных имен переменных
            bool isLettersOnly = true;
            for (char c : token) { if (!isalpha(c)) isLettersOnly = false; }

            // ОГРАНИЧЕНИЕ: Переменные только одной строчной буквой 
            if (isLettersOnly) 
            {
                errors.push_back(Error(ErrorType::InvalidVarName, pos, token));
            }
            else 
            {
                errors.push_back(Error(ErrorType::UnsupportedChar, pos, token));
            }
            clearStack(); 
            return false;
        }

        // ОГРАНИЧЕНИЕ: Суммарное число операций не более 100
        if (opCount > 100)
        {
            errors.push_back(Error(ErrorType::UnsupportedChar, pos, "OVER_OP_LIMIT"));
            clearStack(); 
            return false;
        }
    }

    // 6. Если в стеке осталось больше одного элемента И массив ошибок пуст:
    if (st.size() > 1 && errors.empty())
    {
        // 6.1. Зафиксировать ошибку NotEnoughOperators
        errors.push_back(Error(ErrorType::NotEnoughOperators, (int) rpnString.length(), ""));
    }

    // 7. Если массив ошибок не пуст:
    if (!errors.empty())
    {
        clearStack();
        return false;
    }

    // 8. Присвоить единственный оставшийся в стеке элемент
    if (!st.empty())
    {
        root = st.top();
        st.pop();
    }

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