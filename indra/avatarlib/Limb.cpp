#include "Limb.h"
#include "myvec.h"
Limb::Limb() {
}

void Limb::init( Ogre::Matrix4 elbow, Ogre::Matrix4 wrist,float ZAxis[],bool leg) 
{
     
					
	int i, j;

	Limb::ogreToIKAN(elbow, this->elbowIkan,leg,false);
	Limb::ogreToIKAN(wrist, this->wristIkan,leg,true);
  
	Ogre::Matrix4 wr,el;
	
//The axis depend on the limb
//	float ZAxis[] = {0, 0, 1}; 
	float XAxis[] = {1, 0, 0};


	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {

			el[i][j] = (float)this->elbowIkan[i][j];
		}
	}

	//change from row to column matrix
	el = el.transpose();

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			wr[i][j] = (float)this->wristIkan[i][j];
			

		}
	}

	//change from row to column matrix
	wr = wr.transpose();
        
	this->ikSolver.init(this->elbowIkan, this->wristIkan, ZAxis, XAxis);
}


int Limb::solve(const Ogre::Matrix4 goal, Ogre::Matrix4& newShoulder, Ogre::Matrix4& newWrist, float &angle,bool leg,float psi) {
	Ogre::Matrix4 newR12,g;
	Matrix goalIkan;
	this->ogreToIKANg(goal, goalIkan);
	int result = 0;

	Matrix newR1, newR2;
	
	

	result = this->ikSolver.SetGoal(goalIkan, angle);
/*     float p[3];
	this->ikSolver.AngleToPos(0,p);*/

    this->ikSolver.SolveR1R2(psi,newR1,newR2);
//	this->ikSolver.SolveR1R2(angle, newR1, newR2);

	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			newR12[i][j] = (float)newR1[i][j];
			g[i][j]=(float)goalIkan[i][j];
		}
	}

	//change from row to column matrix
	newR12 = newR12.transpose();
	g=g.transpose();

	this->IKANToOgre(newR1, newShoulder,leg,false);
	
	this->IKANToOgre(newR2, newWrist,leg,true);
	

	return result;
}


Limb::~Limb(){
}


void Limb::ogreToIKAN(const Ogre::Matrix4 from, Matrix to,bool leg,bool wrist) {


Ogre::Matrix4 R_IKAN;
	if (leg)
	
		R_IKAN = Ogre::Matrix4(0, 1, 0, 0,
							   0, 0, 1, 0,
							   1, 0, 0, 0,
							   0, 0, 0, 1);
	else{
		if (wrist) R_IKAN=Ogre::Matrix4(0, 0, 1, 0,
										0, -1, 0, 0,
										1, 0, 0, 0,
										0, 0, 0, 1);

		else R_IKAN=Ogre::Matrix4(0, -1, 0, 0,
							      0, 0, -1, 0,
		                          1, 0, 0, 0,
						          0, 0, 0, 1);
	}


	//change the rotation part to the new coordinate system
	Ogre::Matrix3 rotation, newRotation;
	from.extract3x3Matrix(rotation);
	Ogre::Radian ogreZ, ogreY, ogreX;


	if((leg)||(!wrist))	rotation.ToEulerAnglesYZX(ogreY, ogreZ, ogreX);
		else rotation.ToEulerAnglesZYX(ogreZ, ogreY, ogreX);
	
	Ogre::Radian ikanZ, ikanY, ikanX;
	

//Shoulder &&elbow
	/*ikanZ = ogreX;
	ikanY = -ogreZ;
	ikanX = -ogreY;*/

//Leg
	if (leg){
	ikanZ = ogreX;
	ikanY = ogreZ;
	ikanX = ogreY;
	}
	else{
		if (wrist) {
			ikanZ = ogreX;
			ikanY = -ogreY;
			ikanX = ogreZ;
		}
		else
		{ikanZ = ogreX;
		ikanY = -ogreZ;
		ikanX = -ogreY;
		}
	}
	
	newRotation.FromEulerAnglesXYZ(ikanX, ikanY, ikanZ);


	Ogre::Matrix4 tempMatrix = Ogre::Matrix4(newRotation);

	//change the translation part to the new coordinate system
	Ogre::Vector3 translation, newTranslation;
	translation = from.getTrans();
	newTranslation = R_IKAN * translation;	
	tempMatrix.setTrans(newTranslation);

	//switch from column-major to row-major
	tempMatrix = tempMatrix.transpose();

	//write the results
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			to[i][j] = (float)tempMatrix[i][j];
		}
	}
}

