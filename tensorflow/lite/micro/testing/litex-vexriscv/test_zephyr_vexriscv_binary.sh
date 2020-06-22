#!/bin/bash -e
# Copyright 2020 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
#
# Tests a 'stm32f4' STM32F4 ELF by parsing the log output of Renode emulation.
#
# First argument is the ELF location.
# Second argument is a regular expression that's required to be in the output logs
# for the test to pass.
#
# This script must be run from the top-level folder of the tensorflow github
# repository as it mounts `pwd` to the renode docker image (via docker run -v)
# and paths in the docker run command assume the entire tensorflow repo is mounted.
declare -r ROOT_DIR=`pwd`
declare -r WORKSPACE=/workspace
declare -r MICRO_LOG_PATH=${WORKSPACE}/tensorflow/lite/micro/testing/logs
declare -r SCRIPT_DIR=${WORKSPACE}/tensorflow/lite/micro/testing/litex-vexriscv/
declare -r HOST_MICRO_LOG_PATH=${ROOT_DIR}/tensorflow/lite/micro/testing/logs
declare -r MICRO_LOG_FILENAME=${MICRO_LOG_PATH}/logs.txt
mkdir -p ${HOST_MICRO_LOG_PATH}

docker build -t renode_stm32f4 \
  -f ${ROOT_DIR}/tensorflow/lite/micro/testing/Dockerfile.stm32f4 \
  ${ROOT_DIR}/tensorflow/lite/micro/testing/

exit_code=0

# running in `if` to avoid setting +e
if ! docker run \
  --log-driver=none -a stdout -a stderr \
  -v ${ROOT_DIR}:${WORKSPACE} \
  -e BIN=${WORKSPACE}/$1 \
  -e SCRIPT_DIR=${WORKSPACE}/${SCRIPT_DIR} \
  -e SCRIPT=${SCRIPT_DIR}litex-vexriscv-tflite-model-tester.resc \
  -p 3333:3333 \
  -e EXPECTED="$2" \
  renode_stm32f4 \
  /bin/bash -c "/opt/renode/tests/test.sh /workspace/tensorflow/lite/micro/testing/litex-vexriscv-tflite.robot 2>&1 > ${MICRO_LOG_FILENAME} && cp *.xml *html ${MICRO_LOG_PATH}"
then
  echo "DOCKER FAILED TO RUN"
  exit_code=1
fi

if [ $exit_code -eq 0 ]
then
  echo "$1: PASS"
else
  echo "$1: FAIL - '$2' not found in logs."
fi
exit $exit_code
                                     