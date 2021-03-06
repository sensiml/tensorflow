/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "tensorflow/core/kernels/data/parallel_interleave_dataset_op.h"

#include "tensorflow/core/kernels/data/dataset_test_base.h"

namespace tensorflow {
namespace data {
namespace {

constexpr char kNodeName[] = "parallel_interleave_dataset";
constexpr int kOpVersion = 2;

class ParallelInterleaveDatasetParams : public DatasetParams {
 public:
  template <typename T>
  ParallelInterleaveDatasetParams(T input_dataset_params,
                                  std::vector<Tensor> other_arguments,
                                  int64 cycle_length, int64 block_length,
                                  int64 num_parallel_calls,
                                  FunctionDefHelper::AttrValueWrapper func,
                                  std::vector<FunctionDef> func_lib,
                                  DataTypeVector type_arguments,
                                  DataTypeVector output_dtypes,
                                  std::vector<PartialTensorShape> output_shapes,
                                  bool sloppy, string node_name)
      : DatasetParams(std::move(output_dtypes), std::move(output_shapes),
                      std::move(node_name)),
        other_arguments_(std::move(other_arguments)),
        cycle_length_(cycle_length),
        block_length_(block_length),
        num_parallel_calls_(num_parallel_calls),
        func_(std::move(func)),
        func_lib_(std::move(func_lib)),
        type_arguments_(std::move(type_arguments)),
        sloppy_(sloppy) {
    input_dataset_params_.push_back(absl::make_unique<T>(input_dataset_params));
    op_version_ = kOpVersion;
    name_utils::IteratorPrefixParams params;
    params.op_version = op_version_;
    iterator_prefix_ = name_utils::IteratorPrefix(
        input_dataset_params.dataset_type(),
        input_dataset_params.iterator_prefix(), params);
  }

  std::vector<Tensor> GetInputTensors() const override {
    auto input_tensors = other_arguments_;
    input_tensors.emplace_back(
        CreateTensor<int64>(TensorShape({}), {cycle_length_}));
    input_tensors.emplace_back(
        CreateTensor<int64>(TensorShape({}), {block_length_}));
    input_tensors.emplace_back(
        CreateTensor<int64>(TensorShape({}), {num_parallel_calls_}));
    return input_tensors;
  }

  Status GetInputNames(std::vector<string>* input_names) const override {
    input_names->emplace_back(ParallelInterleaveDatasetOp::kInputDataset);
    for (int i = 0; i < other_arguments_.size(); ++i) {
      input_names->emplace_back(
          absl::StrCat(ParallelInterleaveDatasetOp::kOtherArguments, "_", i));
    }
    input_names->emplace_back(ParallelInterleaveDatasetOp::kCycleLength);
    input_names->emplace_back(ParallelInterleaveDatasetOp::kBlockLength);
    input_names->emplace_back(ParallelInterleaveDatasetOp::kNumParallelCalls);
    return Status::OK();
  }

  Status GetAttributes(AttributeVector* attr_vector) const override {
    *attr_vector = {
        {ParallelInterleaveDatasetOp::kFunc, func_},
        {ParallelInterleaveDatasetOp::kTarguments, type_arguments_},
        {ParallelInterleaveDatasetOp::kOutputShapes, output_shapes_},
        {ParallelInterleaveDatasetOp::kOutputTypes, output_dtypes_},
        {ParallelInterleaveDatasetOp::kSloppy, sloppy_}};
    return Status::OK();
  }

  string dataset_type() const override {
    return ParallelInterleaveDatasetOp::kDatasetType;
  }

  std::vector<FunctionDef> func_lib() const override { return func_lib_; }

 private:
  std::vector<Tensor> other_arguments_;
  int64 cycle_length_;
  int64 block_length_;
  int64 num_parallel_calls_;
  FunctionDefHelper::AttrValueWrapper func_;
  std::vector<FunctionDef> func_lib_;
  DataTypeVector type_arguments_;
  bool sloppy_;
};

class ParallelInterleaveDatasetOpTest : public DatasetOpsTestBaseV2 {};

FunctionDefHelper::AttrValueWrapper MakeTensorSliceDatasetFunc(
    const DataTypeVector& output_types,
    const std::vector<PartialTensorShape>& output_shapes) {
  return FunctionDefHelper::FunctionRef(
      /*name=*/"MakeTensorSliceDataset",
      /*attrs=*/{{"Toutput_types", output_types},
                 {"output_shapes", output_shapes}});
}

// test case 1: cycle_length = 1, block_length = 1, num_parallel_calls = 1,
// sloppy = false
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams1() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<int64>(TensorShape{3, 3, 1},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 8})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/1,
      /*block_length=*/1,
      /*num_parallel_calls=*/1,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_INT64}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_INT64},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/false,
      /*node_name=*/kNodeName);
}

