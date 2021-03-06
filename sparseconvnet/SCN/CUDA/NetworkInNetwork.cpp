// Copyright 2016-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <algorithm>

template <typename T>
double cuda_NetworkInNetwork_updateOutput(
    /*cuda float*/ at::Tensor input_features,
    /*cuda float*/ at::Tensor output_features,
    /*cuda float*/ at::Tensor weight, /*cuda float*/ at::Tensor bias) {
  auto nActive = input_features.size(0);
  auto input_nPlanes = weight.size(0);
  auto output_nPlanes = weight.size(1);
  output_features.resize_({nActive, output_nPlanes});
  if (bias.numel())
    output_features.copy_(bias);
  else
    output_features.zero_();
  if (nActive)
    output_features.addmm_(input_features, weight);
  return nActive * input_nPlanes * output_nPlanes;
}

template <typename T>
void cuda_NetworkInNetwork_updateGradInput(
    /*cuda float*/ at::Tensor d_input_features,
    /*cuda float*/ at::Tensor d_output_features,
    /*cuda float*/ at::Tensor weight) {

  int nActive = d_output_features.size(0);
  d_input_features.resize_({nActive, weight.size(0)});
  d_input_features.zero_();
  if (nActive)
    at::mm_out(d_input_features, d_output_features, weight.t());
}

template <typename T>
void cuda_NetworkInNetwork_accGradParameters(
    /*cuda float*/ at::Tensor input_features,
    /*cuda float*/ at::Tensor d_output_features,
    /*cuda float*/ at::Tensor d_weight, /*cuda float*/ at::Tensor d_bias) {
  auto nActive = input_features.size(0);
  if (nActive and d_bias.numel())
    at::sum_out(d_bias, d_output_features, {0}, false);
  if (nActive)
    at::mm_out(d_weight, input_features.t(), d_output_features);
}
