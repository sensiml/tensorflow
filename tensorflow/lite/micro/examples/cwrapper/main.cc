#include "tensorflow/lite/micro/examples/cwrapper/micro_api_benchmark.h"
#include "tensorflow/lite/micro/examples/cwrapper/model.h"
#include "tensorflow/lite/micro/examples/cwrapper/test_data.h"

int main(int argc, char** argv) {
  int arena_size = 128 * 1024;
  int* arena_size_p = &arena_size;
  int ret;
  int i;

  ret = find_arena_size(g_model, arena_size_p, 2 * 1024, 128 * 1024);

  if (ret == success) {
    printf("arena_size is %d\n", arena_size);
  }

  else if (ret == version_unspported) {
    printf("unuported version.\n");
  }

  else if (ret == allocate_failed) {
    printf("allocation failed.\n");
  }

  else if (ret == malloc_failed) {
    printf("malloc failed.\n");
  }

  for (int index = 0; index < TEST_DATA_LENGTH; index++) {
    for (i = 0; i < MODEL_OUTPUTS; i++) {
      results[i] = 0.0;
    }

    ret = test_invoke(g_model, arena_size, test_data[index], MODEL_INPUTS,
                      results, MODEL_OUTPUTS);

    if (ret == success) {
      printf("Test Vector %d result: ", index);
      for (i = 0; i < MODEL_OUTPUTS; i++) {
        printf("%f, ", results[i]);
        results[i] = 0.0;
      }
      printf("\n");
    }
  }
}