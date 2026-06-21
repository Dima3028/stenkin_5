/**
 * @mainpage Программа переброски слагаемых и множителей
 *
 *
 * @section sec_purpose Назначение
 * Программа принимает математическое равенство или неравенство в виде
 * обратной польской записи (ОПЗ) и переносит все слагаемые и множители
 * из правой части в левую, приравнивая правую часть к нулю.
 * После переноса выполняется упрощение и сортировка операндов по алфавиту.
 *
 * @section sec_limits Ограничения
 * - Переменные: одна строчная латинская буква (a–z)
 * - Константы: целые числа в диапазоне [-1000; 1000]
 * - Операции: +, -, *, /, ^, унарный минус (_-)
 * - Знаки сравнения: =, <, >, <=, >= (ровно один на выражение)
 * - Показатель степени: натуральное число не более 100
 * - Максимум 100 операций в выражении
 * - Максимальная длина строки: 500 символов
 *
 * @section sec_examples Примеры преобразований
 * | Входное выражение | Результат |
 * |---|---|
 * | 4 + a*b - 2 = a*b - 3 | 5 = 0 |
 * | 6 + a^3 * a^3 > 7 | -1 + a^6 > 0 |
 * | 9 + a < -a | 9 + 2*a < 0 |
 * | a*b - a*b = 0 | 0 = 0 |
 */

#pragma once
#include <string>
#include <vector>
#include <cstddef>

/**
 * @enum typeExprNode
 * @brief Перечисление типов узлов дерева математических выражений.
 */
enum class typeExprNode 
{
    con,        ///< Константа. Целое число в диапазоне от -1000 до 1000. 
    var,        ///< Переменная. Одиночная строчная латинская буква. 
    plus,       ///< Операция сложения (+). 
    minus,      ///< Операция бинарного вычитания (-). 
    u_minus,    ///< Операция унарного минуса (_-). 
    mul,        ///< Операция умножения (*). 
    div,        ///< Операция деления (/). 
    pow,        ///< Операция возведения в степень (^). 
    eq,         ///< Знак равенства (=). 
    lt,         ///< Знак строго меньше (<). 
    gt,         ///< Знак строго больше (>). 
    le,         ///< Знак меньше или равно (<=). 
    ge          ///< Знак больше или равно (>=). 
};

/**
 * @enum ErrorType
 * @brief Перечисление типов кодов ошибок, возникающих при обработке выражений.
 */
enum class ErrorType 
{
    NoError,              ///< Ошибок нет. Значение по умолчанию. 
    FileNotExist,         ///< Указанный входной файл не существует или к нему нет доступа. 
    OutFileCreateFail,    ///< Невозможно создать указанный выходной файл. 
    FileEmpty,            ///< Указанный входной файл пуст. 
    MultipleRelations,    ///< Ошибка парсинга: во входных данных больше одного знака равенства или неравенства. 
    InvalidVarName,       ///< Неверное написание имени переменной. 
    OutOfRange,           ///< Введенные числа (константы) выходят за допустимый диапазон [-1000; 1000]. 
    UnsupportedChar,      ///< Встретилась неподдерживаемая последовательность символов. 
    NotEnoughOperands,    ///< Недостаточное количество операндов для операции.
    NotEnoughOperators,   ///< Недостаточное количество операций (лишние операнды в стеке). 
    InvalidOperands,      ///< Недопустимые операнды в выражении (например, переменная в показателе степени). 
    DivideByZero,         ///< Ошибка вычисления или парсинга: попытка деления на ноль. 
    NegativePower,        ///< Ошибка: показатель степени является отрицательным числом. 
    FractionalPower,      ///< Ошибка: показатель степени является дробным числом. 
    MissingRelation       ///< Ошибка парсинга: во входных данных отсутствует знак равенства или неравенства. 
};

/**
 * @class Error
 * @brief Класс, содержащий информацию о возникшей в программе ошибке.
 */
