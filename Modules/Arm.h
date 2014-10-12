#ifndef ARM_H
#define ARM_H

#include "WPILib.h"
#include "../AfterLib/AfterPID.h"
#include <math.h>

class Arm {
private:
	Victor *towerMotor;
	Victor *wristMotor;
	Victor *clawMotor;

	AnalogChannel *towerPot;
	AnalogChannel *wristPot;
	AnalogChannel *clawPot;

	AfterPID *towerPID;
	AfterPID *wristPID;
	AfterPID *clawPID;

	DigitalInput *towerLimit;
	
	bool StopArmControl;
	
public:
	Arm(void);
	~Arm(void);

	void DriveArm();

	float GetTowerPot();
	float GetWristPot();
	float GetClawPot();
	
	bool SetStartingPosition();
	bool SetClawClose();
	bool RaiseTower();
	bool SetScoringPosition();
	bool ScoreTube();
};

#endif
