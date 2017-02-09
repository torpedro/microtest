#ifndef __MICROTEST_H__
#define __MICROTEST_H__

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

////////////////
// Assertions //
////////////////

#define ASSERT(cond) ASSERT_TRUE(cond);

#define ASSERT_TRUE(cond) if (!(cond)) throw mt::AssertFailedException(#cond);

#define ASSERT_FALSE(cond) if (cond) throw mt::AssertFailedException(#cond);

#define ASSERT_NULL(value) ASSERT_TRUE(value == NULL);

#define ASSERT_NOTNULL(value) ASSERT_TRUE(value != NULL);

#define ASSERT_STREQ(a, b) \
  if (std::string(a).compare(std::string(b)) != 0) throw mt::AssertFailedException(#a " == " #b)

#define ASSERT_EQ(a, b) \
  if (a != b) { \
    std::cout << "Actual values: " << a << " != " << b << std::endl; \
  } \
  ASSERT(a == b);


////////////////
// Unit Tests //
////////////////

#define TEST(name) \
  void name();\
  namespace mt {\
    bool __##name = TestsManager::AddTest(name, #name);\
  }\
  void name()


///////////////
// Framework //
///////////////

namespace mt {

  inline const char* green() {
    return "\033[0;32m";
  }

  inline const char* red() {
    return "\033[1;31m";
  }

  inline const char* def() {
    return "\033[0m";
  }

  inline void printRunning(const char* message, FILE* file = stdout) {
    fprintf(file, "%s{ running}%s %s\n", green(), def(), message);
  }

  inline void printOk(const char* message, FILE* file = stdout) {
    fprintf(file, "%s{      ok}%s %s\n", green(), def(), message);
  }

  inline void printFailed(const char* message, FILE* file = stdout) {
    fprintf(file, "%s{  failed} %s%s\n", red(), message, def());
  }

  // Exception that is thrown when an assertion fails.
  class AssertFailedException : public std::exception {
   public:
    AssertFailedException(std::string description) :
      std::exception(),
      description_(description) {};

    virtual const char* what() const throw() {
      return description_.c_str();
    }

   protected:
    std::string description_;
  };

  class TestsManager {
    // Note: static initialization fiasco
    // http://www.parashift.com/c++-faq-lite/static-init-order.html
    // http://www.parashift.com/c++-faq-lite/static-init-order-on-first-use.html
   public:
    struct Test {
      const char* name;
      void (*fn)(void);
    };

    static std::vector<Test>& tests() {
      static std::vector<Test> tests_;
      return tests_;
    }

    // Adds a new test to the current set of tests.
    // Returns false if a test with the same name already exists.
    inline static bool AddTest(void (*fn)(void), const char* name) {
      tests().push_back({ name, fn });
      return true;
    }

    // Run all tests that are registered.
    // Returns the number of tests that failed.
    inline static size_t RunAllTests(FILE* file = stdout) {
      size_t num_failed = 0;

      for (const Test& test : tests()) {
        // Run the test.
        // If an AsserFailedException is thrown, the test has failed.
        try {
          printRunning(test.name, file);

          (*test.fn)();

          printOk(test.name, file);

        } catch (AssertFailedException& e) {
          printFailed(test.name, file);
          fprintf(file, "\t%sAssertion failed: %s%s\n", red(), e.what(), def());
          ++num_failed;
        }
      }

      return num_failed;
    }
  };
}


#define TEST_MAIN() \
  int main() {\
    size_t num_failed = mt::TestsManager::RunAllTests(stdout);\
    if (num_failed == 0) {\
      fprintf(stdout, "%s{ summary} All tests succeeded!%s\n", mt::green(), mt::def());\
      return 0;\
    } else {\
      double percentage = 100.0 * num_failed / mt::TestsManager::tests().size();\
      fprintf(stderr, "%s{ summary} %lu tests failed (%.2f%%)%s\n", mt::red(), num_failed, percentage, mt::def());\
      return -1;\
    }\
  }

#endif // __MICROTEST_H__