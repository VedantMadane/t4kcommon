#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "SDL.h"
#include "t4k_common.h"
#include "test_public_functions.h"

void test_T4K_inRect(void)
{
    SDL_Rect rect;
    rect.x = 100; rect.y = 100; rect.w = 500; rect.h = 100;
    CU_ASSERT_EQUAL(T4K_inRect(rect, 100, 100), 1);
    CU_ASSERT_EQUAL(T4K_inRect(rect, 350, 150), 1);
    CU_ASSERT_EQUAL(T4K_inRect(rect, 600, 200), 1);
    CU_ASSERT_EQUAL(T4K_inRect(rect,  99,  99), 0);
    CU_ASSERT_EQUAL(T4K_inRect(rect, 601, 201), 0);
    CU_ASSERT_EQUAL(T4K_inRect(rect, -100, -100), 0);
}

void test_T4K_CheckFile(void)
{
    FILE* f;
    CU_ASSERT_EQUAL(T4K_CheckFile("unexistant_file"), 0);
    f = fopen("temp_file", "a");
    if (f == NULL) { perror("fopen"); return; }
    CU_ASSERT_EQUAL(T4K_CheckFile("temp_file"), 1);
    if (fclose(f) == EOF) perror("fclose");
    if (remove("temp_file") == -1) perror("remove");
}

void test_T4K_RemoveSlash(void)
{
    char  winpath[]  = "C:\\my\\windows\\path\\";
    char  unixpath[] = "/home/my/unix/path/";
    char* nopath     = NULL;
    T4K_RemoveSlash(nopath);
    CU_ASSERT_PTR_NULL(nopath);
    T4K_RemoveSlash(winpath);
    CU_ASSERT_STRING_EQUAL(winpath, "C:\\my\\windows\\path");
    T4K_RemoveSlash(unixpath);
    CU_ASSERT_STRING_EQUAL(unixpath, "/home/my/unix/path");
}

/*
 * T4K_SetRect(SDL_Rect* rect, const float* pos)
 * pos is a 4-element float array [x, y, w, h] as fractions of screen size.
 * Requires SDL to be initialised; suite_init creates a 320x240 surface and
 * calling T4K_GetScreen() primes t4k's internal screen pointer via its own
 * SDL_GetVideoSurface() check.
 * NOTE: local variable is named 'surf', not 'screen', to avoid shadowing
 *       the extern SDL_Surface* screen declared in t4k_common.h.
 */
void test_T4K_SetRect(void)
{
    SDL_Surface* surf = T4K_GetScreen();
    SDL_Rect r;
    float pos[4];

    /* Normal fractions */
    pos[0] = 0.1f; pos[1] = 0.2f; pos[2] = 0.5f; pos[3] = 0.25f;
    T4K_SetRect(&r, pos);
    CU_ASSERT_EQUAL(r.x, (int)(0.1f * surf->w));
    CU_ASSERT_EQUAL(r.y, (int)(0.2f * surf->h));
    CU_ASSERT_EQUAL(r.w, (int)(0.5f * surf->w));
    CU_ASSERT_EQUAL(r.h, (int)(0.25f * surf->h));

    /* Zero fractions — all fields must be zero */
    pos[0] = 0.0f; pos[1] = 0.0f; pos[2] = 0.0f; pos[3] = 0.0f;
    T4K_SetRect(&r, pos);
    CU_ASSERT_EQUAL(r.x, 0);
    CU_ASSERT_EQUAL(r.y, 0);
    CU_ASSERT_EQUAL(r.w, 0);
    CU_ASSERT_EQUAL(r.h, 0);

    /* Full-screen fractions — w and h must match screen dimensions */
    pos[0] = 0.0f; pos[1] = 0.0f; pos[2] = 1.0f; pos[3] = 1.0f;
    T4K_SetRect(&r, pos);
    CU_ASSERT_EQUAL(r.w, surf->w);
    CU_ASSERT_EQUAL(r.h, surf->h);
}

void test_T4K_LineWrap(void)
{
    char out[MAX_LINES][MAX_LINEWIDTH];
    int  n;
    int  i;

    /* Single word, wide column — stays on one line */
    n = T4K_LineWrap("Hello", out, 9999, MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT_EQUAL(n, 1);
    CU_ASSERT_STRING_EQUAL(out[0], "Hello");

    /* Multi-word string, narrow column — must split into multiple lines */
    n = T4K_LineWrap("one two three four five", out, 5, MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT(n > 1);

    /* Every returned line must fit within MAX_LINEWIDTH */
    for (i = 0; i < n; i++)
        CU_ASSERT((int)strlen(out[i]) <= MAX_LINEWIDTH - 1);

    /* max_lines clamp — result must never exceed the requested limit */
    n = T4K_LineWrap("one two three four five six seven", out, 5, 2, MAX_LINEWIDTH);
    CU_ASSERT(n <= 2);
}

/*
 * NOTE: T4K_LineWrapInsBreaks calls strlen(input) on entry BEFORE it checks
 * for NULL input (upstream library bug).  The NULL-input case is therefore
 * not tested here to avoid a crash.  Only the NULL-output and normal/narrow
 * cases are exercised.
 */
void test_T4K_LineWrapInsBreaks(void)
{
    char output[MAX_LINES * MAX_LINEWIDTH];
    int  n;

    /* NULL output — must not crash, returns 0 */
    n = T4K_LineWrapInsBreaks("hello", NULL, 9999, MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT_EQUAL(n, 0);

    /* Single word, wide column — no newline inserted */
    n = T4K_LineWrapInsBreaks("Hello", output, 9999, MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT_EQUAL(n, 0);
    CU_ASSERT_STRING_EQUAL(output, "Hello");
    CU_ASSERT_PTR_NULL(strchr(output, '\n'));

    /* Multi-word string, narrow column — at least one newline inserted */
    n = T4K_LineWrapInsBreaks("one two three four five", output, 5,
                               MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT(n > 0);
    CU_ASSERT_PTR_NOT_NULL(strchr(output, '\n'));
}

void test_T4K_LineWrapList(void)
{
    char input[MAX_LINES][MAX_LINEWIDTH];
    char output[MAX_LINES][MAX_LINEWIDTH];

    /* Empty list — all output must be empty */
    memset(input,  0, sizeof(input));
    memset(output, 0, sizeof(output));
    T4K_LineWrapList(input, output, 9999, MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT_EQUAL(output[0][0], '\0');

    /* Single short string, wide column — copied unchanged */
    memset(input,  0, sizeof(input));
    memset(output, 0, sizeof(output));
    strncpy(input[0], "Hello", MAX_LINEWIDTH - 1);
    T4K_LineWrapList(input, output, 9999, MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT_STRING_EQUAL(output[0], "Hello");
    CU_ASSERT_EQUAL(output[1][0], '\0');

    /* Multi-word entry, narrow column — produces multiple output lines */
    memset(input,  0, sizeof(input));
    memset(output, 0, sizeof(output));
    strncpy(input[0], "one two three four five", MAX_LINEWIDTH - 1);
    T4K_LineWrapList(input, output, 5, MAX_LINES, MAX_LINEWIDTH);
    CU_ASSERT(strlen(output[0]) > 0);
    CU_ASSERT(strlen(output[1]) > 0);
}
