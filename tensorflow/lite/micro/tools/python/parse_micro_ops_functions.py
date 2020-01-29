import os
import re
import tensorflow as tf
import binascii
import re

def parse_model_file(path, variable_name):
    """ this will parse the hex string form a model file model_data[] = {0x18, 0x00, ....} given
        the file path and the variable name
    """
    M = []
    with open(path, "r") as fid:
        start_parsing = False
        for line in fid.readlines():
            if start_parsing:
                M.append(line.lstrip())
            if variable_name in line:
                start_parsing = True
            if "};" in line:
                start_parsing = False

    return parse_model_hex_string("".join(M).replace("};", ""))


def parse_model_hex_string(hex_string):
    """ this will parse the hex string form a model file 
        only pass in the string with the hex values  '0x18, 0x00...
    '"""
    hex_string.replace(",\n", "")
    return "".join([x.lstrip().rstrip()[2:4] for x in hex_string.split(",")])


def get_min_max_versions(line):
    l = line.split(',')
    if len(l) > 2:
        return [int(re.sub('\D', '', l[2])), int(re.sub('\D', '', l[3]))]
        
    return None

def parse_all_ops(path):

    if not os.path.exists(path):
        raise Exception("cannot find all_ops_resolver.cc at path {}".format(path))

    function_list = []
    function_versions = {}
    with open(path, "r") as fid:
        for line in fid.readlines():
            if "BuiltinOperator" in line:
                s = line.lstrip()[27:].split(",")
                function_list.extend([s[0]])
                function_versions[s[0]] = get_min_max_versions(line)

    return function_list, function_versions

def get_name_interpreter(name):

    if "dense" in name:
        return "FULLY_CONNECTED"

    return name.upper()

def gen_model_functions(function_list, function_versions):

    M = [
        "  // Only Pull in functions that are needed by the model",
    ]

    template = "  AddBuiltin(BuiltinOperator_{0}, Register_{0}());"
    template_version = "  AddBuiltin(BuiltinOperator_{0}, Register_{0}(), {1}, {2});"

    for function in sorted(list(function_list)): # sort so always in same order
        version = function_versions[function]
        if version[0]:
            M.append(template_version.format(function, version[0], version[1]))
        else:
            M.append(template.format(function))

    return M


# TODO: This function needs to be improved. Right now it is only pulling out the ops name. Do we need to pull out activations as well? What about only specifiying the specific versions that we need.
def parse_tensorflow_binary_model(model_binary):

    tf_model = tf.lite.Interpreter(model_content=binascii.unhexlify(model_binary))
    used_functions = set()
    for op in tf_model._get_ops_details():
        name = get_name_interpreter(op["op_name"])
        if name:
            used_functions.add(name)

    return used_functions


def validate_micro_functions_available(used_functions, micro_functions):

    for used_function in used_functions:
        if used_function not in micro_functions:
            raise Exception(
                "model uses {} which is not a supported function in tf micro".format(
                    used_function
                )
            )

    print("Found Ops", used_functions) 


def gen_micro_mutable_ops_resolver_add(model, all_ops_path):

    micro_functions, function_versions = parse_all_ops(all_ops_path)

    used_functions = parse_tensorflow_binary_model(model)

    validate_micro_functions_available(used_functions, micro_functions)

    return gen_model_functions(used_functions, function_versions)


def fill_template_file(
    model,
    template_path="./used_ops_resolver.cc.tpl",
    output="./used_ops_resolver.cc",
    all_ops_path="../../kernels/all_ops_resolver.cc",
    ):


    template = {'used_ops_resolver': gen_micro_mutable_ops_resolver_add(model, all_ops_path)}

    with open(template_path, "r") as fid:
        output_str = "".join(fid.readlines())
        for key, value in template.items():
            output_str = re.sub(
                r"//FILL_{}\b".format(key.upper()), "\n".join(value), output_str,
            )

    with open(output, "w") as out:
        out.write(output_str)

    return template


if __name__ == "__main__":

    import argparse
    import sys

    parser = argparse.ArgumentParser(description='Generate micro_api.cc with necessary functions to run a model.')

    parser.add_argument('--model_file', type=str, help='path to model file to use')
    parser.add_argument('--model_name', help='name of the variable holding the model data')                    
    parser.add_argument('--model_binary', type=str, help='model binary string to use')

    args = parser.parse_args()

    if not len(sys.argv) > 1:
        # Use all ops
        model_binary = None
    
    elif args.model_binary:
        model_binary = args.model_binary

    elif (args.model_file or args.model_name):
        if args.model_file and not args.model_name:
            raise Exception("Model name was not supplied.")

        if args.model_name and not args.model_file:
            raise Exception("Model File was not supplied.")
        
        model_binary = parse_model_file(
            args.model_file,
            args.model_name,
        )

    print(fill_template_file(model_binary))