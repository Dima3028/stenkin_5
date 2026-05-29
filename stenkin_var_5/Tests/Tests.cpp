#include "pch.h"
#include "CppUnitTest.h"
#include "../stenkin_var_5/Header.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{


    /**
    * @brief Вспомогательная функция: строит дерево из ОПЗ,
    *        сохраняет .dot до упрощения, применяет simplifyTree,
    *        сохраняет .dot после, возвращает корень.
    */
    static ExprNode* buildAndSimplify(const std::string& input,
        const std::string& testName)
    {
        ExprNode* root = nullptr;
        std::vector<Error> errors;
        buildTree(input, root, errors);

        generateDotFile(root, "input_simplify_" + testName + ".dot");
        simplifyTree(root);
        generateDotFile(root, "fin_simplify_" + testName + ".dot");
        return root;
    }


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


    TEST_CLASS(Tests_simplifyTree)
    {
    public:

        TEST_METHOD(testNullptr)
        {
            int counterBefore = ExprNode::globalIdCounter;
            ExprNode* ptr = nullptr;
            simplifyTree(ptr);
            Assert::AreEqual(counterBefore, ExprNode::globalIdCounter,
                L"При nullptr новые узлы создаваться не должны.");
        }

        TEST_METHOD(testNestedPow)
        {
            const std::string name = "testNestedPow";
            ExprNode* root = buildAndSimplify("a 2 ^ 3 ^ 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::pow, { makeVar("a", 1), makeCon(6.0f, 1) }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");
            computeHash(root);
            Assert::IsTrue(areTreesEqual(root, expected), L"(a^2)^3 => a^6.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testPowConstants)
        {
            const std::string name = "testPowConstants";
            ExprNode* root = buildAndSimplify("2 3 ^ 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(8.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected), L"2^3 => 8.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testDivEqualNodes)
        {
            const std::string name = "testDivEqualNodes";
            ExprNode* root = buildAndSimplify("a b + a b + / 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(1.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected), L"(a+b)/(a+b) => 1.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testDivZeroNumerator)
        {
            const std::string name = "testDivZeroNumerator";
            ExprNode* root = buildAndSimplify("0 a / 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(0.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected), L"0/a => 0.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testDivConstants)
        {
            const std::string name = "testDivConstants";
            ExprNode* root = buildAndSimplify("10 2 / 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(5.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected), L"10/2 => 5.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testMulByZero)
        {
            const std::string name = "testMulByZero";
            ExprNode* root = buildAndSimplify("a b * 0 * 0 =", name);

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(0.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected), L"a*b*0 => 0.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testMulConstantsMerge)
        {
            const std::string name = "testMulConstantsMerge";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a 2 * 3 * 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");
            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeVar("a",  6),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"a*2*3 должно стать mul(6, a).");

            freeTree(expected);
            freeTree(root);
        }


        TEST_METHOD(testMulGroupIntoPow)
        {
            const std::string name = "testMulGroupIntoPow";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a a * a * b * 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::mul,
                    {
                        makeOp(typeExprNode::pow,
                        {
                            makeVar("a",  1),
                            makeCon(3.0f, 1)
                        }, 1),
                        makeVar("b", 1)
                    }, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"a*a*a*b должно стать mul(pow(a,3), b).");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testPlusConstantsMerge)
        {
            const std::string name = "testPlusConstantsMerge";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a 5 + 3 + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeVar("a",   1),
                        makeCon(8.0f,  1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"a+5+3 должно стать plus(a, con8).");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testPlusRemoveZeros)
        {
            const std::string name = "testPlusRemoveZeros";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a 0 + b + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeVar("a", 1),
                        makeVar("b", 1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"a+0+b должно стать plus(a, b).");
            freeTree(expected);
            freeTree(root);
        }


        TEST_METHOD(testPlusLikeTerms)
        {
            const std::string name = "testPlusLikeTerms";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a a + a + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expr = root->operands[0];
            Assert::IsTrue(expr->type == typeExprNode::var, L"Результат — var.");
            Assert::AreEqual(std::string("a"), expr->varName);
            Assert::AreEqual(3, expr->coefficient, L"Коэффициент — 3.");
            freeTree(root);
        }

        TEST_METHOD(testPlusCancellation)
        {
            const std::string name = "testPlusCancellation";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a a _- + b + b _- + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expr = root->operands[0];
            Assert::IsTrue(expr->type == typeExprNode::con, L"Результат — con.");
            Assert::AreEqual(0.0f, expr->value, L"Значение — 0.");
            freeTree(root);
        }

        TEST_METHOD(testFullSimplification)
        {
            const std::string name = "testFullSimplification";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a a + 3 3 / * 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expr = root->operands[0];
            Assert::IsTrue(expr->type == typeExprNode::var, L"Результат — var(a).");
            Assert::AreEqual(std::string("a"), expr->varName);
            Assert::AreEqual(2, expr->coefficient, L"Коэффициент — 2.");
            freeTree(root);
        }

        TEST_METHOD(testMulRemoveUnitConstants)
        {
            const std::string name = "testMulRemoveUnitConstants";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a 1 * 1 * 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeVar("a", 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"a*1*1 должно стать var 'a' с coefficient=1.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testMulGroupWithCoefficients)
        {
            const std::string name = "testMulGroupWithCoefficients";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("2 a * 3 a * * 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");
            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::pow,
                    {
                        makeVar("a",    1),
                        makeCon(2.0f,   1)
                    }, 6),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"(2*a)*(3*a) должно стать pow(a,2) с coefficient=6.");
            freeTree(expected);
            freeTree(root);
        }



        TEST_METHOD(testPlusOnlyZeros)
        {
            const std::string name = "testPlusOnlyZeros";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("0 0 + 0 + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(0.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"0+0+0 должно стать con 0.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testPlusLikeComplexTerms)
        {
            const std::string name = "testPlusLikeComplexTerms";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a a * a a * + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::pow,
                    {
                        makeVar("a",  1),
                        makeCon(2.0f, 1)
                    }, 2),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"(a*a)+(a*a) должно стать pow(a,2) с coefficient=2.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testPlusSingleOperand)
        {
            const std::string name = "testPlusSingleOperand";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a 0 + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeVar("a", 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"a+0 должно схлопнуться в var 'a'.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testComplex1)
        {
            const std::string name = "testComplex1";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("2 a * a * 3 a * a * + 5 2 * + 0 + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());
            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeOp(typeExprNode::plus,
                    {
                        makeOp(typeExprNode::pow,
                        {
                            makeVar("a",  1),
                            makeCon(2.0f, 1)
                        }, 5),
                        makeCon(10.0f, 1)
                    }),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"(2*a*a)+(3*a*a)+(5*2)+0 должно стать plus(pow(a,2)(coef=5), con10).");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testComplex2)
        {
            const std::string name = "testComplex2";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("a b + a b + / 5 * 5 _- + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());
            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");
            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(0.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"((a+b)/(a+b))*5 + (-5) должно стать con 0.");
            freeTree(expected);
            freeTree(root);
        }

        TEST_METHOD(testComplex3)
        {
            const std::string name = "testComplex3";
            ExprNode* root = nullptr;
            std::vector<Error> errors;
            buildTree("0 a 2 3 ^ ^ / b b * b * b 3 ^ / + 0 =", root, errors);
            Assert::AreEqual(size_t(0), errors.size());

            generateDotFile(root, "input_simplify_" + name + ".dot");
            transformTree(root);
            simplifyTree(root);
            generateDotFile(root, "fin_simplify_" + name + ".dot");

            ExprNode* expected = makeOp(typeExprNode::eq,
                {
                    makeCon(1.0f, 1),
                    makeCon(0.0f, 1)
                });
            computeHash(expected);
            generateDotFile(expected, "expected_simplify_" + name + ".dot");

            Assert::IsTrue(areTreesEqual(root, expected),
                L"0/(a^8) + b^3/b^3 должно стать con 1.");
            freeTree(expected);
            freeTree(root);
        }

    };
}