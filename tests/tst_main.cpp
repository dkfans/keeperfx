//
// Created by Sim on 7/11/21.
//
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <CUnit.h>
#include <Basic.h>

extern "C" {
}

int run_serv(int argc, char **argv);
int run_client(int argc, char **argv);

int SDL_main(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "serv"))
        {
            return run_serv(argc, argv);
        }
        else if (0 == strcmp(argv[i], "client"))
        {
            return run_client(argc, argv);
        }
    }
    //CU_initialize_registry is called on any test
    CU_basic_run_tests();
    CU_cleanup_registry();
    return 0;
}

