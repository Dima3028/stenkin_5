#include "pch.h"
#include "CppUnitTest.h"
#include "../stenkin_var_5/Header.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
    TEST_CLASS(Tests_buildTree)
    {
    public:

        /**
         * @brief Вспомогательная функция для тестирования ошибок.
         */
        void runErrorTest(const std::string& input, ErrorType expectedErrorType)
        {
            ExprNode* root = nullptr;
            std::vector<Error> errors;

            bool result = buildTree(input, root, errors);

            Assert::IsFalse(result, L"Функция должна вернуть false при наличии ошибок.");
            Assert::IsTrue(errors.size() > 0, L"Вектор ошибок не должен быть пустым.");

            bool errorFound = false;
            for (int i = 0; i < errors.size(); ++i)
            {
                if (errors[i].type == expectedErrorType)
                {
                    errorFound = true;
                    break;
                }
            }

            Assert::IsTrue(errorFound, L"Ожидаемый тип ошибки не найден в векторе ошибок.");

            freeTree(root);
        }
        
        TEST_METHOD(testMinimalTree)
        {
            std::string input = "a 0 =";
            ExprNode* root = nullptr;
            std::vector<Error> errors;

            bool result = buildTree(input, root, errors);

            Assert::IsTrue(result, L"Функция должна вернуть true.");
            Assert::AreEqual(size_t(0), errors.size(), L"Вектор ошибок должен быть пустым.");
            Assert::IsNotNull(root, L"Дерево должно быть построено.");
            Assert::IsTrue(root->type == typeExprNode::eq, L"Корень должен быть знаком равенства.");

            if (root)
            {
                Assert::IsTrue(root->operands.size() == 2, L"У корня должно быть 2 потомка.");
                Assert::IsTrue(root->operands[0]->type == typeExprNode::var, L"Левый потомок должен быть переменной.");
                Assert::IsTrue(root->operands[1]->type == typeExprNode::con, L"Правый потомок должен быть константой.");
            }

            generateDotFile(root, "testMinimalTree.dot");
            freeTree(root);
        }

        TEST_METHOD(testComplexTree)
        {
            std::string input = "a 2 ^ b _- * 5 + 0 >";
            ExprNode* root = nullptr;
            std::vector<Error> errors;

            bool result = buildTree(input, root, errors);

            Assert::IsTrue(result, L"Функция должна вернуть true.");
            Assert::AreEqual(size_t(0), errors.size(), L"Вектор ошибок должен быть пустым.");
            Assert::IsNotNull(root, L"Дерево должно быть построено.");
            Assert::IsTrue(root->type == typeExprNode::gt, L"Корень должен быть знаком строго больше (>).");

            generateDotFile(root, "testComplexTree.dot");
            freeTree(root);
        }

        TEST_METHOD(testLargeTree)
        {
            std::string input = "a b + c c + - c b - c d + * ="; // ( a + b) - ( c+ c) = (c - b) *(c + d) 
            ExprNode* root = nullptr;
            std::vector<Error> errors;

            bool result = buildTree(input, root, errors);

            Assert::IsTrue(result, L"Функция должна вернуть true.");
            Assert::AreEqual(size_t(0), errors.size(), L"Вектор ошибок должен быть пустым.");
            Assert::IsNotNull(root, L"Дерево должно быть построено.");

            generateDotFile(root, "testLargeTree.dot");
            freeTree(root);
        }

        TEST_METHOD(testEmptyString)
        {
            std::string input = "";
            ExprNode* root = nullptr;
            std::vector<Error> errors;

            bool result = buildTree(input, root, errors);

            Assert::IsFalse(result);
            Assert::IsTrue(errors.size() > 0);

            bool errorFound = false;
            for (int i = 0; i < errors.size(); ++i)
            {
                if (errors[i].type == ErrorType::MissingRelation || errors[i].type == ErrorType::FileEmpty)
                {
                    errorFound = true;
                    break;
                }
            }
            Assert::IsTrue(errorFound, L"Ожидается ошибка FileEmpty или MissingRelation.");
            freeTree(root);
        }

        TEST_METHOD(testTwoOperationsInRow)
        {
            runErrorTest("a b + * 0 =", ErrorType::NotEnoughOperands);
        }

        TEST_METHOD(testMissingRelation)
        {
            runErrorTest("a b + c *", ErrorType::MissingRelation);
        }

        TEST_METHOD(testMultipleRelations)
        {
            runErrorTest("a b = c =", ErrorType::MultipleRelations);
        }

        TEST_METHOD(testInvalidVarName)
        {
            runErrorTest("ab c + 0 =", ErrorType::InvalidVarName);
        }

        TEST_METHOD(testInvalidConstant)
        {
            runErrorTest("a 1005 + 0 =", ErrorType::OutOfRange);
        }

        TEST_METHOD(testUnsupportedChar)
        {
            runErrorTest("a & b + 0 =", ErrorType::UnsupportedChar);
        }

        TEST_METHOD(testNotEnoughOperands)
        {
            runErrorTest("a + 0 =", ErrorType::NotEnoughOperands);
        }

        TEST_METHOD(testNotEnoughOperators)
        {
            runErrorTest("a b c + 0 =", ErrorType::NotEnoughOperators);
        }

        TEST_METHOD(testInvalidOperands)
        {
            runErrorTest("a b + 2 ^ 0 =", ErrorType::InvalidOperands);
        }

        TEST_METHOD(testDivideByZero)
        {
            runErrorTest("a 0 / 0 =", ErrorType::DivideByZero);
        }

        TEST_METHOD(testNegativePower)
        {
            runErrorTest("a -2 ^ 0 =", ErrorType::NegativePower);
        }

        TEST_METHOD(testVarInPower)
        {
            runErrorTest("a b ^ 0 =", ErrorType::InvalidOperands);
        }
        
        
    };
}