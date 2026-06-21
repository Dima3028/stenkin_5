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
#include <set>

//Глобальный счетчик узлов для дот файла
int ExprNode::globalIdCounter = 0;

//Все бинарные операции
static const std::set<std::string> binaryOperatorTokens =
{ "+", "-", "*", "/", "^", "=", "<", ">", "<=", ">=" };

//Все операции сравнения
static const std::set<std::string> relationTokens =
{ "=", "<", ">", "<=", ">=" };

//Конструктор для ошибки
Error::Error (ErrorType errorType, int errorPosition, const std::string& lexemText): type(errorType), position(errorPosition), invalidLexem(lexemText)
{
}

//Конструктор для узла выражения
ExprNode::ExprNode()
{
    id = ++globalIdCounter;
    type = typeExprNode::con;
    value = 0.0f;
    coefficient = 1;
    hashValue = 0;
}

 // Оператор сравнения
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
    //Если строка пустая
    if (errorList.empty())
    {
        //Нет ошибок
        return "Проверка завершена. Ошибок не обнаружено.\n";
    }

    //Иначе выводим ошибки
    std::string fullReport = "========================================\n";
    fullReport += "ВНИМАНИЕ! Обнаружено ошибок: " + std::to_string(errorList.size()) + "\n";
    fullReport += "========================================\n";

    //Формирование строки с каждой ошибкой
    for (const auto& currentError : errorList)
    {
        fullReport += "Лексема: '" + currentError.invalidLexem + "'\n";
        fullReport += "Позиция: " + std::to_string(currentError.position) + "\n";
        fullReport += "Детали:  " + currentError.generateErrorMessage() + "\n";
        fullReport += "----------------------------------------\n";
    }

    return fullReport;
}

