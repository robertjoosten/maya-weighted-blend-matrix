#include "weightedBlendMatrixNode.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MTypes.h>
#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MTransformationMatrix.h>

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>


using namespace Eigen;


MTypeId WeightedBlendMatrixNode::id(0x00000001);
MObject WeightedBlendMatrixNode::aOutputMatrix;
MObject WeightedBlendMatrixNode::aOffsetMatrix;
MObject WeightedBlendMatrixNode::aRestMatrix;
MObject WeightedBlendMatrixNode::aBlendInputMatrix;
MObject WeightedBlendMatrixNode::aBlendOffsetMatrix;
MObject WeightedBlendMatrixNode::aBlendTranslateWeight;
MObject WeightedBlendMatrixNode::aBlendRotateWeight;
MObject WeightedBlendMatrixNode::aBlendScaleWeight;
MObject WeightedBlendMatrixNode::aBlendShearWeight;
MObject WeightedBlendMatrixNode::aBlendMatrix;


const MString WeightedBlendMatrixNode::kName("wtBlendMatrix");


MStatus WeightedBlendMatrixNode::initialize() {
    MStatus status;

    MFnCompoundAttribute cAttr;
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;
    MFnUnitAttribute uAttr;

    aOutputMatrix = mAttr.create("outputMatrix", "outputMatrix");
    mAttr.setWritable(false);
    mAttr.setStorable(false);
    addAttribute(aOutputMatrix);

    aOffsetMatrix = mAttr.create("offsetMatrix", "offsetMatrix");
    mAttr.setWritable(true);
    mAttr.setStorable(true);
    mAttr.setReadable(false);
    addAttribute(aOffsetMatrix);
    attributeAffects(aOffsetMatrix, aOutputMatrix);

    aRestMatrix = mAttr.create("restMatrix", "restMatrix");
    mAttr.setWritable(true);
    mAttr.setStorable(true);
    mAttr.setReadable(false);
    addAttribute(aRestMatrix);
    attributeAffects(aRestMatrix, aOutputMatrix);

    aBlendInputMatrix = mAttr.create("blendInputMatrix", "blendInputMatrix");
    mAttr.setWritable(true);
    mAttr.setStorable(true);
    mAttr.setReadable(false);

    aBlendOffsetMatrix = mAttr.create("blendOffsetMatrix", "blendOffsetMatrix");
    mAttr.setWritable(true);
    mAttr.setStorable(true);
    mAttr.setReadable(false);

    aBlendTranslateWeight = nAttr.create("blendTranslateWeight", "blendTranslateWeight", MFnNumericData::kFloat, 1);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);

    aBlendRotateWeight = nAttr.create("blendRotateWeight", "blendRotateWeight", MFnNumericData::kFloat, 1);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);

    aBlendScaleWeight = nAttr.create("blendScaleWeight", "blendScaleWeight", MFnNumericData::kFloat, 1);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);

    aBlendShearWeight = nAttr.create("blendShearWeight", "blendShearWeight", MFnNumericData::kFloat, 1);
    nAttr.setKeyable(true);
    nAttr.setMin(0.0);

    aBlendMatrix = cAttr.create("blendMatrix", "blendMatrix");
    cAttr.setReadable(true);
    cAttr.setArray(true);
    cAttr.addChild(aBlendInputMatrix);
    cAttr.addChild(aBlendOffsetMatrix);
    cAttr.addChild(aBlendTranslateWeight);
    cAttr.addChild(aBlendRotateWeight);
    cAttr.addChild(aBlendScaleWeight);
    cAttr.addChild(aBlendShearWeight);
    addAttribute(aBlendMatrix);
    attributeAffects(aBlendMatrix, aOutputMatrix);

    return MS::kSuccess;
}


void* WeightedBlendMatrixNode::creator() {
    return new WeightedBlendMatrixNode();
}


WeightedBlendMatrixNode::WeightedBlendMatrixNode() {
}


WeightedBlendMatrixNode::~WeightedBlendMatrixNode() {
}


