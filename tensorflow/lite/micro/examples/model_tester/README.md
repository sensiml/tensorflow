This is a simple project that can be used to test your model. 

First call the codegen.py file and pass in the path to your model.json file. A test example is 
included in the directory. The model.json file should include the tflite model as a string, 
test data

`python codegen.py test_model.json`

which will create the micro_api.cc file with a file containing only the operations
that need to be loaded by the micro_mutable_ops_resolver. 

you can generate the binary string directly from your tensorflow model


```python
import binascii
converter = tf.lite.TFLiteConverter.from_keras_model(tf_model)
tflite_model = converter.convert()
binascii.hexlify(tflite_model).decode('ascii')
```








