#include "tensorflow/lite/micro/examples/model_tester/model.h"
#include "tensorflow/lite/micro/examples/model_tester/test_data.h"
#include "tensorflow/lite/micro/examples/model_tester/micro_api.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/debug_log.h"
//#include "tensorflow/lite/micro/examples/model_tester/micro_api_benchmark.h"

#define MAX_TENSOR_ARENA_SIZE 10 * 1024
#define MIN_TENSOR_ARENA_SIZE 2 * 1024
#define ARENA_TOLERANCE  1024

uint8_t tensor_arena[MAX_TENSOR_ARENA_SIZE];

int main(int argc, char** argv) {
  int arena_size = MAX_TENSOR_ARENA_SIZE;
  int* arena_size_p = &arena_size;
  int ret;

/*
  ret = find_arena_size(g_model, tensor_arena, arena_size_p,
                        MIN_TENSOR_ARENA_SIZE, MAX_TENSOR_ARENA_SIZE,
                        ARENA_TOLERANCE);

  if (ret == success) {
    //printf("Approximate Arena Size is %d\n", arena_size);
  }

  else if (ret == version_unspported) {
    //printf("unuported version.\n");
  }

  else if (ret == allocate_failed) {
   // printf("allocation failed.\n");
  }

  else if (ret == malloc_failed) {
  //  printf("malloc failed.\n");
  }
*/
  DebugLog("Test Test.");
  ret = tf_micro_model_setup(g_model, tensor_arena, arena_size, true);

  if (ret == success) 
  {
    for (int index = 0; index < TEST_DATA_LENGTH; index++) {
      for (int i = 0; i < MODEL_OUTPUTS; i++) {
        results[i] = 0.0;
      }

      ret = tf_micro_model_invoke(test_data[index], MODEL_INPUTS, results, MODEL_OUTPUTS);

      if (ret == success) {
       // printf("Test Vector %d result: ", index);
        for (int i = 0; i < MODEL_OUTPUTS; i++) {
         // printf("%f, ", results[i]);
          results[i] = 0.0;
        }
        //printf("\n");
      }
    }
}

}