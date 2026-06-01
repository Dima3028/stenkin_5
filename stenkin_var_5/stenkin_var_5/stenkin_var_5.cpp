#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <stack>
#include <cctype>
#include "Header.h" 
#include <algorithm>

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

// Функция сортировки дерева
void sortTree(ExprNode* node)
{
    if (!node) return;

    //Рекурсивно сортируем потомков
    for (int i = 0; i < node->operands.size(); ++i)
    {
        sortTree(node->operands[i]);
    }

    //Сортируем операнды только у коммутативных операций
    if (node->type == typeExprNode::plus || node->type == typeExprNode::mul)
    {
        //Сортировка с лямбда выражением
        std::sort(node->operands.begin(), node->operands.end(),[](const ExprNode* a, const ExprNode* b)
            {
                return *a < *b;
            });
    }
}

// Функция формирования сообщения об ошибке
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

// Функция составления сообщения об ошибках
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

bool areTreesEqual(const ExprNode* node1, const ExprNode* node2, bool ignoreCoef)
{
    // Быстрая проверка по хэшу (только если ignoreCoef=false, иначе хэши могут отличаться)
    if (!ignoreCoef)
    {
        if (node1 && node2 && node1->hashValue != node2->hashValue)
            return false;
    }

    if (node1 == node2)         // оба nullptr или один и тот же указатель
        return true;

    if (!node1 || !node2)
        return false;

    if (node1->type != node2->type ||
        node1->varName != node2->varName ||
        node1->value != node2->value)
        return false;

    if (!ignoreCoef && node1->coefficient != node2->coefficient)
        return false;

    if (node1->operands.size() != node2->operands.size())
        return false;

    for (size_t i = 0; i < node1->operands.size(); ++i)
    {
        if (!areTreesEqual(node1->operands[i], node2->operands[i], ignoreCoef))
            return false;
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

// Фнукция формирования строки с данными для графового файла
void generateDotParams(const ExprNode* node, std::ofstream& outFile)
{
    if (!node) return;

    std::string label;
    if (node->type == typeExprNode::var)
    {
        label = node->varName;
        if (node->coefficient != 1)
            label += "\\ncoef=" + std::to_string(node->coefficient);
    }
    else if (node->type == typeExprNode::con)
    {
        label = std::to_string(node->value);
        if (node->coefficient != 1)
            label += "\\ncoef=" + std::to_string(node->coefficient);
    }
    else
    {
        label = opToString(node->type);
        if (node->coefficient != 1)
            label += "\\ncoef=" + std::to_string(node->coefficient);
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

// Функция генерации файлы для графа
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

// Функция разбиения строки на массив лексем
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

//Функция очистки памяти
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

// Функция построения дерева из строки
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
    int relationCount = 0; 
    int opCount = 0;       

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
                //ОГРАНИЧЕНИЕ: один оператор сравнения
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

            //Извлекаем правый, затем левый операнд, потому что праавый появился в стеке после левого
            ExprNode* right = st.top(); st.pop();
            ExprNode* left = st.top(); st.pop();

            
            //Защита от деления на ноль
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
            
            else if (token == "^")
            {
                //ОГРАНИЧЕНИЕ: База не поддерживается для не одиночных переменных или не констант или не степеней
                if (left->type != typeExprNode::var &&
                    left->type != typeExprNode::con &&
                    left->type != typeExprNode::pow)
                {
                    errors.push_back(Error(ErrorType::InvalidOperands, pos, token));
                    freeTree(left); freeTree(right); clearStack(); return false;
                }

                //ОГРАНИЧЕНИЕ: Показатель степени - константа (натуральное число <= 100) или степень
                if (right->type != typeExprNode::con &&
                    right->type != typeExprNode::pow)
                {
                    errors.push_back(Error(ErrorType::InvalidOperands, pos, token));
                    freeTree(left); freeTree(right); clearStack(); return false;
                }
                //ОГРАНИЧЕНИЯ для показателя степени
                if (right->type == typeExprNode::con)
                {
                    int powerVal = (int)right->value;
                    if (powerVal < 0)
                    {
                        errors.push_back(Error(ErrorType::NegativePower, pos, token));
                        freeTree(left); freeTree(right); clearStack(); return false;
                    }
                    if (powerVal == 0)
                    {
                        errors.push_back(Error(ErrorType::InvalidOperands, pos, token));
                        freeTree(left); freeTree(right); clearStack(); return false;
                    }
                    if (powerVal > 100)
                    {
                        errors.push_back(Error(ErrorType::OutOfRange, pos, token));
                        freeTree(left); freeTree(right); clearStack(); return false;
                    }
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

            //ОГРАНИЧЕНИЕ: Переменные только одной строчной буквой 
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

        //ОГРАНИЧЕНИЕ: Суммарное число операций не более 100
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

//Вычисление хеша
void computeHash(ExprNode* node)
{
    if (!node) return;

    for (ExprNode* child : node->operands)
        computeHash(child);

    std::hash<int> intHasher;
    node->hashValue = intHasher((int)node->type);

    //Рассчет для узлов-переменных
    if (node->type == typeExprNode::var)
    {
        std::hash<std::string> strHasher;
        node->hashValue ^= strHasher(node->varName)
            + 0x9e3779b9 + (node->hashValue << 6) + (node->hashValue >> 2);
    }
    //Рассчет для узлов-чисел 
    else if (node->type == typeExprNode::con)
    {
        std::hash<float> floatHasher;
        node->hashValue ^= floatHasher(node->value)
            + 0x9e3779b9 + (node->hashValue << 6) + (node->hashValue >> 2);
    }
    
    else
    {
        for (ExprNode* child : node->operands)
        {
            //Рассчет для узлов-операций "+", "*", как для коммутативных
            if (node->type == typeExprNode::plus || node->type == typeExprNode::mul)
            {
                node->hashValue += child->hashValue;
            }
            //Рассчет для остальных узлов
            else
            {
                node->hashValue ^= child->hashValue
                    + 0x9e3779b9 + (node->hashValue << 6) + (node->hashValue >> 2);
            }
        }
    }
}

void transformTree(ExprNode* node)
{
    if (!node) return;

    //Рекурсивынй спуск
    for (ExprNode* child : node->operands)
        transformTree(child);

    //Приведение унарного минуса к отрицательному коэфициенту узла
    if (node->type == typeExprNode::u_minus)
    {
        ExprNode* child = node->operands[0];

        node->type = child->type;
        node->varName = child->varName;
        node->value = child->value;
        node->coefficient *= -1;

        node->operands.clear();
        for (ExprNode* grandChild : child->operands)
            node->operands.push_back(grandChild);

        child->operands.clear();
        delete child;
    }

    //Приведение бинарного минуса к "+" и отрацитальному коэфициенту
    if (node->type == typeExprNode::minus)
    {
        node->type = typeExprNode::plus;
        node->operands[1]->coefficient *= -1;
    }

    //Приведение двойного деления к делению на произведение делителей
    if (node->type == typeExprNode::div &&
        node->operands[0]->type == typeExprNode::div)
    {
        ExprNode* innerDiv = node->operands[0];

        ExprNode* mulNode = new ExprNode();
        mulNode->type = typeExprNode::mul;
        mulNode->operands.push_back(innerDiv->operands[1]); // b
        mulNode->operands.push_back(node->operands[1]);     // c

        node->operands[0] = innerDiv->operands[0]; // a
        node->operands[1] = mulNode;

        innerDiv->operands.clear();
        delete innerDiv;

        transformTree(mulNode);
    }

    //Перенос детей на уровни выше, если типы операций совпадают для "+", "*"
    if (node->type == typeExprNode::plus || node->type == typeExprNode::mul)
    {
        std::vector<ExprNode*> newChild;

        for (ExprNode* child : node->operands)
        {
            if (child->type == node->type)
            {
                if (node->type == typeExprNode::plus)
                {
                    for (ExprNode* grandChild : child->operands)
                        grandChild->coefficient *= child->coefficient;
                }
                else // mul
                {
                    node->coefficient *= child->coefficient;
                }

                for (ExprNode* grandChild : child->operands)
                    newChild.push_back(grandChild);

                child->operands.clear();
                delete child;
            }
            else
            {
                newChild.push_back(child);
            }
        }

        node->operands = newChild;
    }

    //Перерасчет хеша
    computeHash(node);
}

// Упрощение степени 
void simplifyPow(ExprNode*& node)
{
    if (node->type != typeExprNode::pow)
        return;

    ExprNode* leftOp = node->operands[0];
    ExprNode* rightOp = node->operands[1];
    
    //Раскрытие вложенной степени 
    if (leftOp->type == typeExprNode::pow &&
        leftOp->operands[1]->type == typeExprNode::con &&
        rightOp->type == typeExprNode::con)
    {
        ExprNode* base = leftOp->operands[0];
        ExprNode* innerExp = leftOp->operands[1];
        float     outerVal = rightOp->value;

        innerExp->value *= outerVal;

        freeTree(node->operands[1]);   
        leftOp->operands.clear();
        delete leftOp;                

        node->operands[0] = base;
        node->operands[1] = innerExp;

        leftOp = node->operands[0];
        rightOp = node->operands[1];
    }

    //Вычисление констант 
    if (leftOp->type == typeExprNode::con &&
        rightOp->type == typeExprNode::con)
    {
        float result = std::pow(leftOp->value, rightOp->value);
        freeTree(node->operands[0]);
        freeTree(node->operands[1]);
        node->operands.clear();
        node->type = typeExprNode::con;
        node->value = result;
    }
}

// Упрощение деления
void simplifyDiv(ExprNode*& node)
{
    if (node->type != typeExprNode::div || node->operands.size() != 2)
        return;

    ExprNode* num = node->operands[0];
    ExprNode* den = node->operands[1];

    // 4.1: числитель == знаменатель => 1
    if (areTreesEqual(num, den))
    {
        freeTree(num);
        freeTree(den);
        node->operands.clear();
        node->type = typeExprNode::con;
        node->value = 1.0f;
        node->coefficient = 1;
        return;
    }

    // 4.2: числитель == 0 => 0
    if (num->type == typeExprNode::con && num->value == 0.0f)
    {
        freeTree(num);
        freeTree(den);
        node->operands.clear();
        node->type = typeExprNode::con;
        node->value = 0.0f;
        node->coefficient = 1;
        return;
    }

    // 4.3: оба константы и знаменатель != 0 => вычислить
    if (num->type == typeExprNode::con &&
        den->type == typeExprNode::con &&
        den->value != 0.0f)
    {
        float result = num->value / den->value;
        freeTree(num);
        freeTree(den);
        node->operands.clear();
        node->type = typeExprNode::con;
        node->value = result;
    }
}

// Проверка нуля в операндах умножениея
bool mulHasZero(ExprNode* node)
{
    // Если в операндах умножения есть хоть один 0
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        ExprNode* child = node->operands[i];
        if ((child->type == typeExprNode::con && child->value == 0.0f) || child->coefficient == 0)
            return true;
    }
    return false;
}

// Вынос коэффициентов потомков
void mulCollectCoefficients(ExprNode* node)
{
    int totalCoef = node->coefficient;
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        totalCoef *= node->operands[i]->coefficient;
        node->operands[i]->coefficient = 1;
    }
    node->coefficient = totalCoef;
}

// Вычисление констант в умножении
void mulMergeConstants(ExprNode* node)
{
    float constProduct = 1.0f;
    bool hadConst = false;
    std::vector<ExprNode*> nonConst;

    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        ExprNode* child = node->operands[i];
        if (child->type == typeExprNode::con)
        {
            constProduct *= child->value;
            hadConst = true;
            freeTree(child);
        }
        else
        {
            nonConst.push_back(child);
        }
    }
    node->operands = nonConst;

    if (hadConst && constProduct != 1.0f)
    {
        ExprNode* constNode = new ExprNode();
        constNode->type = typeExprNode::con;
        constNode->value = constProduct;
        node->operands.insert(node->operands.begin(), constNode);
    }
}

// Свертывание произведения одиннаковых деревьев в степень
void mulGroupIntoPow(ExprNode* node)
{
    for (int i = 0; i < (int)node->operands.size(); )
    {
        int count = 1;
        while (i + count < (int)node->operands.size() &&
            areTreesEqual(node->operands[i], node->operands[i + count]))
        {
            ++count;
        }

        if (count > 1)
        {
            ExprNode* expConst = new ExprNode();
            expConst->type = typeExprNode::con;
            expConst->value = (float)count;
            ExprNode* powNode = new ExprNode();
            powNode->type = typeExprNode::pow;
            powNode->operands.push_back(node->operands[i]);
            powNode->operands.push_back(expConst);

            for (int k = 1; k < count; ++k)
                freeTree(node->operands[i + k]);

            node->operands.erase(node->operands.begin() + i + 1, node->operands.begin() + i + count);
            node->operands[i] = powNode;
        }
        ++i;
    }
}

// Приведение константы к коэфициенту 
void mulAbsorbConstantIntoCoef(ExprNode*& node)
{
    if (node->type != typeExprNode::mul)
        return;

    ExprNode* nonConstNode = nullptr;
    int       nonConstCount = 0;
    float     constProduct = 1.0f;

    // Вычисление константы
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        ExprNode* child = node->operands[i];
        if (child->type == typeExprNode::con)
            constProduct *= child->value;
        else
        {
            nonConstNode = child;
            ++nonConstCount;
        }
    }

    if (nonConstCount != 1 || node->operands.size() <= 1)
        return;

    int intConst = (int)constProduct;
    if ((float)intConst != constProduct)
        return;

    // Удаляем константные узлы
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        if (node->operands[i] != nonConstNode)
            freeTree(node->operands[i]);
    }
    node->operands.clear();

    // Поглощаем nonConstNode в текущий узел
    node->type = nonConstNode->type;
    node->varName = nonConstNode->varName;
    node->value = nonConstNode->value;
    node->coefficient *= nonConstNode->coefficient * intConst;
    node->operands = nonConstNode->operands;

    nonConstNode->operands.clear();
    delete nonConstNode;
}

// Все упрощения для умножения
void simplifyMul(ExprNode*& node)
{
    if (node->type != typeExprNode::mul)
        return;

    // Если есть 0 в операндах, то весь узел = 0
    if (mulHasZero(node))
    {
        for (int i = 0; i < (int)node->operands.size(); ++i)
            freeTree(node->operands[i]);
        node->operands.clear();
        node->type = typeExprNode::con;
        node->value = 0.0f;
        node->coefficient = 1;
        computeHash(node);
        return;
    }

    mulCollectCoefficients(node);
    mulMergeConstants(node);      
    mulGroupIntoPow(node);    
    mulAbsorbConstantIntoCoef(node); 
}

// Вычисление констант для +
void plusMergeConstants(ExprNode* node)
{
    float                  constSum = 0.0f;
    bool                   hadConst = false;
    std::vector<ExprNode*> nonConst;

    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        ExprNode* child = node->operands[i];
        if (child->type == typeExprNode::con)
        {
            constSum += child->value * (float) (child->coefficient);
            hadConst = true;
            freeTree(child);
        }
        else
        {
            nonConst.push_back(child);
        }
    }
    node->operands = nonConst;

    if (hadConst)
    {
        ExprNode* constNode = new ExprNode();
        constNode->type = typeExprNode::con;
        constNode->value = constSum;
        node->operands.push_back(constNode);
    }
}

