// 单元测试：针对 int get_user_input(char*, size_t) 的行为
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ui.h" /* 期望定义 INPUT_OK/INPUT_EOF/INPUT_TRUNC/INPUT_ERROR 和 get_user_input */

static void restore_stdin(FILE *saved) {
    if (saved) {
        stdin = saved;
    }
}

int main(void) {
    /* 验证宏存在 */
#ifdef INPUT_OK
#else
    fprintf(stderr, "跳过固定缓冲测试：未定义 INPUT_OK 等宏的 ui.h \n");
    return 2;
#endif

    /* 1) 普通输入 + 去除首尾空白 */
    {
        const char *data = "   hello world   \n";
        char buf[256];
        FILE *saved = stdin;
        FILE *mem = fmemopen((void*)data, strlen(data), "r");
        stdin = mem;

        int ret = get_user_input(buf, sizeof(buf));
        fclose(mem);
        restore_stdin(saved);

        assert(ret == INPUT_OK);
        if (strcmp(buf, "hello world") != 0) {
            fprintf(stderr, "测试失败：去首尾空白，期望 'hello world'，实际 '%s'\n", buf);
            return 1;
        }
    }

    /* 2) 截断输入（超长） */
    {
        const char *data = "this_is_a_very_long_line_without_newline\n";
        char buf[8]; /* 故意很小以触发截断 */
        FILE *saved = stdin;
        FILE *mem = fmemopen((void*)data, strlen(data), "r");
        stdin = mem;

        int ret = get_user_input(buf, sizeof(buf));
        fclose(mem);
        restore_stdin(saved);

        if (ret != INPUT_TRUNC) {
            fprintf(stderr, "测试失败：期望截断返回 INPUT_TRUNC, 实际 %d\n", ret);
            return 1;
        }
    }

    /* 3) EOF 情况 */
    {
        const char *data = "";
        char buf[16];
        FILE *saved = stdin;
        FILE *mem = fmemopen((void*)data, strlen(data), "r");
        stdin = mem;

        int ret = get_user_input(buf, sizeof(buf));
        fclose(mem);
        restore_stdin(saved);

        if (ret != INPUT_EOF) {
            fprintf(stderr, "测试失败：期望 EOF 返回 INPUT_EOF, 实际 %d\n", ret);
            return 1;
        }
    }

    printf("固定缓冲单元测试通过\n");
    return 0;
}