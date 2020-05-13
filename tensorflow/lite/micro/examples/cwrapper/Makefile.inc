TENSORFLOW_LIB_SRCS := tensorflow/lite/micro/examples/cwrapper/micro_api.cc

TENSORFLOW_LIB_HDRS := tensorflow/lite/micro/examples/cwrapper/micro_api.h 

TENSORFLOW_CWRAPPER_TEST_SRCS := tensorflow/lite/micro/examples/cwrapper/micro_api.cc \
					   tensorflow/lite/micro/examples/cwrapper/model.cc \
					   tensorflow/lite/micro/examples/cwrapper/main.cc


TENSORFLOW_CWRAPPER_TEST_HDRS := tensorflow/lite/micro/examples/cwrapper/micro_api.h \
					   tensorflow/lite/micro/examples/cwrapper/micro_api_benchmark.h \
					   tensorflow/lite/micro/examples/cwrapper/model.h \
					   tensorflow/lite/micro/examples/cwrapper/test_data.h


# Builds a standalone library.
$(eval $(call microlite_test,cwrapper,\
$(TENSORFLOW_LIB_SRCS),$(TENSORFLOW_LIB_HDRS)))


# Builds a standalone library.
$(eval $(call microlite_test,cwrapper_test,\
$(TENSORFLOW_CWRAPPER_TEST_SRCS),$(TENSORFLOW_CWRAPPER_TEST_HDRS)))
