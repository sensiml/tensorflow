#include "tensorflow/lite/micro/examples/cwrapper/micro_api_benchmark.h"
#include "tensorflow/lite/micro/examples/cwrapper/model.h"
#include "tensorflow/lite/micro/examples/cwrapper/test_data.h"

int main(int argc, char** argv) {
  int arena_size = 128 * 1024;
  int* arena_size_p = &arena_size;
  int ret;
  int i;

  if (argc != 2) {
    printf("Requires one parameter (a path to a tflite file)\n");
    return 1;
  }

  ret = find_arena_size(tflite_buffer, arena_size_p, 2 * 1024, 128 * 1024);

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

  for (int index = 0; index < test_data_len; index++) {
    for (i = 0; i < MODEL_OUTPUTS; i++) {
      result[i] = 0.0;
    }

    ret = test_invoke(tflite_buffer, arena_size, test_data[index], num_inputs,
                      test_result, num_outputs);

    if (ret == success) {
      printf("Test Vector %d result: ", index);
      for (i = 0; i < MODEL_OUTPUTS; i++) {
        printf("%f, ", test_result[i]);
        test_result[i] = 0.0;
      }
      printf("\n");
    }
  }
}