MStatus WeightedBlendMatrixNode::compute(const MPlug& plug, MDataBlock& data) {
    MStatus status;

    if (plug != aOutputMatrix) {
        return MS::kUnknownParameter;
    }

    MArrayDataHandle hBlendMatrix = data.inputArrayValue(aBlendMatrix);
    int count = hBlendMatrix.elementCount();

    MatrixXd translateMatrix = MatrixXd(count, 3);
    MatrixXd rotateMatrix = MatrixXd(count, 4);
    MatrixXd scaleMatrix = MatrixXd(count, 3);
    MatrixXd shearMatrix = MatrixXd(count, 3);

    VectorXd translateWeights = VectorXd(count);
    VectorXd rotateWeights = VectorXd(count);
    VectorXd scaleWeights = VectorXd(count);
    VectorXd shearWeights = VectorXd(count);

    for (int i = 0; i < count; ++i) {
        MDataHandle hBlendMatrixElement = hBlendMatrix.inputValue();

        // extract matrix components
        MMatrix inputBlendMatrix = hBlendMatrixElement.child(aBlendInputMatrix).asMatrix();
        MMatrix offsetBlendMatrix = hBlendMatrixElement.child(aBlendOffsetMatrix).asMatrix();
        MatrixComponents components = splitMatrix(offsetBlendMatrix * inputBlendMatrix);

        // populate component matrices
        translateMatrix.row(i) = components.translate;
        rotateMatrix.row(i) = components.rotate;
        scaleMatrix.row(i) = components.scale;
        shearMatrix.row(i) = components.shear;

        // populate component weights
        translateWeights(i) = hBlendMatrixElement.child(aBlendTranslateWeight).asFloat();
        rotateWeights(i) = hBlendMatrixElement.child(aBlendRotateWeight).asFloat();
        scaleWeights(i) = hBlendMatrixElement.child(aBlendScaleWeight).asFloat();
        shearWeights(i) = hBlendMatrixElement.child(aBlendShearWeight).asFloat();

        hBlendMatrix.next();
    }

    // extract rest matrix components
    MMatrix restMatrix = data.inputValue(aRestMatrix).asMatrix();
    MatrixComponents restComponents = splitMatrix(restMatrix);

    // calculate output matrix
    Vector3d outputTranslate = calculateComponent(translateMatrix, translateWeights, restComponents.translate);
    Vector4d outputRotate = calculateComponent(rotateMatrix, rotateWeights, restComponents.rotate);
    Vector3d outputScale = calculateComponent(scaleMatrix, scaleWeights, restComponents.scale);
    Vector3d outputShear = calculateComponent(shearMatrix, shearWeights, restComponents.shear);
    MMatrix outputMatrix = constructMatrix(outputTranslate, outputRotate, outputScale, outputShear);

    // extract offset matrix
    MMatrix offsetMatrix = data.inputValue(aOffsetMatrix).asMatrix();

    MDataHandle hOut = data.outputValue(aOutputMatrix);
    hOut.setMMatrix(offsetMatrix * outputMatrix);
    hOut.setClean();

    return MS::kSuccess;
}


Vector3d WeightedBlendMatrixNode::calculateComponent(MatrixXd& matrix, VectorXd& weights, Vector3d& rest) {
    const double total = weights.sum();
    if (total == 0.0)
        return rest;
    else
        return (MatrixXd)(matrix.transpose() * (weights / total)).rowwise().sum();
}


Vector4d WeightedBlendMatrixNode::calculateComponent(MatrixXd& matrix, VectorXd& weights, Vector4d& rest) {
    const double total = weights.sum();
    if (total == 0.0)
        return rest;
    else
    {
        MatrixXd Q = matrix.transpose() * (weights / total);
        Q *= Q.transpose();

        Eigen::Index maxIndex;
        Eigen::SelfAdjointEigenSolver<MatrixXd> solver(Q);
        solver.eigenvalues().maxCoeff(&maxIndex);

        return solver.eigenvectors().col(maxIndex);
    }
}


MatrixComponents WeightedBlendMatrixNode::splitMatrix(MMatrix& matrix) {
    MTransformationMatrix transform = MTransformationMatrix(matrix);
    
    double dRotateX; double dRotateY; double dRotateZ; double dRotateW;
    double dScale[3];
    double dShear[3];

    MVector vTranslate = transform.getTranslation(MSpace::kWorld);
	transform.getRotationQuaternion(dRotateX, dRotateY, dRotateZ, dRotateW);
	transform.getScale(dScale, MSpace::kWorld);
	transform.getShear(dShear, MSpace::kWorld);

    Vector3d translate(vTranslate.x, vTranslate.y, vTranslate.z);
    Vector4d rotate(dRotateX, dRotateY, dRotateZ, dRotateW);
    Vector3d scale(dScale[0], dScale[1], dScale[2]);
    Vector3d shear(dShear[0], dShear[1], dShear[2]);

    return MatrixComponents{ translate, rotate, scale, shear };
}


MMatrix WeightedBlendMatrixNode::constructMatrix(Vector3d& translate, Vector4d& rotate, Vector3d& scale, Vector3d& shear) {
    MTransformationMatrix transform = MTransformationMatrix();

    MVector vTranslate = MVector(translate[0], translate[1], translate[2]);
    const double dScale[3] = { scale[0], scale[1], scale[2] };
	const double dShear[3] = { shear[0], shear[1], shear[2] };

    transform.setTranslation(vTranslate, MSpace::kWorld);
    transform.setRotationQuaternion(rotate[0], rotate[1], rotate[2], rotate[3]);
    transform.setScale(dScale, MSpace::kWorld);
    transform.setShear(dShear, MSpace::kWorld);

    return transform.asMatrix();
}