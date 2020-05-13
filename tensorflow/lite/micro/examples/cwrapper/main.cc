#include "tensorflow/lite/micro/examples/cwrapper/micro_api_benchmark.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Requires one parameter (a path to a tflite file)\n");
        return 1;
    }
    int arena_size =  128 * 1024;
    int * arena_size_p = &arena_size;
    int ret;

    ret = find_arena_size(tflite_buffer, arena_size_p, 2 * 1024, 128 * 1024);

    if (ret == success)
    {
        printf("arena_size is %d\n", arena_size);
    }

    else if (ret==version_unspported)
    {
        printf("unuported version.");
    }

    else if (ret==allocate_failed)
    {
        printf("allocation failed.");
    }

    else if (ret==malloc_failed)
    {
        printf("malloc failed.");
    }


    ret = test_invoke(tflite_buffer, arena_size);
}