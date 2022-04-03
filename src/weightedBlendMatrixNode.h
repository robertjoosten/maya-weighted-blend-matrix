#include <maya/MPxNode.h>
#include <maya/MTypes.h>
#include <maya/MStatus.h>
#include <maya/MMatrix.h>
#include <maya/MVector.h>
#include <maya/MQuaternion.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>


using namespace Eigen;


struct MatrixComponents { 
	Vector3d translate; 
	Vector4d rotate;
	Vector3d scale; 
	Vector3d shear; 
};


class WeightedBlendMatrixNode : public MPxNode {
public:
	WeightedBlendMatrixNode();
	virtual ~WeightedBlendMatrixNode();
	static void* creator();

	virtual MStatus compute(const MPlug& plug, MDataBlock& data);

	static MStatus initialize();
	static MTypeId id;
	static const MString kName;
	static MObject aOutputMatrix;
	static MObject aOffsetMatrix;
	static MObject aRestMatrix;
	static MObject aBlendInputMatrix;
	static MObject aBlendOffsetMatrix;
	static MObject aBlendTranslateWeight;
	static MObject aBlendRotateWeight;
	static MObject aBlendScaleWeight;
	static MObject aBlendShearWeight;
	static MObject aBlendMatrix;
private:
	Vector3d calculateComponent(const MatrixXd& matrix, const VectorXd& weights, const Vector3d& rest);
	Vector4d calculateComponent(const MatrixXd& matrix, const VectorXd& weights, const Vector4d& rest);
	MatrixComponents splitMatrix(const MMatrix& matrix);
	MMatrix constructMatrix(
		const Vector3d& translate, 
		const Vector4d& rotate, 
		const Vector3d& scale, 
		const Vector3d& shear
	);
};
