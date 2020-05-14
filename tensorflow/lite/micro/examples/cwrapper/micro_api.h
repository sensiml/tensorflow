
#ifndef TENSORFLOW_LITE_MICRO_C_API_H_
#define TENSORFLOW_LITE_MICRO_C_API_H_


#ifdef __cplusplus
extern "C" {
#endif

extern int __exidx_start(); //some library wants these set, note sure what, exceptions should be disabled. https://forum.pjrc.com/threads/57192-Teensy-4-0-linker-issues-with-STL-libraries
extern int __exidx_end();



typedef enum {success, version_unspported, allocate_failed, invoke_failed, malloc_failed} MICRO_ERRORS;

int tf_micro_model_setup(const void * model_data, unsigned char * tensor_arena, int kTensorArenaSize, bool set_static);

int tf_micro_model_invoke(float* input_data, int num_inputs, float* results, int num_outputs);




#ifdef __cplusplus
}
#endif


#endif //TENSORFLOW_LITE_MICRO_C_API_H_             
