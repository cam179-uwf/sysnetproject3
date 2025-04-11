#ifndef TESTS_H
#define TESTS_H

#include <string>
#include <iostream>

#define ASSERT(a, b) __assert_eq(a, b, __FILE__, __LINE__)
#define ASSERT_M(a, b, failedMessage) __assert_eq(a, b, failedMessage, __FILE__, __LINE__)
#define TESTS() void __test_main(int argc, char **argv)

template <typename T>
void __assert_eq(T a, T b, std::string fileName, int line)
{
    if (a != b)
    {
        std::cout << "Expected <" << b << "> but got <" << a << "> at " << fileName << ":" << line << std::endl;
        exit(1);
    }
}

template <typename T>
void __assert_eq(T a, T b, std::string failedMsg, std::string fileName, int line)
{
    if (a != b)
    {
        std::cout << "Failed Message: " << failedMsg << std::endl;
        std::cout << "Expected <" << b << "> but got <" << a << "> at " << fileName << ":" << line << std::endl;
        exit(1);
    }
}

void __test_main(int argc, char **argv);

int main(int argc, char **argv)
{
    __test_main(argc, argv);
    std::cout << "Successfully ran all tests!" << std::endl;
    return EXIT_SUCCESS;
}

#endif // TESTS_H