#include "pch.h"
#include "CppUnitTest.h"
#include "../stenkin_var_5/Header.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{

    /**
         * @brief Вспомогательная функция: строит дерево из ОПЗ,
         *        сохраняет .dot до трансформации, применяет transformTree,
         *        сохраняет .dot после, возвращает корень.
         */
    ExprNode* buildAndTransform(const std::string& input,
        const std::string& testName)
    {
        ExprNode* root = nullptr;
        std::vector<Error> errors;
        bool result = buildTree(input, root, errors);
        Assert::IsTrue(result, L"buildTree должен вернуть true.");
        Assert::AreEqual(size_t(0), errors.size(), L"Ошибок быть не должно.");

        generateDotFile(root, "input_" + testName + ".dot");
        transformTree(root);
        generateDotFile(root, "fin_" + testName + ".dot");

        return root;
    }

    /**
     * @brief Создаёт узел-переменную.
     */
    ExprNode* makeVar(const std::string& name, int coef = 1)
    {
        ExprNode* n = new ExprNode();
        n->type = typeExprNode::var;
        n->varName = name;
        n->coefficient = coef;
        return n;
    }

    /**
     * @brief Создаёт узел-константу.
     */
    ExprNode* makeCon(float val, int coef = 1)
    {
        ExprNode* n = new ExprNode();
        n->type = typeExprNode::con;
        n->value = val;
        n->coefficient = coef;
        return n;
    }

    /**
     * @brief Создаёт операционный узел с заданными потомками.
     */
    ExprNode* makeOp(typeExprNode op,
        std::vector<ExprNode*> children,
        int coef = 1)
    {
        ExprNode* n = new ExprNode();
        n->type = op;
        n->coefficient = coef;
        n->operands = children;
        return n;
    }

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




    TEST_CLASS(Tests_transformTree)
    {
    public:

        TEST_METHOD(testNullptr)
        {
            int counterBefore = ExprNode::globalIdCounter;

            transformTree(nullptr);

            int counterAfter = ExprNode::globalIdCounter;
            Assert::AreEqual(counterBefore, counterAfter,
                L"При передаче nullptr новые узлы создаваться не должны.");
        }

        TEST_METHOD(testUnaryMinusVar)
        {

            const std::string name = "testUnaryMinusVar";
            ExprNode* root = buildAndTransform("a _- 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeVar("a", -1),
                    makeCon(0.0f,  1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testUnaryMinusCon)
        {
            
            const std::string name = "testUnaryMinusCon";
            ExprNode* root = buildAndTransform("a 5 _- =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeVar("a",   1),
                    makeCon(5.0f, -1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testBinaryMinus)
        {

            const std::string name = "testBinaryMinus";
            ExprNode* root = buildAndTransform("a b - 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeVar("a",  1),
                        makeVar("b", -1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testNestedDivDiv)
        {
            
            const std::string name = "testNestedDivDiv";
            ExprNode* root = buildAndTransform("a b / c / 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::div,
                    {
                        makeVar("a", 1),
                        makeOp(typeExprNode::mul,
                        {
                          makeVar("b", 1),
                            makeVar("c", 1)  
                        })
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testFlattenPlusInPlus)
        {

            const std::string name = "testFlattenPlusInPlus";
            ExprNode* root = buildAndTransform("a b + c + 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeVar("a", 1),
                        makeVar("b", 1),
                        makeVar("c", 1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testFlattenMulInMul)
        {
            const std::string name = "testFlattenMulInMul";
            ExprNode* root = buildAndTransform("a b * c * 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::mul,
                    {
                        makeVar("a", 1),
                        makeVar("b", 1),
                        makeVar("c", 1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testNestedMinus)
        {
            const std::string name = "testNestedMinus";
            ExprNode* root = buildAndTransform("a b - c - 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeVar("a",  1),
                        makeVar("b", -1),
                        makeVar("c", -1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testDivMinusNumeratorNestedDiv)
        {

            const std::string name = "testDivMinusNumeratorNestedDiv";
            ExprNode* root = buildAndTransform("a b - c / d / 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::div,
                    {
                        makeOp(typeExprNode::plus,
                        {
                            makeVar("a",  1),
                            makeVar("b", -1)
                        }),
                        makeOp(typeExprNode::mul,
                        {
                            makeVar("c", 1),
                            makeVar("d", 1)
                        })
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Дерево после трансформации должно совпасть с эталоном.");

            freeTree(expected);
            freeTree(root);
        }


        TEST_METHOD(testComplexNestedDivAndMinus)
        {

            const std::string name = "testComplexNestedDivAndMinus";
            ExprNode* root = buildAndTransform(
                "a b - c / d e - / f / g h - i - * 0 =", name);
            // a -b)\c  *(d-e)

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::mul,
                    {
                        makeOp(typeExprNode::div,
                        {
                            makeOp(typeExprNode::plus,
                            {
                                makeVar("a",  1),
                                makeVar("b", -1)
                            }),
                            makeOp(typeExprNode::mul,
                            {
                                makeVar("c", 1),
                                makeOp(typeExprNode::plus,
                                {
                                    makeVar("d",  1),
                                    makeVar("e", -1)
                                }),
                                makeVar("f", 1)
                            })
                        }),
                        makeOp(typeExprNode::plus,
                        {
                            makeVar("g",  1),
                            makeVar("h", -1),
                            makeVar("i", -1)
                        })
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Комплексный тест: вложенные деления и минусы.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testComplexMulAndUnaryMinus)
        {
            const std::string name = "testComplexMulAndUnaryMinus";
            ExprNode* root = buildAndTransform(
                "a b _- * c * d * e f _- + g + h + i - - 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeOp(typeExprNode::mul,
                        {
                            makeVar("a",  1),
                            makeVar("b", -1),
                            makeVar("c",  1),
                            makeVar("d",  1)
                        }),
                        makeVar("e", -1),
                        makeVar("f",  1),
                        makeVar("g", -1),
                        makeVar("h", -1),
                        makeVar("i", 1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Комплексный тест: умножения и унарные минусы.");

            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testComplexUnaryBinaryDivMul)
        {
            const std::string name = "testComplexUnaryBinaryDivMul";
            ExprNode* root = buildAndTransform(
                "a b _- - c / d / e f g * h * - - 0 =", name);
            // a + b  /c/d ) - ( e - (f*g*h)) => a+b /(c*d) - e + (f*g*h)
            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeOp(typeExprNode::div,
                        {
                            makeOp(typeExprNode::plus,
                            {
                                makeVar("a", 1),
                                makeVar("b", 1)  
                            }),
                            makeOp(typeExprNode::mul,
                            {
                                makeVar("c", 1),
                                makeVar("d", 1)
                            })
                        }, 1),
                        makeVar("e", -1),
                        makeOp(typeExprNode::mul,
                        {
                            makeVar("f", 1),
                            makeVar("g", 1),
                            makeVar("h", 1)
                        }, 1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);

            Assert::IsTrue(areTreesEqual(root, expected),
                L"Комплексный тест: унарные минусы, бинарные минусы, деление и умножение.");

            freeTree(expected);
            freeTree(root);
        }
    };

}