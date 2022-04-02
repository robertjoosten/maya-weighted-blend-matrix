#include <maya/MFnPlugin.h>

#include "weightedBlendMatrixNode.h"


MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Robert Joosten", "1.0", "any");

    status = plugin.registerNode(
        WeightedBlendMatrixNode::kName, 
        WeightedBlendMatrixNode::id, 
        WeightedBlendMatrixNode::creator, 
        WeightedBlendMatrixNode::initialize
    );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(WeightedBlendMatrixNode::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}