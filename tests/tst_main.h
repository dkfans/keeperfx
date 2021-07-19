//
// Created by gamer on 7/13/21.
//

#ifndef GIT_TST_MAIN_H
#define GIT_TST_MAIN_H

#include <CUnit.h>

#include <globals.h>

class TestRegistryWrapper
{

public:
    TestRegistryWrapper(const char *file, const char *fn_name, void (*test_fn)())
    {
        if (!CU_registry_initialized())
        {
            CU_initialize_registry();
        }
        CU_pSuite suite = CU_get_suite(file);
        if (suite == NULL)
        {
            suite = CU_add_suite(file, NULL, NULL);
        }
        CU_add_test(suite, fn_name, test_fn);
    }
};

#define TEST_OBJ_NAME(fn_name) fn_name ##__LINE__

#define ADD_TEST(X) \
   static void X(); \
   static TestRegistryWrapper TEST_OBJ_NAME(X) (__FILE__, #X, &X); \
   static void X()


/****************/
#ifdef __cplusplus
extern "C" {
#endif

// Should be called at start of a test
void tst_init_map();

// Parse map of slabs into a game
void tst_parse_map(const char *data,
                   MapSlabCoord *src_x, MapSlabCoord *src_y, MapSlabCoord *max_x, MapSlabCoord *max_y);

// Returns a textual slab map
const char *tst_print_map(MapSlabCoord max_x, MapSlabCoord max_y, const char* (*test_fn)(MapSlabCoord x, MapSlabCoord y));

// Returns a textual like-map with color
const char *tst_print_colored_map(MapSlabCoord max_x, MapSlabCoord max_y, int (*test_fn)(MapSlabCoord x, MapSlabCoord y));

const char* tst_slab_to_symbol(MapSlabCoord x, MapSlabCoord y);

#ifdef __cplusplus
}
#endif
#endif //GIT_TST_MAIN_H
