#ifndef INCLUDED_LIMB_H
#define INCLUDED_LIMB_H

#include "srs.h"
#include "myvec.h"

#include <OGRE/OgreMatrix4.h>

/**
*
* Models an arm or a leg. The limb is represented with 
* two spherical joints (for example, shoulder and wrist),
* and one revolute joint (for example, elbow).
* 
* This class handles coordinate transformations
* between IKAN and OGRE components.
*
*/
class Limb {

public:

	enum SIDE_OF_BODY {LEFT = 0, RIGHT = 1};
	
	/**
	* Is the limb on the right or left side of the body?
	*/
	SIDE_OF_BODY leftOrRight;

	Limb();

	/**
	* The limb is initialized with Ogre-style matrices containing
	* full transformations of shoulder, elbow and wrist joints. 
	* The class also needs to know whether the limb is on the left
	* right side of the body.
	*
	* This function should be called before calling the solve function.
	* @param shoulder full transformation of the shoulder bone
	* @param elbow full transformation of the elbow bone
	* @param wrist full transformation of the wrist bone
	* @param leftOrRight left or right arm
	*/
	virtual void init( Ogre::Matrix4 elbow, Ogre::Matrix4 wrist,float ZAxis[],bool leg);
	
	/**
	* When given a certain goal, this method returns the rotations for shoulder, elbow and
	* wrist. Before this method is called, Limb should be initialized with the function init.
	* Elbow rotation is given in radians, not in a matrix representation, because
	* the elbow can rotate only around one axis.
	*
	* @param goal A 4x4 matrix containing the goal's position (and orientation, if needed).
	* @param newShoulder This will contain the required rotation of the shoulder in order 
	* for the arm to hit the goal.
	* @param angle The angle of rotation (in radians) for the elbow from the current orientation
	* to the goal orientation.
	* @param newWrist If necessary, the wrist can also be rotated to match the rotation of the
	* goal.
	* @return 0 if the goal is not reachable.
	* 
	*/
	virtual int solve(const Ogre::Matrix4 goal, Ogre::Matrix4& newShoulder, Ogre::Matrix4& newWrist, float &angle,bool leg,float psi);
	virtual ~Limb();

protected:

	/**
	* IKAN's inverse kinematics solver.
	*/
	SRS ikSolver;
	
	/**
	* Shoulder transformation in OGRE's format
	*/
	Ogre::Matrix4 shoulderOgre;
	/**
	* Elbow transformation in OGRE's format
	*/
	Ogre::Matrix4 elbowOgre;
	/**
	* Wrist transformation in OGRE's format
	*/
	Ogre::Matrix4 wristOgre;

	/**
	* Shoulder transformation in IKAN's format
	*/
	Matrix shoulderIkan;
	/**
	* Elbow transformation in IKAN's format
	*/
	Matrix elbowIkan;
	/**
	* Wrist transformation in IKAN's format
	*/
	Matrix wristIkan;

	/**
	* Matrix converter from OGRE to IKAN. It does not change
	* the internal data, only makes it understandable
	* between the two systems.
	* @param from the original matrix in OGRE's format
	* @param to the resulting matrix in IKAN's format
	*/
	void ogreToIKAN(const Ogre::Matrix4 from, Matrix to,bool leg,bool wrist);
	void ogreToIKANg(const Ogre::Matrix4 from, Matrix to);
	void ogreToIKANwrist(const Ogre::Matrix4 from, Matrix to);
	void IKANToOgrewrist(const Matrix from, Ogre::Matrix4& to);
	/**
	* Matrix converter from IKAN to OGRE. It does not change
	* the internal data, only makes it understandable
	* between the two systems.
	* @param from the original matrix in IKAN's format
	* @param to the resulting matrix in OGRE's format
	*/
	void IKANToOgre(const Matrix from, Ogre::Matrix4& to,bool leg,bool wrist);
};

#endif