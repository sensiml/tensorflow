TENSORFLOW_LIB_SRCS := tensorflow/lite/micro/examples/model_tester/micro_api.cc

TENSORFLOW_LIB_HDRS := tensorflow/lite/micro/examples/model_tester/micro_api.h 

TENSORFLOW_model_tester_TEST_SRCS := tensorflow/lite/micro/examples/model_tester/micro_api.cc \
					   tensorflow/lite/micro/examples/model_tester/model.cc \
					   tensorflow/lite/micro/examples/model_tester/main.cc \



TENSORFLOW_model_tester_TEST_HDRS := tensorflow/lite/micro/examples/model_tester/micro_api.h \
					   tensorflow/lite/micro/examples/model_tester/model.h \
					   tensorflow/lite/micro/examples/model_tester/test_data.h 



#Find any platform - specific rules for this example.
include $(wildcard tensorflow/lite/micro/examples/model_tester/*/Makefile.inc)


# Builds a standalone library.
$(eval $(call microlite_test,model_tester_lib,\
$(TENSORFLOW_LIB_SRCS),$(TENSORFLOW_LIB_HDRS)))


# Builds a standalone library.
$(eval $(call microlite_test,model_tester,\
$(TENSORFLOW_model_tester_TEST_SRCS),$(TENSORFLOW_model_tester_TEST_HDRS)))