void Limb::IKANToOgre(const Matrix from, Ogre::Matrix4& to,bool leg,bool wrist) {

Ogre::Matrix4 R_OGRE;
if (leg)
	R_OGRE= Ogre::Matrix4(	0, 0, 1, 0,
						    1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 0, 1); 
else {
	if(wrist) R_OGRE=Ogre::Matrix4(0, 0, 1, 0,
								   0, 1, 0, 0,
		                           1, 0, 0, 0,
		                           0, 0, 0, 1); 

	else R_OGRE= Ogre::Matrix4(0, 0, 1, 0,
						      -1, 0, 0, 0,
					           0, -1, 0, 0,
						       0, 0, 0, 1);
}


	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			to[i][j] = (float)from[i][j];
		}
	}

	//change from row to column matrix
	to = to.transpose();

	//Change the rotation part
	Ogre::Matrix3 rotation, newRotation;
	to.extract3x3Matrix(rotation);
	Ogre::Radian ikanZ, ikanY, ikanX;
	Ogre::Radian ogreZ, ogreY, ogreX;

	rotation.ToEulerAnglesXYZ(ikanX, ikanY, ikanZ);

	if(leg){
	ogreZ = ikanY;
	ogreY = ikanX;
	ogreX = ikanZ;
	}
	else{ 
		if (wrist)
		{
		ogreZ = ikanX;
	    ogreY = ikanY;
	    ogreX = ikanZ;
		}
		else{
		ogreZ = -ikanY;
		ogreY = -ikanX;
		ogreX = ikanZ;
		}}
	//Change the translation part
	Ogre::Vector3 translation = to.getTrans();
	Ogre::Vector3 newTranslation = R_OGRE * translation;

	if((leg)||(!wrist)) newRotation.FromEulerAnglesYZX(ogreY, ogreZ, ogreX);
	else newRotation.FromEulerAnglesZYX(ogreZ, ogreY, ogreX);

	to = Ogre::Matrix4(newRotation);
	to.setTrans(newTranslation);
}
void Limb::ogreToIKANg(const Ogre::Matrix4 from, Matrix to) {

	//Ogre coordinates system:
	Ogre::Matrix4 R_IKAN = Ogre::Matrix4(0, 0, -1, 0,
		1, 0, 0, 0,
		0, -1, 0, 0,
		0, 0, 0, 1);
		

	//change the rotation part to the new coordinate system
	Ogre::Matrix3 rotation, newRotation;
	from.extract3x3Matrix(rotation);
	Ogre::Radian ogreZ, ogreY, ogreX;

	rotation.ToEulerAnglesZXY(ogreZ, ogreX, ogreY);
	Ogre::Radian ikanZ, ikanY, ikanX;
	
	ikanZ = -ogreY;
	ikanY = ogreX;
	ikanX = -ogreZ;


	
	newRotation.FromEulerAnglesXYZ(ikanX, ikanY, ikanZ);
	Ogre::Matrix4 tempMatrix = Ogre::Matrix4(newRotation);

	//change the translation part to the new coordinate system
	Ogre::Vector3 translation, newTranslation;
	translation = from.getTrans();
	newTranslation = R_IKAN * translation;	
	tempMatrix.setTrans(newTranslation);

	//switch from column-major to row-major
	tempMatrix = tempMatrix.transpose();

	//write the results
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			to[i][j] = (float)tempMatrix[i][j];
		}
	}
}
void Limb::ogreToIKANwrist(const Ogre::Matrix4 from, Matrix to) {


//Left wrist		
  /*Ogre::Matrix4 R_IKAN = Ogre::Matrix4(0, 0, 1, 0,
		0, 1, 0, 0,
		1, 0, 0, 0,
		0, 0, 0, 1);*/
//Right wrist
Ogre::Matrix4 R_IKAN = Ogre::Matrix4(0, 0, 1, 0,
		0, -1, 0, 0,
		1, 0, 0, 0,
		0, 0, 0, 1);

	//change the rotation part to the new coordinate system
	Ogre::Matrix3 rotation, newRotation;
	from.extract3x3Matrix(rotation);
	Ogre::Radian ogreZ, ogreY, ogreX;


	rotation.ToEulerAnglesZYX(ogreZ, ogreY, ogreX);
	Ogre::Radian ikanZ, ikanY, ikanX;
	


	ikanZ = ogreX;
	ikanY = -ogreY;
	ikanX = ogreZ;
	

	newRotation.FromEulerAnglesXYZ(ikanX, ikanY, ikanZ);

	Ogre::Matrix4 tempMatrix = Ogre::Matrix4(newRotation);

	//change the translation part to the new coordinate system
	Ogre::Vector3 translation, newTranslation;
	translation = from.getTrans();
	newTranslation = R_IKAN * translation;	
	tempMatrix.setTrans(newTranslation);

	//switch from column-major to row-major
	tempMatrix = tempMatrix.transpose();

	//write the results
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			to[i][j] = (float)tempMatrix[i][j];
		}
	}
}
void Limb::IKANToOgrewrist(const Matrix from, Ogre::Matrix4& to) {



Ogre::Matrix4 R_OGRE = Ogre::Matrix4(0, 0, 1, 0,
		0, 1, 0, 0,
		1, 0, 0, 0,
		0, 0, 0, 1); 

	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			to[i][j] = (float)from[i][j];
		}
	}

	//change from row to column matrix
	to = to.transpose();

	//Change the rotation part
	Ogre::Matrix3 rotation, newRotation;
	to.extract3x3Matrix(rotation);
	Ogre::Radian ikanZ, ikanY, ikanX;
	Ogre::Radian ogreZ, ogreY, ogreX;


	rotation.ToEulerAnglesXYZ(ikanX, ikanY, ikanZ);


	ogreZ = ikanX;
	ogreY = ikanY;
	ogreX = ikanZ;

	//Change the translation part
	Ogre::Vector3 translation = to.getTrans();
	Ogre::Vector3 newTranslation = R_OGRE * translation;


	newRotation.FromEulerAnglesZYX(ogreZ, ogreY, ogreX);

	to = Ogre::Matrix4(newRotation);
	to.setTrans(newTranslation);
}