// ФФункция сравнения деревьев
bool areTreesEqual(const ExprNode* node1, const ExprNode* node2, bool ignoreCoef)
{
    // Быстрая проверка по хэшу (только если ignoreCoef=false, иначе хэши могут отличаться)
    if (!ignoreCoef)
    {
        if (node1 && node2 && node1->hashValue != node2->hashValue)
            //деревья разные
            return false;
    }

    if (node1 == node2)         // оба nullptr или один и тот же указатель
        return true;

    if (!node1 || !node2) //Если один из указателей пуст, а другой нет
        //деревья разные
        return false;

    //Если не совпадает тип
    //или не совпадает имя
    //или не совпадает значение
    if (node1->type != node2->type ||
        node1->varName != node2->varName ||
        node1->value != node2->value)
        //деревья разные 
        return false;
    //При проверке коеффициентов, если они неравны
    if (!ignoreCoef && node1->coefficient != node2->coefficient)
        //деревья разные
        return false;
    
    //Если разное кол-во потомков
    if (node1->operands.size() != node2->operands.size())
        //деревья разные
        return false;

    //Если потомки не совпадают
    for (size_t i = 0; i < node1->operands.size(); ++i)
    {
        if (!areTreesEqual(node1->operands[i], node2->operands[i], ignoreCoef))
            //деревья разные
            return false;
    }

    //Иначе деревья одиннаковые
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
    // Если узел пустой — выходим
    if (!node) return;

    //Формируем подпись узла
    std::string label;
    //Если переменная
    if (node->type == typeExprNode::var)
    {
        //Берем ее имя 
        label = node->varName;
        //Учитываем коэф, если есть
        if (node->coefficient != 1)
            label += "\\ncoef=" + std::to_string(node->coefficient);
    }
    //Если число
    else if (node->type == typeExprNode::con)
    {
        //Берем его значение
        label = std::to_string(node->value);
        //Учитываем коэф, если есть
        if (node->coefficient != 1)
            label += "\\ncoef=" + std::to_string(node->coefficient);
    }
    //Иначе операция
    else
    {
        //Берем ее тип
        label = opToString(node->type);
        //Учитываем коэф, если есть
        if (node->coefficient != 1)
            label += "\\ncoef=" + std::to_string(node->coefficient);
    }

    outFile << "    node" << node->id << " [label=\"" << label << "\"];\n";

    //Для всех потомков повторить
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
void generateDotFile(const ExprNode* root, const std::string& filename, bool append)
{
    if (!root)
    {
        return;
    }

    std::ofstream outFile;

    if (append == true)
    {
        //Открываем файл в режиме дозаписи, текст добавится в конец
        outFile.open(filename, std::ios::app);
    }
    else
    {
        // Открываем файл заново, старое содержимое стирается
        outFile.open(filename);
    }

    //Если нет доступа к файлу, выходим
    if (!outFile.is_open())
    {
        return;
    }

    //Запись дерева
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
    //Если пустая, то нет
    if (token.empty()) return false;
    size_t start = 0;
    //Проверяем наличие знака
    if (token[0] == '-' || token[0] == '+')
    {
        if (token.length() == 1) return false;
        start = 1;
    }
    //Проверяем дальнейшее число в строке
    for (size_t i = start; i < token.length(); ++i)
    {
        if (!isdigit(token[i])) return false;
    }
    //Пытаемся преобразовать
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
    //Если узел пуст, то конец
    if (node == nullptr)
    {
        return;
    }
    //Для всех потомков повторить
    for (ExprNode* child : node->operands)
    {
        freeTree(child);
    }
    //Удалить узел
    node->operands.clear();
    delete node;
}

// Проверка длины входной строки
bool validateInputLength(const std::string& rpnString, std::vector<Error>& errors)
{
    if (rpnString.length() > 500)
    {
        errors.push_back(Error(ErrorType::UnsupportedChar, 500, "Строка > 500"));
        return false;
    }
    return true;
}

// Проверка, что выражение заканчивается знаком сравнения (=, <, > и т.д.)
void validateRelationPresence(const std::vector<std::string>& tokens,
    const std::string& rpnString, std::vector<Error>& errors)
{
    //Если массив не пуст
    if (!tokens.empty())
    {
        //Проверить, что полседняя лексема это операция сравнения
        const std::string& lastToken = tokens.back();
        if (lastToken != "=" && lastToken != "<" && lastToken != ">" &&
            lastToken != "<=" && lastToken != ">=")
        {
            errors.push_back(Error(ErrorType::MissingRelation, (int)(rpnString.length()), ""));
        }
    }
    else
    {
        errors.push_back(Error(ErrorType::MissingRelation, 0, ""));
    }
}

// Обработка лексемы-операнда
bool handleOperandToken(const std::string& token, int pos,
    std::stack<ExprNode*>& st, std::vector<Error>& errors)
{
    int conVal = 0;
    //Если число, проверяем допустимые пределы
    if (isCon(token, conVal))
    {
        if (conVal < -1000 || conVal > 1000)
        {
            errors.push_back(Error(ErrorType::OutOfRange, pos, token));
            return false;
        }
    }
    //Создаем новый узел и заполняем в зависимости от типа
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
    return true;
}

// Обработка унарного минуса (_-)
bool handleUnaryMinusToken(const std::string& token, int pos,
    std::stack<ExprNode*>& st, std::vector<Error>& errors)
{
    //Если стек пуст — минус не к чему применить
    if (st.empty())
    {
        errors.push_back(Error(ErrorType::NotEnoughOperands, pos, token));
        return false;
    }
    
    //Создаем новый узел с потомком
    ExprNode* operand = st.top();
    st.pop();

    ExprNode* node = new ExprNode();
    node->type = typeExprNode::u_minus;
    node->operands.push_back(operand);
    st.push(node);
    return true;
}

// Проверка ограничений для операции возведения в степень
bool validatePowerOperands(const ExprNode* left, const ExprNode* right,
    int pos, std::vector<Error>& errors)
{
    //Проверяем базу степени (левый операнд)
    if (left->type != typeExprNode::var &&
        left->type != typeExprNode::con &&
        left->type != typeExprNode::pow)
    {
        errors.push_back(Error(ErrorType::InvalidOperands, pos, "^"));
        return false;
    }

    //Проверяем показатель степени (правый операнд)
    if (right->type != typeExprNode::con &&
        right->type != typeExprNode::pow)
    {
        errors.push_back(Error(ErrorType::InvalidOperands, pos, "^"));
        return false;
    }

    //Если показатель — константа, проверяем её диапазон
    if (right->type == typeExprNode::con)
    {
        int powerVal = (int)right->value;
        if (powerVal < 0)
        {
            errors.push_back(Error(ErrorType::NegativePower, pos, "^"));
            return false;
        }
        if (powerVal == 0)
        {
            errors.push_back(Error(ErrorType::InvalidOperands, pos, "^"));
            return false;
        }
        if (powerVal > 100)
        {
            errors.push_back(Error(ErrorType::OutOfRange, pos, "^"));
            return false;
        }
    }
    return true;
}

// Переводит строковый символ операции в тип узла дерева
typeExprNode tokenToNodeType(const std::string& token)
{
    if (token == "+")  return typeExprNode::plus;
    if (token == "-")  return typeExprNode::minus;
    if (token == "*")  return typeExprNode::mul;
    if (token == "/")  return typeExprNode::div;
    if (token == "^")  return typeExprNode::pow;
    if (token == "=")  return typeExprNode::eq;
    if (token == "<")  return typeExprNode::lt;
    if (token == ">")  return typeExprNode::gt;
    if (token == "<=") return typeExprNode::le;
    return typeExprNode::ge;
}

// Считает количество знаков сравнения и проверяет, что их не больше одного
bool checkRelationLimit(const std::string& token, int pos,
    int& relationCount, std::vector<Error>& errors)
{
    //Если токен вообще не знак сравнения
    if (relationTokens.count(token) == 0)
        return true;
    //Токен оказался знаком сравнения 
    relationCount++;
    //Если это уже второй (или более) знак сравнения — ошибка
    if (relationCount > 1)
    {
        errors.push_back(Error(ErrorType::MultipleRelations, pos, token));
        return false;
    }
    return true;
}

// Проверяет операнды бинарной операции в зависимости от её вида
bool validateOperandsForOperator(const std::string& token, const ExprNode* left,
    const ExprNode* right, int pos, std::vector<Error>& errors)
{
    //Деление - проверяем, что знаменатель не равен константе 0
    if (token == "/")
    {
        if (right->type == typeExprNode::con && right->value == 0.0f)
        {
            errors.push_back(Error(ErrorType::DivideByZero, pos, token));
            return false;
        }
        return true;
    }
    //Степень 
    if (token == "^")
        return validatePowerOperands(left, right, pos, errors);

    return true;
}

// Обработка бинарной операции или знака сравнения
bool handleBinaryOperatorToken(const std::string& token, int pos,
    std::stack<ExprNode*>& st, int& relationCount, std::vector<Error>& errors)
{
    //Считаем знаки сравнения
    if (!checkRelationLimit(token, pos, relationCount, errors))
        return false;

    //Проверяем кол-во операторов
    if (st.size() < 2)
    {
        errors.push_back(Error(ErrorType::NotEnoughOperands, pos, token));
        return false;
    }

    //Достаем операдны
    // Сначала правый
    ExprNode* right = st.top(); st.pop();
    //Затем левый
    ExprNode* left = st.top(); st.pop();

    //Проверяем операнды на ошибки
    if (!validateOperandsForOperator(token, left, right, pos, errors))
    {
        freeTree(left);
        freeTree(right);
        return false;
    }
    
    //Создаем узел
    ExprNode* node = new ExprNode();
    node->type = tokenToNodeType(token);
    node->operands.push_back(left);
    node->operands.push_back(right);
    st.push(node);
    return true;
}

// Обработка некорректной лексемы
void handleInvalidToken(const std::string& token, int pos, std::vector<Error>& errors)
{
    bool isLettersOnly = true;
    for (char c : token) { if (!isalpha(c)) isLettersOnly = false; }

    if (isLettersOnly)
    {
        errors.push_back(Error(ErrorType::InvalidVarName, pos, token));
    }
    else
    {
        errors.push_back(Error(ErrorType::UnsupportedChar, pos, token));
    }
}


// Определяет тип текущей лексемы и вызывает нужный обработчик
bool processToken(const std::string& token, int pos, std::stack<ExprNode*>& st, int& relationCount, int& opCount, std::vector<Error>& errors)
{
    int conVal = 0;

    //Лексема — переменная или константа (операнд)
    if (isVar(token) || isCon(token, conVal))
        return handleOperandToken(token, pos, st, errors);

    //Лексема — унарный минус
    if (token == "_-")
    {
        opCount++;
        return handleUnaryMinusToken(token, pos, st, errors);
    }

    //Лексема — бинарная операция или знак сравнения
    if (binaryOperatorTokens.count(token) > 0)
    {
        opCount++;
        return handleBinaryOperatorToken(token, pos, st, relationCount, errors);
    }

    //Остальные случаи - ошибка
    handleInvalidToken(token, pos, errors);
    return false;
}

//Функция построения бинарного дерева из строки
bool buildTree(const std::string& rpnString, ExprNode*& root, std::vector<Error>& errors)
{
    //Проверка длины строки
    if (!validateInputLength(rpnString, errors))
        return false;

    //Разбие строик на лексемы
    std::vector<std::string> tokens = tokenizeRPN(rpnString);

    //Проверка, что последний знак -  операция сравнения
    validateRelationPresence(tokens, rpnString, errors);

    std::stack<ExprNode*> st;

    //Лямбда для очистки стека при ошибке, освобождает память всех узлов
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

    // Обработка всех лексем
    for (const std::string& token : tokens)
    {
        currentPos = (int)rpnString.find(token, currentPos);
        int pos = currentPos;
        currentPos += (int)token.length();

        //Обрабатываем лексему
        // если возникла ошибка — прерываем разбор
        if (!processToken(token, pos, st, relationCount, opCount, errors))
        {
            clearStack();
            return false;
        }

        //Выход за предел операций - ошибка
        if (opCount > 100)
        {
            errors.push_back(Error(ErrorType::UnsupportedChar, pos, "OVER_OP_LIMIT"));
            clearStack();
            return false;
        }
    }

    // Если в стеке осталось больше 1 узла
    if (st.size() > 1 && errors.empty())
    {
        errors.push_back(Error(ErrorType::NotEnoughOperators, (int)rpnString.length(), ""));
    }

    //Если есть ошибки
    if (!errors.empty())
    {
        clearStack();
        return false;
    }

    //Если остался узел - он корень
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
    //Если текущий узел пуст -	Прервать выполнение
    if (!node) return;

    //В цикле для каждого дочернего узла 
    for (ExprNode* child : node->operands)
        //Вызвать computeHash для дочернего узла 
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
        for (const ExprNode* child : node->operands)
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

// Приводит унарный минус к отрицательному коэффициенту узла
void normalizeUnaryMinus(ExprNode* node)
{
    //Если это не унарный минус — делать нечего
    if (node->type != typeExprNode::u_minus)
        return;

    //Копируем содержимое ребёнка в текущий узел
    ExprNode* child = node->operands[0];

    node->type = child->type;
    node->varName = child->varName;
    node->value = child->value;
    node->coefficient *= -1;

    //Переносим детей ребёнка, если они были в текущий узел
    node->operands.clear();
    for (ExprNode* grandChild : child->operands)
        node->operands.push_back(grandChild);

    //Удаляем старый узел
    child->operands.clear();
    delete child;
}

// Приводит бинарный минус к сложению с отрицательным
void normalizeBinaryMinus(ExprNode* node)
{
    if (node->type != typeExprNode::minus)
        return;

    //Меняем минус на +
    node->type = typeExprNode::plus;
    // Умножаем второй операнд на -1
    node->operands[1]->coefficient *= -1;
}

// Приводит двойное деление к делению на произведение делителей
void normalizeNestedDivision(ExprNode* node)
{
    if (node->type != typeExprNode::div ||
        node->operands[0]->type != typeExprNode::div)
        return;

    ExprNode* innerDiv = node->operands[0];

    //Создаём новый узел "*" для объединения двух делителей в один
    ExprNode* mulNode = new ExprNode();
    mulNode->type = typeExprNode::mul;
    mulNode->operands.push_back(innerDiv->operands[1]);
    mulNode->operands.push_back(node->operands[1]);
    //Числитель внутреннего div становится новым числителем
    node->operands[0] = innerDiv->operands[0];
    //знаменателем теперь становится произведение двух делителей
    node->operands[1] = mulNode;

    innerDiv->operands.clear();
    delete innerDiv;
    //Дальнейшая нормализация
    transformTree(mulNode);
}

// Переносит потомков дочернего узла на уровень выше, если его тип
//совпадает с типом текущего узла 
void flattenAssociativeOperands(ExprNode* node)
{
    //Работаем только с коммутативными операции +  и *
    if (node->type != typeExprNode::plus && node->type != typeExprNode::mul)
        return;

    std::vector<ExprNode*> newChild;

    //Проходим по всем текущим операндам узла
    for (ExprNode* child : node->operands)
    {
        if (child->type == node->type)
        {
            //Если +
            if (node->type == typeExprNode::plus)
            {
                //Домножаем коэф потомков на текущий
                for (ExprNode* grandChild : child->operands)
                    grandChild->coefficient *= child->coefficient;
            }
            else
            {
                //Для умножения переносим
                node->coefficient *= child->coefficient;
            }

            //ПОднимаем узлы на уровень выше
            for (ExprNode* grandChild : child->operands)
                newChild.push_back(grandChild);

            //Удаляем п=лишнее
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

// Функция преобразования дерева в н-арное с упрощениями
void transformTree(ExprNode* node)
{
    if (!node) return;

    //Рекурсивный спуск
    for (ExprNode* child : node->operands)
        transformTree(child);

    //Шаги преобразования
    normalizeUnaryMinus(node);
    normalizeBinaryMinus(node);
    normalizeNestedDivision(node);
    flattenAssociativeOperands(node);

    //Вычислешие хэша
    computeHash(node);
}

// Упрощение степени 
void simplifyPow(ExprNode*& node)
{
    if (node->type != typeExprNode::pow)
        return;

    ExprNode* leftOp = node->operands[0];
    const ExprNode* rightOp = node->operands[1];
    
    //Раскрытие вложенной степени 
    if (leftOp->type == typeExprNode::pow &&
        leftOp->operands[1]->type == typeExprNode::con &&
        rightOp->type == typeExprNode::con)
    {
        ExprNode* base = leftOp->operands[0];
        ExprNode* innerExp = leftOp->operands[1];
        float     outerVal = rightOp->value;

        //Перемножаем показатели степеней
        innerExp->value *= outerVal;

        //Удаляем лишнее
        freeTree(node->operands[1]);   
        leftOp->operands.clear();
        delete leftOp;                

        node->operands[0] = base;
        node->operands[1] = innerExp;

        leftOp = node->operands[0];
        rightOp = node->operands[1];
    }

    //Вычисление констант, при условии, что оба оператора константы
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
        const ExprNode* child = node->operands[i];
        if ((child->type == typeExprNode::con && child->value == 0.0f) || child->coefficient == 0)
            //Вернуть правду
            return true;
    }
    //Иначе ложь
    return false;
}

// Вынос коэффициентов потомков
void mulCollectCoefficients(ExprNode* node)
{
    int totalCoef = node->coefficient;
    //Для всех потомков
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        //Перемножаем коэфициент
        totalCoef *= node->operands[i]->coefficient;
        //У потомков он становится = 1
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

    // Для всех операндов
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        //Если константа 
        ExprNode* child = node->operands[i];
        if (child->type == typeExprNode::con)
        {
            //Перемножаем
            constProduct *= child->value;
            hadConst = true;
            //Удаляем
            freeTree(child);
        }
        else
        {
            nonConst.push_back(child);
        }
    }
    node->operands = nonConst;

    //Если были константы и итог не 1
    if (hadConst && constProduct != 1.0f)
    {
        //Создаем новый узел константы
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
        //Считать равные подряд идущих деревьев
        while (i + count < (int)node->operands.size() &&
            areTreesEqual(node->operands[i], node->operands[i + count]))
        {
            ++count;
        }

        //Если есть равные
        if (count > 1)
        {
            //Создать узел степени с показателем, равным кол-ву этих деревьев
            ExprNode* expConst = new ExprNode();
            expConst->type = typeExprNode::con;
            expConst->value = (float)count;
            ExprNode* powNode = new ExprNode();
            powNode->type = typeExprNode::pow;
            powNode->operands.push_back(node->operands[i]);
            powNode->operands.push_back(expConst);

            //Удалить повторяющиеся
            for (int k = 1; k < count; ++k)
                freeTree(node->operands[i + k]);

            node->operands.erase(node->operands.begin() + i + 1, node->operands.begin() + i + count);
            node->operands[i] = powNode;
        }
        //Перейти к следующему 
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

    // Проверка константы
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

    //Если больше 1 константы или в узле нет переменных
    if (nonConstCount != 1 || node->operands.size() <= 1)
        //Завершить
        return;

    int intConst = (int)constProduct;
    //Если нецелое число
    if ((float)intConst != constProduct)
        //Завершить
        return;

    // Удаляем константные узлы
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        if (node->operands[i] != nonConstNode)
            freeTree(node->operands[i]);
    }
    node->operands.clear();

    // Поглощаем nonConstNode в текущий узел с коэффициентом
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
        //Необходим пересчет хэша, после переформирования узла
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

    //Для всех узлов
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        //Если число, то суммируем с учетом коэф и удаляем
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

    //Если константы были
    if (hadConst)
    {
        //Создаем итоговую
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
    //Для всех операндов
    for (int i = 0; i < (int)node->operands.size(); ++i)
    {
        ExprNode* child = node->operands[i];
        //Если число 0
        if ((child->type == typeExprNode::con && child->value == 0.0f) ||
            child->coefficient == 0)
            //Удалить
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
            //Если равны с учетом коэфициентов
            if (areTreesEqual(node->operands[i], node->operands[j], true))
            {
                //Сложить их коэфициента
                node->operands[i]->coefficient += node->operands[j]->coefficient;
                //Удалить второе дерево
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
    //Удаляли опернды только в + и *, значит только они могут быть пустыми
    if (node->type != typeExprNode::plus && node->type != typeExprNode::mul)
        return;

    typeExprNode originalType = node->type;

    //Если нет потомков
    if (node->operands.empty())
    {
        node->type = typeExprNode::con;
        //Если +, Значит 0 (тк нейтрален для сложения)
        if (originalType == typeExprNode::plus)
        {
            node->value = 0.0f;
        }
        //Иначе * и 1(тк нейтральна для *)
        else
        {
            node->value = 1.0f;
        }
        return;
    }

    //Если один потомок
    if (node->operands.size() == 1)
    {
        //Переносим его в текущий узел
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
    //Упрощения степени
    simplifyPow(node);
    //Упрощения деления
    simplifyDiv(node);
    //Упрощения умножения
    simplifyMul(node);
    //Упрощения сложения
    simplifyPlus(node);
    //Удаляение пустых узлов после преобразований 
    collapseNode(node);
    //Пересчет хэша, для повторных проходов
    computeHash(node);   

    // Если MUL с единственным операндом схлопнулся в операцию возведения
    // в степень, этот новый узел появился уже ПОСЛЕ того, 
    // как simplifyPow для него отработал выше. Нужен повторный вызов
    if (node->type == typeExprNode::pow)
        simplifyPow(node);
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

    // Проверка возможности записи в выходной файл
    {
        std::ofstream testFile(outputPath);
        if (!testFile.is_open())
        {
            std::vector<Error> error;
            error.push_back(Error(ErrorType::OutFileCreateFail, 0, outputPath));
            std::cout << formatAllErrors(error);
            freeTree(root);
            return 1;
        }
    }

    // Фиксация исходного дерева 
    generateDotFile(root, outputPath , false);

    // Преобразования
    moveTermsToLeft(root);
    transformTree(root);
    sortTree(root);
    simplifyTree(root);

    // Фиксация результирующего дерева 
    generateDotFile(root, outputPath, true);

    freeTree(root);

    std::cout << "Готово. Результат записан в: " << outputPath << "\n";
    return 0;
}