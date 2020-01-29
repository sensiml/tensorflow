
you can use the parse_micro_ops_function.py in order to pull out of a tf_lite model 
only the necessary functions. which will generate a used_ops_resolver.cc file 
file with a file containing only the operations that need to be loaded by the 
micro_mutable_ops_resolver. You can then build a much smaller library file to include
in your project.

to parse use a model file

`python parse_micro_ops_functions.py --model_file=../micro_speech/micro_features/tiny_conv_micro_features_model_data.cc --model_name=g_tiny_conv_micro_features_model_data`

to use a model binary directly

`python parse_micro_ops_functions.py --model_binary=<model_binary>`

you can generate the binary string directly from your tensorflow model

```python
    import binascii
    converter = tf.lite.TFLiteConverter.from_keras_model(tf_model)
    tflite_model = converter.convert()
    binascii.hexlify(tflite_model).decode('ascii') 
```    