// test case 2: cycle_length = 2, block_length = 1, num_parallel_calls = 2,
// sloppy = false
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams2() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<int64>(TensorShape{3, 3, 1},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 8})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/2,
      /*block_length=*/1,
      /*num_parallel_calls=*/2,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_INT64}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_INT64},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/false,
      /*node_name=*/kNodeName);
}

// test case 3: cycle_length = 3, block_length = 1, num_parallel_calls = 2,
// sloppy = true
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams3() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<int64>(TensorShape{3, 3, 1},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 8})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/3,
      /*block_length=*/1,
      /*num_parallel_calls=*/2,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_INT64}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_INT64},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 4: cycle_length = 5, block_length = 1, num_parallel_calls = 4,
// sloppy = true

ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams4() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<int64>(TensorShape{3, 3, 1},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 8})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/5,
      /*block_length=*/1,
      /*num_parallel_calls=*/4,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_INT64}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_INT64},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 5: cycle_length = 2, block_length = 2, num_parallel_calls = 1,
// sloppy = false
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams5() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<tstring>(
          TensorShape{3, 3, 1}, {"a", "b", "c", "d", "e", "f", "g", "h", "i"})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/2,
      /*block_length=*/2,
      /*num_parallel_calls=*/1,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_STRING}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_STRING},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 6: cycle_length = 2, block_length = 3, num_parallel_calls = 2,
// sloppy = true
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams6() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<tstring>(
          TensorShape{3, 3, 1}, {"a", "b", "c", "d", "e", "f", "g", "h", "i"})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/2,
      /*block_length=*/3,
      /*num_parallel_calls=*/2,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_STRING}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_STRING},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 7: cycle_length = 3, block_length = 2, num_parallel_calls = 2,
// sloppy = false
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams7() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<tstring>(
          TensorShape{3, 3, 1}, {"a", "b", "c", "d", "e", "f", "g", "h", "i"})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/3,
      /*block_length=*/2,
      /*num_parallel_calls=*/2,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_STRING}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_STRING},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/false,
      /*node_name=*/kNodeName);
}

// test case 8: cycle_length = 3, block_length = 3, num_parallel_calls = 3,
// sloppy = true
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams8() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<tstring>(
          TensorShape{3, 3, 1}, {"a", "b", "c", "d", "e", "f", "g", "h", "i"})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/3,
      /*block_length=*/3,
      /*num_parallel_calls=*/3,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_STRING}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_STRING},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 9: cycle_length = 4, block_length = 4, num_parallel_calls = 4,
// sloppy = true
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams9() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<tstring>(
          TensorShape{3, 3, 1}, {"a", "b", "c", "d", "e", "f", "g", "h", "i"})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/4,
      /*block_length=*/4,
      /*num_parallel_calls=*/4,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_STRING}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_STRING},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 10: cycle_length = 3, block_length = 3,
// num_parallel_calls = kAutotune, sloppy = true
ParallelInterleaveDatasetParams ParallelInterleaveDatasetParams10() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<tstring>(
          TensorShape{3, 3, 1}, {"a", "b", "c", "d", "e", "f", "g", "h", "i"})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/4,
      /*block_length=*/4,
      /*num_parallel_calls=*/model::kAutotune,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_STRING}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_STRING},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 11: cycle_length = 0, block_length = 1, num_parallel_calls = 2,
// sloppy = true
ParallelInterleaveDatasetParams
ParallelInterleaveDatasetParamsWithInvalidCycleLength() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<int64>(TensorShape{3, 3, 1},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 8})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/0,
      /*block_length=*/1,
      /*num_parallel_calls=*/2,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_INT64}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_INT64},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 12: cycle_length = 1, block_length = -1, num_parallel_calls = 2,
