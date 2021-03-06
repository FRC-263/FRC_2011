#include "TankDrive.h"

extern JagData* jagData;

TankDrive::TankDrive(void) {
	leftBlackMotor = new CANJaguar(LEFTBLACKMOTOR, CANJaguar::kPercentVbus);
	leftGreyMotor = new CANJaguar(LEFTGREYMOTOR, CANJaguar::kPercentVbus);
	rightBlackMotor = new CANJaguar(RIGHTBLACKMOTOR, CANJaguar::kPercentVbus);
	rightGreyMotor = new CANJaguar(RIGHTGREYMOTOR, CANJaguar::kPercentVbus);

	leftGreyPID = new AfterPID(.1, .001, .001);
	rightGreyPID = new AfterPID(.1, .001, .001);

	leftStick = new Joystick(LEFTSTICK);
	rightStick = new Joystick(RIGHTSTICK);

	kPercentVbusLeft = 0;
	kPercentVbusRight = 0;
}

TankDrive::~TankDrive(void) {
	delete leftBlackMotor;
	delete leftGreyMotor;
	delete rightBlackMotor;
	delete rightGreyMotor;
	delete leftStick;
	delete rightStick;
}

void TankDrive::SetupDriveTrain() {
	SetupJaguar(leftBlackMotor);
	SetupJaguar(rightBlackMotor);
	SetupJaguar(leftGreyMotor);
	SetupJaguar(rightGreyMotor);

	leftBlackMotor->SetSafetyEnabled(false);
	rightBlackMotor->SetSafetyEnabled(false);
	leftGreyMotor->SetSafetyEnabled(false);
	rightGreyMotor->SetSafetyEnabled(false);
}

void TankDrive::SetupJaguar(CANJaguar *jag, UINT16 codesPerRev) {
	jag->ConfigEncoderCodesPerRev(codesPerRev);
	jag->ConfigNeutralMode(CANJaguar::kNeutralMode_Brake);
	jag->SetSpeedReference(CANJaguar::kSpeedRef_QuadEncoder);
	jag->SetPositionReference(CANJaguar::kPosRef_QuadEncoder);
	//jag->SetPID(CAN_PID_P, CAN_PID_I, CAN_PID_D);
	jag->EnableControl();
}

void TankDrive::SetJagData() {
	jagData->leftBlackBusVoltage = leftBlackMotor->GetBusVoltage();
	jagData->leftBlackCurrent = leftBlackMotor->GetOutputCurrent();
	jagData->leftBlackOutVoltage = leftBlackMotor->GetOutputVoltage();
	jagData->leftBlackTemp = leftBlackMotor->GetTemperature();

	jagData->leftGreyBusVoltage = leftGreyMotor->GetBusVoltage();
	jagData->leftGreyCurrent = leftGreyMotor->GetOutputCurrent();
	jagData->leftGreyOutVoltage = leftGreyMotor->GetOutputVoltage();
	jagData->leftGreyTemp = leftGreyMotor->GetTemperature();

	jagData->rightBlackBusVoltage = rightBlackMotor->GetBusVoltage();
	jagData->rightBlackCurrent = rightBlackMotor->GetOutputCurrent();
	jagData->rightBlackOutVoltage = rightBlackMotor->GetOutputVoltage();
	jagData->rightBlackTemp = rightBlackMotor->GetTemperature();

	jagData->rightGreyBusVoltage = rightGreyMotor->GetBusVoltage();
	jagData->rightGreyCurrent = rightGreyMotor->GetOutputCurrent();
	jagData->rightGreyOutVoltage = rightGreyMotor->GetOutputVoltage();
	jagData->rightGreyTemp = rightGreyMotor->GetTemperature();

	jagData->leftSpeed = leftBlackMotor->GetSpeed();
	jagData->rightSpeed = rightBlackMotor->GetSpeed();
}

void TankDrive::Reset()
{
	leftBlackMotor->DisableControl();
	leftBlackMotor->EnableControl();
}

void TankDrive::Drive() {
	// Feed left positive, right negative to go forward
	Drive(leftStick->GetY() * -1, rightStick->GetY());
}

float TankDrive::GetPosition()
{
	return leftBlackMotor->GetPosition();
}

bool TankDrive::AutoDrive(float distance, bool backwards, int fleft, int fright)
{
	if(leftBlackMotor->GetPosition() < distance && !backwards)
	{
		float left = 0.3 * 1.5;
		float right = -0.3 * 1.5;
		
		if(fleft == 0)
		{
			left -= 0.2;
			right -= .2;
		}
		if(fright == 0)
		{
			right += 0.2;
			left += 0.2;
		}
		
		Drive(left, right);
		UpdateSyncGroups();
		return false;
	}
	else if(leftBlackMotor->GetPosition() > distance  && backwards)
	{
	 	Drive(-.5, .5);
		UpdateSyncGroups();
		return false;
	}
	else
	{
		Drive(0,0);
		UpdateSyncGroups();
		return true;
	}
}

void TankDrive::UpdateSyncGroups() {
	leftBlackMotor->UpdateSyncGroup(1);
	leftGreyMotor->UpdateSyncGroup(1);
	rightBlackMotor->UpdateSyncGroup(1);
	rightGreyMotor->UpdateSyncGroup(1);
}

void TankDrive::Drive(float leftY, float rightY) {	
	kPercentVbusLeftDelta = (leftGreyPID->GetOutput(leftBlackMotor->Get(),
			(leftY), 0, .001));
	kPercentVbusLeft += kPercentVbusLeftDelta;

	if (kPercentVbusLeft > 1)
		kPercentVbusLeft = 1;
	else if (kPercentVbusLeft < -1)
		kPercentVbusLeft = -1;

	if (fabs(leftY) < JOYSTICK_ERROR_OFFSET)
		kPercentVbusLeft = 0;

	kPercentVbusRightDelta = (rightGreyPID->GetOutput(
			rightBlackMotor->Get(), 
			(rightY), 0, .001));
	kPercentVbusRight+= kPercentVbusRightDelta;

	if (kPercentVbusRight > 1)
		kPercentVbusRight = 1;
	else if (kPercentVbusRight < -1)
		kPercentVbusRight = -1;

	if (fabs(rightY) < JOYSTICK_ERROR_OFFSET)
		kPercentVbusRight = 0;

	if (fabs(leftY) > JOYSTICK_ERROR_OFFSET) {
		leftBlackMotor->Set(kPercentVbusLeft, 1);
		leftGreyMotor->Set(kPercentVbusLeft, 1);
	} else {
		leftBlackMotor->Set(0, 1);
		leftGreyMotor->Set(0, 1);
	}

	if (fabs(rightY) > JOYSTICK_ERROR_OFFSET) {
		rightBlackMotor->Set(kPercentVbusRight, 1);
		rightGreyMotor->Set(kPercentVbusRight, 1);
	} else {
		rightBlackMotor->Set(0, 1);
		rightGreyMotor->Set(0, 1);
	}
}

Joystick* TankDrive::GetLeftJoystick()
{
	return leftStick;
}

Joystick* TankDrive::GetRightJoystick()
{
	return rightStick;
}