// Удаление нулевых слагаемых
void plusRemoveZeros(ExprNode* node)
{
    std::vector<ExprNode*> filtered;
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        ExprNode* child = node->operands[i];
        if ((child->type == typeExprNode::con && child->value == 0.0f) ||
            child->coefficient == 0)
            freeTree(child);
        else
            filtered.push_back(child);
    }
    node->operands = filtered;
}

// Приведение подобных деревьев для +
void plusCombineLikeTerms(ExprNode* node)
{
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        for (int j = i + 1; j < (int)node->operands.size(); )
        {
            if (areTreesEqual(node->operands[i], node->operands[j], true))
            {
                node->operands[i]->coefficient += node->operands[j]->coefficient;
                freeTree(node->operands[j]);
                node->operands.erase(node->operands.begin() + j);
            }
            else
            {
                ++j;
            }
        }
    }
}

// Все упрощения сложения
void simplifyPlus(ExprNode* node)
{
    if (node->type != typeExprNode::plus)
        return;

    plusMergeConstants(node);    
    plusRemoveZeros(node);   
    plusCombineLikeTerms(node);
    plusRemoveZeros(node); 
}

// Схлопывание пустых и одиночных узлов дерева
void collapseNode(ExprNode*& node)
{
    if (node->type != typeExprNode::plus && node->type != typeExprNode::mul)
        return;

    typeExprNode originalType = node->type;

    if (node->operands.empty())
    {
        node->type = typeExprNode::con;
        if (originalType == typeExprNode::plus)
        {
            node->value = 0.0f;
        }
        else
        {
            node->value = 1.0f;
        }
        return;
    }

    if (node->operands.size() == 1)
    {
        ExprNode* only = node->operands[0];
        node->type = only->type;
        node->varName = only->varName;
        node->value = only->value;
        node->coefficient *= only->coefficient;
        node->operands = only->operands;
        only->operands.clear();
        delete only;
    }
}

