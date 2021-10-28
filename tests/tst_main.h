//
// Created by gamer on 7/13/21.
//

#ifndef GIT_TST_MAIN_H
#define GIT_TST_MAIN_H

#include <CUnit.h>

class TestRegistryWrapper
{

public:
    TestRegistryWrapper(const char *fn_name, void (*test_fn)())
    {
        if (!CU_registry_initialized())
        {
            CU_initialize_registry();
        }
        CU_pSuite suite = CU_get_suite(__FILE__);
        if (suite == NULL)
        {
            suite = CU_add_suite(__FILE__, NULL, NULL);
        }
        CU_add_test(suite, fn_name, test_fn);
    }
};

#define TEST_OBJ_NAME(fn_name) fn_name ##__LINE__

#define ADD_TEST(X) \
   static void X(); \
   static TestRegistryWrapper TEST_OBJ_NAME(X) (#X, &X); \
   static void X()


#endif //GIT_TST_MAIN_H
