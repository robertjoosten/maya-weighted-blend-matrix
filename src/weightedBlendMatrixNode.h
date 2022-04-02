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

struct MatrixComponents { Vector3d translate; Vector4d rotate; Vector3d scale; Vector3d shear; };

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
	Vector3d calculateComponent(MatrixXd& matrix, VectorXd& weights, Vector3d& rest);
	Vector4d calculateComponent(MatrixXd& matrix, VectorXd& weights, Vector4d& rest);
	MatrixComponents splitMatrix(MMatrix& matrix);
	MMatrix constructMatrix(Vector3d& translate, Vector4d& rotate, Vector3d& scale, Vector3d& shear);
};