// Упрощение всего дерева
void simplifyTree(ExprNode*& node)
{
    if (!node) return;

    for (int i = 0; i < (int)node->operands.size(); ++i) 
        simplifyTree(node->operands[i]);

    simplifyPow(node);
    simplifyDiv(node);
    simplifyMul(node);
    simplifyPlus(node);
    collapseNode(node);
    computeHash(node);   
}

// Функция переброски правой части в левую
void moveTermsToLeft(ExprNode*& root)
{
    // Проверка существует ли непустой корень
    if (!root || root->operands.size() != 2)
        return;

    ExprNode* rightPart = root->operands[1];

    // Если правая часть уже 0 — уже приравнено 
    if (rightPart->type == typeExprNode::con && rightPart->value == 0.0f)
        return;

    // Создаём новый узел +
    ExprNode* plusNode = new ExprNode();
    plusNode->type = typeExprNode::plus;
    // Левая часть переходит в + 
    plusNode->operands.push_back(root->operands[0]);
    // Правая часть переходит в + с отрицательным коэффициентом
    rightPart->coefficient *= -1;
    plusNode->operands.push_back(rightPart);
    // Создаём новую константу 0 для правой части
    ExprNode* zero = new ExprNode();
    zero->type = typeExprNode::con;
    zero->value = 0.0f;
    // Строим потомков корня
    root->operands[0] = plusNode;
    root->operands[1] = zero;

}

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    std::string inputPath, outputPath;
    std::cout << "Введите путь к входному файлу: ";
    std::getline(std::cin, inputPath);
    std::cout << "Введите путь к выходному файлу: ";
    std::getline(std::cin, outputPath);

    // Получение входных данных
    std::ifstream inputFile(inputPath);
    if (!inputFile.is_open())
    {
        std::vector<Error> error;
        error.push_back(Error(ErrorType::FileNotExist, 0, inputPath));
        std::cout << formatAllErrors(error);
        return 1;
    }

    std::string rpnString;
    std::getline(inputFile, rpnString);
    inputFile.close();

    // Если файл пуст
    if (rpnString.empty())
    {
        std::vector<Error> error;
        error.push_back(Error(ErrorType::FileEmpty, 0, ""));
        std::cout << formatAllErrors(error);
        return 1;
    }

    // Построение дерева
    ExprNode* root = nullptr;
    std::vector<Error> errors;

    if (!buildTree(rpnString, root, errors))
    {
        std::cout << formatAllErrors(errors);
        return 1;
    }

    // Создание файлы выходв
    std::ofstream outFile(outputPath);
    if (!outFile.is_open())
    {
        std::vector<Error> error;
        error.push_back(Error(ErrorType::OutFileCreateFail, 0, outputPath));
        std::cout << formatAllErrors(error);
        freeTree(root);
        return 1;
    }

    // Фиксация входного дерева
    outFile << "// Входное выражение\n";
    outFile << "digraph Input {\n";
    generateDotParams(root, outFile);
    outFile << "}\n\n";

    // Преобразования
    moveTermsToLeft(root);
    transformTree(root);
    sortTree(root);
    simplifyTree(root);

    // Фиксация выходного дерева
    outFile << "// Результат\n";
    outFile << "digraph Output {\n";
    generateDotParams(root, outFile);
    outFile << "}\n";
    outFile.close();
    freeTree(root);

    std::cout << "Готово. Результат записан в: " << outputPath << "\n";
    return 0;
}