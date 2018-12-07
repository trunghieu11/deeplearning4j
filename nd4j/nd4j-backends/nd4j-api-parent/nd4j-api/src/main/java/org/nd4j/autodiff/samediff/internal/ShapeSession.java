package org.nd4j.autodiff.samediff.internal;

import org.nd4j.autodiff.functions.DifferentialFunction;
import org.nd4j.autodiff.samediff.SameDiff;
import org.nd4j.linalg.api.shape.LongShapeDescriptor;

public class ShapeSession extends AbstractSession<LongShapeDescriptor, DifferentialFunction> {

    public ShapeSession(SameDiff sameDiff) {
        super(sameDiff);
    }

    @Override
    public LongShapeDescriptor[] getOutputs(DifferentialFunction op) {
        throw new UnsupportedOperationException("Not yet implemented");
    }

    @Override
    public DifferentialFunction getAndParameterizeOp(String opName) {
        throw new UnsupportedOperationException("Not yet implemented");
    }
}