class Error 
{
public:
    ErrorType type;           ///< Тип возникшей ошибки. 
    int position;             ///< Позиция ошибочной лексемы или символа во входной строке. 
    std::string invalidLexem; ///< Строковое представление лексемы, вызвавшей ошибку. 

    /**
     * @brief Конструктор класса Error с параметрами по умолчанию.
     * @param[in] t Тип ошибки.
     * @param[in] pos Позиция ошибки в строке.
     * @param[in] lexem Невалидная лексема.
     */
    explicit Error(ErrorType t = ErrorType::NoError, int pos = -1, const std::string& lexem = "");

    /**
     * @brief Метод, генерирующий и возвращающий сообщение с подробным описанием ошибки.
     * @return Текстовое описание ошибки для вывода пользователю.
     */
    std::string generateErrorMessage() const;
};

/**
 * @brief Функция для формирования единого сообщения со всеми ошибками.
 * @param[in] errorList Вектор, содержащий все найденные ошибки.
 * @return Строка с полным отчетом об ошибках.
 */
std::string formatAllErrors(const std::vector<Error>& errorList);

/**
 * @class ExprNode
 * @brief Класс, представляющий собой узел дерева математических выражений.
 */
class ExprNode 
{
public:
    static int globalIdCounter; ///< Статический счетчик для генерации уникальных идентификаторов узлов.

    int id;                           ///< Уникальный идентификатор конкретного узла. 
    typeExprNode type;                ///< Тип текущего узла дерева. 
    std::string varName;              ///< Строковое значение операнда или символа операции. 
    float value;                      ///< Числовое значение операнда (для констант). 
    int coefficient;                  ///< Числовой множитель узла (по умолчанию равен 1). 
    size_t hashValue;                 ///< Уникальный цифровой отпечаток поддерева. 
    std::vector<ExprNode*> operands;  ///< Контейнер указателей на дочерние узлы (потомки). 

    /**
     * @brief Конструктор по умолчанию класса ExprNode.
     * Автоматически инкрементирует глобальный счетчик и присваивает уникальный id.
     */
    ExprNode();

    /**
     * @brief Перегруженный оператор "меньше" для сортировки узлов дерева.
     * @param[in] other Константная ссылка на другой узел для сравнения.
     * @return true, если текущий узел меньше по приоритету или значению, иначе false.
     */
    bool operator<(const ExprNode& other) const;
};


/**
*@brief Рекурсивно проверяет два абстрактных синтаксических дерева на полное совпадение.
* Проверка включает в себя сравнение типов узлов, их значений, имен переменных,
* коэффициентов, а также рекурсивное сравнение всех дочерних операндов.
* @param[in] node1 Константный указатель на корень первого дерева для сравнения.
* @param[in] node2 Константный указатель на корень второго дерева для сравнения.
* @param[in] ignoreCoef Флаг, позволяющий игнорировать коэффициенты узлов при сравнении (по умолчанию false).
* @return true, если структура и содержимое деревьев полностью идентичны, иначе false.
*/
bool areTreesEqual(const ExprNode* node1, const ExprNode* node2, bool ignoreCoef = false);

/**
 * @brief Рекурсивно очищает динамическую память, выделенную под узлы дерева.
 * Проходит по всем дочерним элементам переданного узла и освобождает их.
 * @param[in] node Указатель на корень удаляемого поддерева.
 */
void freeTree(ExprNode* node);

/**
 * @brief Генерирует текстовый файл в формате DOT для визуализации структуры дерева.
 * @param[in] root Константный указатель на корень дерева, которое необходимо визуализировать.
 * @param[in] filename Строка с именем выходного файла (например, "expression_tree.dot").
 * @param[in] append Флаг дозаписи в файл
 */
void generateDotFile(const ExprNode* root, const std::string& filename, bool append = false);