// sloppy = true
ParallelInterleaveDatasetParams
ParallelInterleaveDatasetParamsWithInvalidBlockLength() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<int64>(TensorShape{3, 3, 1},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 8})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/1,
      /*block_length=*/-1,
      /*num_parallel_calls=*/2,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_INT64}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_INT64},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

// test case 13: cycle_length = 1, block_length = 1, num_parallel_calls = -5,
// sloppy = true
ParallelInterleaveDatasetParams
ParallelInterleaveDatasetParamsWithInvalidNumParallelCalls() {
  auto tensor_slice_dataset_params = TensorSliceDatasetParams(
      /*components=*/{CreateTensor<int64>(TensorShape{3, 3, 1},
                                          {0, 1, 2, 3, 4, 5, 6, 7, 8})},
      /*node_name=*/"tensor_slice");
  return ParallelInterleaveDatasetParams(
      tensor_slice_dataset_params,
      /*other_arguments=*/{},
      /*cycle_length=*/1,
      /*block_length=*/1,
      /*num_parallel_calls=*/-5,
      /*func=*/
      MakeTensorSliceDatasetFunc(
          DataTypeVector({DT_INT64}),
          std::vector<PartialTensorShape>({PartialTensorShape({1})})),
      /*func_lib=*/{test::function::MakeTensorSliceDataset()},
      /*type_arguments=*/{},
      /*output_dtypes=*/{DT_INT64},
      /*output_shapes=*/{PartialTensorShape({1})},
      /*sloppy=*/true,
      /*node_name=*/kNodeName);
}

std::vector<GetNextTestCase<ParallelInterleaveDatasetParams>>
GetNextTestCases() {
  return {{/*dataset_params=*/ParallelInterleaveDatasetParams1(),
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}}),
           /*compare_order=*/true},
          {/*dataset_params=*/ParallelInterleaveDatasetParams2(),
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {3}, {1}, {4}, {2}, {5}, {6}, {7}, {8}}),
           /*compare_order=*/true},
          {/*dataset_params=*/ParallelInterleaveDatasetParams3(),
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {3}, {6}, {1}, {4}, {7}, {2}, {5}, {8}}),
           /*compare_order=*/false},
          {/*dataset_params=*/ParallelInterleaveDatasetParams4(),
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {3}, {6}, {1}, {4}, {7}, {2}, {5}, {8}}),
           /*compare_order=*/false},
          {/*dataset_params=*/ParallelInterleaveDatasetParams5(),
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"d"}, {"e"}, {"c"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams6(),
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams7(),
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"d"}, {"e"}, {"g"}, {"h"}, {"c"}, {"f"}, {"i"}}),
           /*compare_order=*/true},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams8(),
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams9(),
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams10(),
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false}};
}

ITERATOR_GET_NEXT_TEST_P(ParallelInterleaveDatasetOpTest,
                         ParallelInterleaveDatasetParams, GetNextTestCases())

TEST_F(ParallelInterleaveDatasetOpTest, DatasetNodeName) {
  auto dataset_params = ParallelInterleaveDatasetParams1();
  TF_ASSERT_OK(Initialize(dataset_params));
  TF_ASSERT_OK(CheckDatasetNodeName(dataset_params.node_name()));
}

TEST_F(ParallelInterleaveDatasetOpTest, DatasetTypeString) {
  auto dataset_params = ParallelInterleaveDatasetParams1();
  TF_ASSERT_OK(Initialize(dataset_params));
  name_utils::OpNameParams params;
  params.op_version = dataset_params.op_version();
  TF_ASSERT_OK(CheckDatasetTypeString(
      name_utils::OpName(ParallelInterleaveDatasetOp::kDatasetType, params)));
}

TEST_F(ParallelInterleaveDatasetOpTest, DatasetOutputDtypes) {
  auto dataset_params = ParallelInterleaveDatasetParams1();
  TF_ASSERT_OK(Initialize(dataset_params));
  TF_ASSERT_OK(CheckDatasetOutputDtypes({DT_INT64}));
}

TEST_F(ParallelInterleaveDatasetOpTest, DatasetOutputShapes) {
  auto dataset_params = ParallelInterleaveDatasetParams1();
  TF_ASSERT_OK(Initialize(dataset_params));
  TF_ASSERT_OK(CheckDatasetOutputShapes({PartialTensorShape({1})}));
}

