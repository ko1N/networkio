#pragma once

#ifndef TESTS_H_
#define TESTS_H_

#include <functional>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#define TEST(condition)                                                        \
  {                                                                            \
    if (!(condition)) {                                                        \
      throw std::runtime_error("line " + std::to_string(__LINE__));            \
    }                                                                          \
  }

#define TEST_EQUAL(x, y)                                                       \
  {                                                                            \
    if ((x) != (y)) {                                                          \
      throw std::runtime_error("line " + std::to_string(__LINE__));            \
    }                                                                          \
  }

#define TEST_NOT_EQUAL(x, y)                                                   \
  {                                                                            \
    if ((x) == (y)) {                                                          \
      throw std::runtime_error("line " + std::to_string(__LINE__));            \
    }                                                                          \
  }

#define BEGIN_TESTS(project, name)                                             \
  int main(int argc, char *argv[]) {                                           \
    printf("%s: running test/%s:\n", project, name);                           \
    std::vector<std::pair<std::string, std::function<void()>>> tests;

#define ADD_TEST(name, func)                                                   \
  tests.emplace_back(std::make_pair(name, []() { func }));

#define END_TESTS(project)                                                     \
  for (size_t i = 0; i < tests.size(); i++) {                                  \
    auto &t = tests[i];                                                        \
    try {                                                                      \
      printf("%s: running test %d/%d: %s\n", project, (uint32_t)i + 1,         \
             (uint32_t)tests.size(), t.first.c_str());                         \
      t.second();                                                              \
    } catch (const std::exception &e) {                                        \
      printf("%s: test %d/%d %s failed at %s.\n", project, (uint32_t)i + 1,    \
             (uint32_t)tests.size(), t.first.c_str(), e.what());               \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  return 0;                                                                    \
  }

#endif