/**
 * @brief Строит бинарное дерево на основе строки в обратной польской записи.
 * @param[in] rpnString Константная ссылка на строку с входным выражением.
 * @param[out] root Ссылка на указатель корневого узла дерева. Функция выделит память и запишет сюда корень.
 * @param[out] errors Ссылка на вектор ошибок, куда будут записываться все проблемы парсинга.
 * @return true, если дерево успешно построено без ошибок, иначе false.
 */
bool buildTree(const std::string& rpnString, ExprNode*& root, std::vector<Error>& errors);

/**
 * @brief Разбивает строку с математическим выражением в обратной польской записи (ОПЗ) на отдельные лексемы.
 * @param[in] rpnString Константная ссылка на входную строку.
 * @return Вектор строк, где каждая строка представляет собой отдельную лексему (операнд или операцию).
 */
std::vector<std::string> tokenizeRPN(const std::string& rpnString);

/**
 * @brief Выполняет структурные преобразования бинарного дерева в n-арное.
 * @param[in] node Указатель на текущий узел дерева, с которого начинается трансформация.
 */
void transformTree(ExprNode* node);

/**
 * @brief Упрощает математическое дерево.
 * @param[in,out] node Ссылка на указатель текущего узла. Передается по ссылке, так как узел может быть заменен или удален в процессе оптимизации.
 */
void simplifyTree(ExprNode*& node);

/**
 * @brief Вычисляет хэш-значение для узла дерева и всех его потомков.
 * Хэш зависит от типа узла, его значения (для констант) или имени (для переменных),
 * а также от хэшей всех дочерних узлов. Для коммутативных операций (PLUS, MUL)
 * хэши потомков суммируются, что обеспечивает инвариантность к порядку операндов.
 * Для остальных операций применяется побитовое смешивание (XOR + сдвиги).
 * @param[in,out] node Указатель на текущий узел дерева. Если nullptr — функция завершается без действий.
 */
void computeHash(ExprNode* node);

/**
 * @brief Сортирует операнды коммутативных операций (сложение, умножение) для приведения к стандартному виду.
 * @param[in,out] node Указатель на текущий узел дерева.
 */
void sortTree(ExprNode* node);

/**
 * @brief Раскрывает вложенные степени и вычисляет степени с константами.
 * @param[in,out] node Ссылка на указатель текущего узла.
 */
void simplifyPow(ExprNode*& node);

/**
 * @brief Упрощает операцию деления.
 * @param[in,out] node Ссылка на указатель текущего узла.
 */
void simplifyDiv(ExprNode*& node);

/**
 * @brief Проверяет наличие нуля среди операндов умножения.
 * @param[in] node Указатель на узел операции умножения.
 * @return true, если среди множителей есть 0, иначе false.
 */
bool mulHasZero(ExprNode* node);

/**
 * @brief Выносит и перемножает коэффициенты всех потомков в родительский узел умножения.
 * @param[in,out] node Указатель на узел операции умножения.
 */
void mulCollectCoefficients(ExprNode* node);

/**
 * @brief Перемножает все константные операнды внутри узла умножения и оставляет один результирующий.
 * @param[in,out] node Указатель на узел операции умножения.
 */
void mulMergeConstants(ExprNode* node);

/**
 * @brief Сворачивает идущие подряд одинаковые множители в операцию возведения в степень.
 * @param[in,out] node Указатель на узел операции умножения.
 */
void mulGroupIntoPow(ExprNode* node);

/**
 * @brief Поглощает оставшуюся константу в произведении, умножая на нее коэффициент единственного неконстантного узла.
 * @param[in,out] node Ссылка на указатель текущего узла.
 */
void mulAbsorbConstantIntoCoef(ExprNode*& node);

/**
 * @brief Выполняет комплексное упрощение для узла операции умножения.
 * @param[in,out] node Ссылка на указатель текущего узла.
 */
void simplifyMul(ExprNode*& node);

/**
 * @brief Суммирует все константные операнды внутри узла сложения и оставляет один результирующий.
 * @param[in,out] node Указатель на узел операции сложения.
 */
void plusMergeConstants(ExprNode* node);