std::vector<CardinalityTestCase<ParallelInterleaveDatasetParams>>
CardinalityTestCases() {
  return {{/*dataset_params=*/ParallelInterleaveDatasetParams1(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams2(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams3(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams4(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams5(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams6(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams7(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams8(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams9(),
           /*expected_cardinality=*/kUnknownCardinality},
          {/*dataset_params=*/ParallelInterleaveDatasetParams10(),
           /*expected_cardinality=*/kUnknownCardinality}};
}

DATASET_CARDINALITY_TEST_P(ParallelInterleaveDatasetOpTest,
                           ParallelInterleaveDatasetParams,
                           CardinalityTestCases())

TEST_F(ParallelInterleaveDatasetOpTest, IteratorOutputDtypes) {
  auto dataset_params = ParallelInterleaveDatasetParams1();
  TF_ASSERT_OK(Initialize(dataset_params));
  TF_ASSERT_OK(CheckIteratorOutputDtypes({DT_INT64}));
}

TEST_F(ParallelInterleaveDatasetOpTest, IteratorOutputShapes) {
  auto dataset_params = ParallelInterleaveDatasetParams1();
  TF_ASSERT_OK(Initialize(dataset_params));
  TF_ASSERT_OK(CheckIteratorOutputShapes({PartialTensorShape({1})}));
}

TEST_F(ParallelInterleaveDatasetOpTest, IteratorPrefix) {
  auto dataset_params = ParallelInterleaveDatasetParams1();
  TF_ASSERT_OK(Initialize(dataset_params));
  name_utils::IteratorPrefixParams params;
  params.op_version = dataset_params.op_version();
  TF_ASSERT_OK(CheckIteratorPrefix(
      name_utils::IteratorPrefix(ParallelInterleaveDatasetOp::kDatasetType,
                                 dataset_params.iterator_prefix(), params)));
}

std::vector<IteratorSaveAndRestoreTestCase<ParallelInterleaveDatasetParams>>
IteratorSaveAndRestoreTestCases() {
  return {{/*dataset_params=*/ParallelInterleaveDatasetParams1(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}}),
           /*compare_order=*/true},
          {/*dataset_params=*/ParallelInterleaveDatasetParams2(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {3}, {1}, {4}, {2}, {5}, {6}, {7}, {8}}),
           /*compare_order=*/true},
          {/*dataset_params=*/ParallelInterleaveDatasetParams3(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {3}, {6}, {1}, {4}, {7}, {2}, {5}, {8}}),
           /*compare_order=*/false},
          {/*dataset_params=*/ParallelInterleaveDatasetParams4(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<int64>(TensorShape{1},
                                {{0}, {3}, {6}, {1}, {4}, {7}, {2}, {5}, {8}}),
           /*compare_order=*/false},
          {/*dataset_params=*/ParallelInterleaveDatasetParams5(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"d"}, {"e"}, {"c"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams6(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams7(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"d"}, {"e"}, {"g"}, {"h"}, {"c"}, {"f"}, {"i"}}),
           /*compare_order=*/true},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams8(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams9(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false},
          {/*dataset_params=*/
           ParallelInterleaveDatasetParams10(),
           /*breakpoints=*/{0, 4, 11},
           /*expected_outputs=*/
           CreateTensors<tstring>(
               TensorShape{1},
               {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}, {"f"}, {"g"}, {"h"}, {"i"}}),
           /*compare_order=*/false}};
}

ITERATOR_SAVE_AND_RESTORE_TEST_P(ParallelInterleaveDatasetOpTest,
                                 ParallelInterleaveDatasetParams,
                                 IteratorSaveAndRestoreTestCases())

TEST_F(ParallelInterleaveDatasetOpTest, InvalidArguments) {
  std::vector<ParallelInterleaveDatasetParams> invalid_params = {
      ParallelInterleaveDatasetParamsWithInvalidCycleLength(),
      ParallelInterleaveDatasetParamsWithInvalidBlockLength(),
      ParallelInterleaveDatasetParamsWithInvalidNumParallelCalls()};
  for (auto& dataset_params : invalid_params) {
    EXPECT_EQ(Initialize(dataset_params).code(),
              tensorflow::error::INVALID_ARGUMENT);
  }
}

}  // namespace
}  // namespace data
}  // namespace tensorflow
