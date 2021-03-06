/*******************************************************************************
 * Copyright (c) 2015-2018 Skymind, Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
// @author Yurii Shyrma (iuriish@yahoo.com), created on 04.06.2018
//


#include <ops/declarable/CustomOperations.h>


namespace nd4j    {
namespace ops     {

//////////////////////////////////////////////////////////////////////////
CUSTOM_OP_IMPL(reduce_stdev, 1, 1, false, 0, 0) {
    auto input   = INPUT_VARIABLE(0);
    auto output  = OUTPUT_VARIABLE(0);

    const bool keepDims      = block.getTArguments()->size() > 0 ? (bool)T_ARG(0) : false;
    const bool biasCorrected = block.getTArguments()->size() > 1 ? (bool)T_ARG(1) : false;
    
    std::vector<int> dimensions = *block.getIArguments();    

    REQUIRE_TRUE(dimensions.size() <= input->rankOf(), 0, "REDUCE_STDEV OP: the number of dimensions to reduce along must be <= input array rank, but got %i instead" , dimensions.size());

    for(const auto& item : dimensions)
        REQUIRE_TRUE(item > -input->rankOf() || item < input->rankOf(), 0, "REDUCE_STDEV OP: the input dimension to reduce along must be in range (-%i, %i), but got %i instead !" , input->rankOf(), input->rankOf(), item);
        
    input->varianceAlongDimension(variance::SummaryStatsStandardDeviation, output, biasCorrected, dimensions);

    return Status::OK();
}

        DECLARE_TYPES(reduce_stdev) {
            getOpDescriptor()
                    ->setAllowedInputTypes(nd4j::DataType::ANY)
                    ->setAllowedOutputTypes({ALL_FLOATS});
        }


DECLARE_SHAPE_FN(reduce_stdev) {    
    const bool keepDims = block.getTArguments()->size() > 0 ? (bool)T_ARG(0) : false;
    
    std::vector<int> dimensions = *block.getIArguments();

    REQUIRE_TRUE(dimensions.size() <= inputShape->at(0)[0], 0, "REDUCE_STDEV OP: the number of dimensions to reduce along must be <= input array rank, but got %i instead" , dimensions.size());
    
    for(const auto& item : dimensions)
        REQUIRE_TRUE(item > -inputShape->at(0)[0] || item < inputShape->at(0)[0], 0, "REDUCE_STDEV OP: the input dimension to reduce along must be in range (-%i, %i), but got %i instead !" , inputShape->at(0)[0], inputShape->at(0)[0], item);

    Nd4jLong* outShapeInfo = ShapeUtils::evalReduceShapeInfo(shape::order(inputShape->at(0)), dimensions, inputShape->at(0), keepDims, false, block.getWorkspace());
    ArrayOptions::setDataType(outShapeInfo, ArrayOptions::dataType(inputShape->at(0)));

    return SHAPELIST(outShapeInfo);
}

        DECLARE_TYPES(reduce_stdev_bp) {
            getOpDescriptor()
                    ->setAllowedInputTypes(nd4j::DataType::ANY)
                    ->setAllowedOutputTypes({ALL_FLOATS});
        }

//////////////////////////////////////////////////////////////////////////
CUSTOM_OP_IMPL(reduce_stdev_bp, 2, 1, false, 0, 0) {
    auto input  = INPUT_VARIABLE(0);
    auto gradO  = INPUT_VARIABLE(1);

    auto gradI  = OUTPUT_VARIABLE(0);

    const bool keepDims = block.getTArguments()->size() > 0 ? (bool)T_ARG(0) : false;
    const bool biasCorrected = block.getTArguments()->size() > 1 ? (bool)T_ARG(1) : false;    
    
    std::vector<int> dimensions = *block.getIArguments();    

    REQUIRE_TRUE(dimensions.size() <= input->rankOf(), 0, "REDUCE_STDEV OP: the number of dimensions to reduce along must be <= input array rank, but got %i instead" , dimensions.size());

    for(const auto& item : dimensions)
        REQUIRE_TRUE(item > -input->rankOf() || item < input->rankOf(), 0, "REDUCE_STDEV OP: the input dimension to reduce along must be in range (-%i, %i), but got %i instead !" , input->rankOf(), input->rankOf(), item);        

    const Nd4jLong N = input->lengthOf() / gradO->lengthOf();
    const Nd4jLong NminusOne = biasCorrected ? N - 1 : N;               

    auto mean = input->reduceAlongDims(reduce::Mean, dimensions, true);
    
    NDArray variance(mean.getShapeInfo(), true, block.getWorkspace());                    // create empty array with shape matching shape of mean array
    input->varianceAlongDimension(variance::SummaryStatsStandardDeviation, &variance, biasCorrected, dimensions);

    gradI->assign( (*input - mean) / (variance * NminusOne));                              // automatic broadcasting happens here

    Nd4jLong* gradOShapeKeepDims = ShapeUtils::evalReduceShapeInfo(input->ordering(), dimensions, *input, true, false, block.getWorkspace());
    const bool isGradOShapeBroadcast = shape::equalsSoft(gradOShapeKeepDims, gradO->getShapeInfo());

    if(!isGradOShapeBroadcast)
        gradO = gradO->reshape(gradO->ordering(), ShapeUtils::pullShapeFromShapeInfo(gradOShapeKeepDims));  // for example could be something like [a,b] -> [1,a,1,b]
    
    *gradI *= *gradO;
    
    if(!isGradOShapeBroadcast)
        delete gradO;
    
    return Status::OK();
}



DECLARE_SHAPE_FN(reduce_stdev_bp) {    
    const bool keepDims = block.getTArguments()->size() > 0 ? (bool)T_ARG(0) : false;

    std::vector<int> dimensions = *block.getIArguments();

    REQUIRE_TRUE(dimensions.size() <= inputShape->at(0)[0], 0, "REDUCE_STDEV OP: the number of dimensions to reduce along must be <= input array rank, but got %i instead" , dimensions.size());
    
    for(const auto& item : dimensions)
        REQUIRE_TRUE(item > -inputShape->at(0)[0] || item < inputShape->at(0)[0], 0, "REDUCE_STDEV OP: the input dimension to reduce along must be in range (-%i, %i), but got %i instead !" , inputShape->at(0)[0], inputShape->at(0)[0], item);
    
    Nd4jLong* gradIshapeInfo(nullptr);
    COPY_SHAPE(inputShape->at(0), gradIshapeInfo);
        
    return SHAPELIST(gradIshapeInfo);
}


}
}