/**
 * @brief Удаляет из узла сложения операнды, являющиеся нулями.
 * @param node Указатель на узел операции сложения.
 */
void plusRemoveZeros(ExprNode* node);

/**
 * @brief Приводит подобные слагаемые в узле сложения (складывает их коэффициенты).
 * @param[in,out] node Указатель на узел операции сложения.
 */
void plusCombineLikeTerms(ExprNode* node);

/**
 * @brief Выполняет комплексное упрощение для узла операции сложения.
 * @param[in,out] node Указатель на текущий узел.
 */
void simplifyPlus(ExprNode* node);

/**
 * @brief Схлопывает узел сложения или умножения, если он пуст или содержит только один операнд.
 * @param[in,out] node Ссылка на указатель текущего узла.
 */
void collapseNode(ExprNode*& node);

/**
 * @brief Переносит все элементы из правой части равенства (или неравенства) в левую, приравнивая правую часть к нулю.
 * Трансформирует выражение вида A = B в A + (-B) = 0. Создает новый узел сложения в левой ветви,
 * куда помещает левую часть и правую часть с инвертированным.
 * Правая ветвь заменяется на узел-константу со значением 0.
 * @param[in,out] root Ссылка на указатель корневого узла.
 */
void moveTermsToLeft(ExprNode*& root);

/**
 * @brief Переводит тип операции в его строковое представление.
 * @param[in] type Тип узла 
 * @return Строка с символом операции 
 */
std::string opToString(typeExprNode type);

/**
 * @brief Проверяет, является ли строка именем переменной.
 * @param[in] token Проверяемая лексема.
 * @return true, если token — корректное имя переменной.
 */
bool isVar(const std::string& token);

/**
 * @brief Проверяет, является ли строка целочисленной константой.
 * @param[in]  token Проверяемая лексема.
 * @param[out] val   Числовое значение константы, если token оказался числом.
 * @return true, если token — целое число.
 */
bool isCon(const std::string& token, int& val);

/**
 * @brief Проверяет, что длина входной строки не превышает лимит в 500 символов.
 * @param[in]  rpnString Входная строка с выражением.
 * @param[out] errors    Вектор ошибок.
 * @return true, если длина допустима.
 */
bool validateInputLength(const std::string& rpnString, std::vector<Error>& errors);

/**
 * @brief Проверяет, что последняя лексема выражения — знак сравнения.
 * @param[in]  tokens  Массив лексем.
 * @param[in]  rpnString Исходная строка.
 * @param[out] errors Вектор ошибок.
 */
void validateRelationPresence(const std::vector<std::string>& tokens,
    const std::string& rpnString, std::vector<Error>& errors);

/**
 * @brief Обрабатывает лексему-операнд при построении дерева.
 * Создаёт соответствующий узел и кладёт его в стек разбора.
 * @param[in]     token  Лексема операнда.
 * @param[in]     pos    Позиция лексемы во входной строке.
 * @param[in,out] st     Стек узлов разбора.
 * @param[out]    errors Вектор ошибок.
 * @return true, если операнд корректен и помещён в стек.
 */
bool handleOperandToken(const std::string& token, int pos,
    std::stack<ExprNode*>& st, std::vector<Error>& errors);

/**
 * @brief Обрабатывает лексему унарного минуса при построении дерева.
 * Берёт один операнд со стека и оборачивает его в узел типа u_minus.
 * @param[in]   token  Лексема.
 * @param[in]   pos   Позиция лексемы во входной строке.
 * @param[in,out] st    Стек узлов разбора.
 * @param[out]  errors Вектор ошибок.
 * @return true, если операция применена успешно.
 */
bool handleUnaryMinusToken(const std::string& token, int pos,
    std::stack<ExprNode*>& st, std::vector<Error>& errors);

/**
 * @brief Проверяет ограничения операции возведения в степень.
 * @param[in]  left   Левый операнд (база степени).
 * @param[in]  right  Правый операнд (показатель степени).
 * @param[in]  pos    Позиция лексемы "^" во входной строке.
 * @param[out] errors Вектор ошибок.
 * @return true, если операнды допустимы.
 */
