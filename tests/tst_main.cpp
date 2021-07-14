//
// Created by Sim on 7/11/21.
//
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <CUnit.h>
#include <Basic.h>

extern "C" {
}

int SDL_main(int argc, char **argv)
{
    //CU_initialize_registry is called on any test
    CU_basic_run_tests();
    CU_cleanup_registry();
    return 0;
}

