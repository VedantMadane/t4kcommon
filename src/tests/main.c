#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "CUnit/Basic.h"
#include "test_public_functions.h"

static int suite_init(void)
{
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    setenv("SDL_AUDIODRIVER", "dummy", 1);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "suite_init: SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }
    if (SDL_SetVideoMode(320, 240, 32, SDL_SWSURFACE) == NULL)
    {
        fprintf(stderr, "suite_init: SDL_SetVideoMode failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    return 0;
}

static int suite_cleanup(void)
{
    SDL_Quit();
    return 0;
}

static int add_test(CU_pSuite suite, CU_TestFunc fn, const char* name)
{
    if (CU_add_test(suite, name, fn) == NULL)
    {
        fprintf(stderr, "CU_add_test(%s): %s\n", name, CU_get_error_msg());
        return -1;
    }
    return 0;
}

#define ADD_TEST(suite, fn) \
    do { if (add_test((suite), (fn), #fn) != 0) goto cleanup; } while (0)

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    CU_pSuite suite = NULL;
    CU_ErrorCode err;
    int exit_code = EXIT_SUCCESS;

    err = CU_initialize_registry();
    if (err != CUE_SUCCESS)
    {
        fprintf(stderr, "CU_initialize_registry: memory allocation failed\n");
        return EXIT_FAILURE;
    }

    suite = CU_add_suite("T4K_common test suite", suite_init, suite_cleanup);
    if (suite == NULL)
    {
        fprintf(stderr, "CU_add_suite: %s\n", CU_get_error_msg());
        goto cleanup;
    }

    ADD_TEST(suite, test_T4K_inRect);
    ADD_TEST(suite, test_T4K_CheckFile);
    ADD_TEST(suite, test_T4K_RemoveSlash);
    ADD_TEST(suite, test_T4K_SetRect);

    err = CU_basic_run_suite(suite);
    if (err != CUE_SUCCESS)
    {
        fprintf(stderr, "CU_basic_run_suite: %s\n", CU_get_error_msg());
        exit_code = EXIT_FAILURE;
    }
    if (CU_get_number_of_failures() > 0)
        exit_code = EXIT_FAILURE;

cleanup:
    CU_cleanup_registry();
    return exit_code;
}