bool validatePowerOperands(const ExprNode* left, const ExprNode* right,
    int pos, std::vector<Error>& errors);

/**
 * @brief Переводит строковый токен бинарной операции/знака сравнения в тип узла.
 * @param[in] token Лексема операции.
 * @return Соответствующее значение typeExprNode.
 */
typeExprNode tokenToNodeType(const std::string& token);

/**
 * @brief Считает встреченные знаки сравнения и проверяет ограничение, что не более одного.
 * @param[in] token    Текущая лексема.
 * @param[in] pos     Позиция лексемы во входной строке.
 * @param[in,out] relationCount Счётчик встреченных знаков сравнения.
 * @param[out]  errors     Вектор ошибок.
 * @return true, если ограничение не нарушено.
 */
bool checkRelationLimit(const std::string& token, int pos,
    int& relationCount, std::vector<Error>& errors);

/**
 * @brief Проверяет операнды бинарной операции в зависимости от её вида
 * @param[in] token Лексема операции.
 * @param[in] left  Левый операнд.
 * @param[in] right  Правый операнд.
 * @param[in] pos  Позиция лексемы во входной строке.
 * @param[out] errors Вектор ошибок.
 * @return true, если операнды допустимы.
 */
bool validateOperandsForOperator(const std::string& token, const ExprNode* left,
    const ExprNode* right, int pos, std::vector<Error>& errors);

/**
 * @brief Обрабатывает бинарную операцию или знак сравнения при построении дерева.
 * Снимает два операнда со стека, проверяет их и строит новый узел операции.
 * @param[in] token  Лексема операции.
 * @param[in] pos  Позиция лексемы во входной строке.
 * @param[in,out] st Стек узлов разбора.
 * @param[in,out] relationCount Счётчик встреченных знаков сравнения.
 * @param[out]  errors   Вектор ошибок.
 * @return true, если узел операции успешно построен.
 */
bool handleBinaryOperatorToken(const std::string& token, int pos,
    std::stack<ExprNode*>& st, int& relationCount, std::vector<Error>& errors);

/**
 * @brief Обрабатывает некорректную лексему.
 * @param[in] token Некорректная лексема.
 * @param[in] pos Позиция лексемы во входной строке.
 * @param[out] errors Вектор ошибок.
 */
void handleInvalidToken(const std::string& token, int pos, std::vector<Error>& errors);

/**
 * @brief Классифицирует одну лексему и делегирует её обработку нужной подфункции.
 * @param[in] token Текущая лексема.
 * @param[in] pos Позиция лексемы во входной строке.
 * @param[in,out] st  Стек узлов разбора.
 * @param[in,out] relationCount Счётчик встреченных знаков сравнения.
 * @param[in,out] opCount   Счётчик всех операций.
 * @param[out] errors Вектор ошибок.
 * @return true, если лексема обработана без ошибок.
 */
bool processToken(const std::string& token, int pos, std::stack<ExprNode*>& st,
    int& relationCount, int& opCount, std::vector<Error>& errors);

/**
 * @brief Приводит унарный минус к отрицательному коэффициенту узла.
 * @param[in,out] node Узел. 
 */
void normalizeUnaryMinus(ExprNode* node);

/**
 * @brief Приводит бинарный минус к + с отрицательным коэффициентом правого операнда.
 * @param[in,out] node Узел. 
 */
void normalizeBinaryMinus(ExprNode* node);

/**
 * @brief Приводит двойное деление к делению на произведение делителей.
 * @param[in,out] node Узел. 
 */
void normalizeNestedDivision(ExprNode* node);

/**
 * @brief Переносит потомков дочернего узла на уровень выше, если его тип совпадает с типом текущего узла.
 * @param[in,out] node Узел.
 */
void flattenAssociativeOperands(ExprNode* node);