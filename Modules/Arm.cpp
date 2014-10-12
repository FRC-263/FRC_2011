#include "Arm.h"
#include "../Defines.h"

Arm::Arm(void) {
	towerMotor = new Victor(TOWER_MOTOR);
	wristMotor = new Victor(WRIST_MOTOR);
	clawMotor = new Victor(CLAW_MOTOR);

	towerPot = new AnalogChannel(TOWER_POT);
	wristPot = new AnalogChannel(WRIST_POT);
	clawPot = new AnalogChannel(CLAW_POT);

	towerPID = new AfterPID(1,0,.075);
	wristPID = new AfterPID(.59,.006,0);
	clawPID = new AfterPID(.415,.01,.00002);
	
	towerLimit = new DigitalInput(14);
	
	StopArmControl = false;
}

float Arm::GetTowerPot() {
	
	return towerPot->GetVoltage();
}

float Arm::GetWristPot() {
	return wristPot->GetVoltage();
}

float Arm::GetClawPot() {
	return clawPot->GetVoltage();
}

bool Arm::SetStartingPosition()
{
	StopArmControl = true;
	float tower_voltage = towerPID->GetOutput(GetTowerPot(), TOWER_END, 0,
				.03) * -1;
	clawPID->SetPID(.35,.01,.00001);
	float claw_voltage = clawPID->GetOutput(GetClawPot(), CLAW_START, 0, .15);
	float wrist_voltage = wristPID->GetOutput(GetWristPot(), WRIST_MIN, 0,
			.05) * -1;
	
	if(tower_voltage == 0 && claw_voltage == 0 && wrist_voltage == 0)
	{
		StopArmControl = false;
		towerMotor->Set(0);
		wristMotor->Set(0);
		clawMotor->Set(0);
		return true;
	}
	else
	{
		towerMotor->Set(tower_voltage);
		wristMotor->Set(wrist_voltage);
		clawMotor->Set(claw_voltage);
	}
	return false;
}

bool Arm::SetClawClose()
{
	clawPID->SetPID(.5,.01,.00001);
	float claw_voltage = clawPID->GetOutput(GetClawPot(), AUTO_CLAW_SCORING, 0, .1);
	if(claw_voltage == 0)
	{
		clawMotor->Set(0);
		return true;
	}
	else
		clawMotor->Set(claw_voltage);
	return false;
}

bool Arm::RaiseTower()
{
		float tower_voltage = towerPID->GetOutput(GetTowerPot(), AUTO_TOWER_SCORING_UP, 0,
					.03) * -1;
		
		if(tower_voltage == 0)
		{
			towerMotor->Set(0);
			return true;
		}
		else
		{
			towerMotor->Set(tower_voltage);
		}
		return false;
}

bool Arm::SetScoringPosition()
{
	float claw_voltage = clawPID->GetOutput(GetClawPot(), AUTO_CLAW_SCORING, 0, .3);
	float wrist_voltage = wristPID->GetOutput(GetWristPot(), AUTO_WRIST_SCORING, 0,
			.2) * -1;
	
	if(claw_voltage == 0 && wrist_voltage == 0)
	{
		wristMotor->Set(0);
		clawMotor->Set(0);
		return true;
	}
	else
	{
		wristMotor->Set(wrist_voltage);
		clawMotor->Set(claw_voltage);
	}
	return false;
}

bool Arm::ScoreTube()
{
	float tower_voltage = towerPID->GetOutput(GetTowerPot(), AUTO_TOWER_SCORING_DOWN, 0,
				.03) * -1;
	float claw_voltage = clawPID->GetOutput(GetClawPot(), CLAW_OPEN, 0, .1);
	
	if(tower_voltage == 0 && claw_voltage == 0)
	{
		towerMotor->Set(0);
		clawMotor->Set(0);
		return true;
	}
	else
	{
		towerMotor->Set(tower_voltage);
		clawMotor->Set(claw_voltage);
	}
	return false;
}

void Arm::DriveArm() {
	float claw_target= CLAW_OPEN;

	DriverStation *ds = DriverStation::GetInstance();

	if(ds->GetDigitalIn(3) && !ds->GetDigitalIn(4))
	{
		float tower_target = (ds->GetEnhancedIO().GetAnalogInRatio(1) * (TOWER_MAX - TOWER_MIN)) + TOWER_MIN;
		float wrist_target = (ds->GetEnhancedIO().GetAnalogInRatio(3) * (WRIST_MAX-WRIST_MIN)) + WRIST_MIN;
	
		if (tower_target > TOWER_MAX)
			tower_target = TOWER_MAX;
		else if (tower_target < TOWER_MIN)
			tower_target = TOWER_MIN;
	
		if (wrist_target < WRIST_MIN)
			wrist_target = WRIST_MIN;
		if (wrist_target > WRIST_MAX)
			wrist_target = WRIST_MAX;
	
		if (ds->GetDigitalIn(1)) {
			claw_target = CLAW_CLOSED;
		}
	
		float tower_voltage = towerPID->GetOutput(GetTowerPot(), tower_target, 0,
				.03) * -1;
		float claw_voltage = clawPID->GetOutput(GetClawPot(), claw_target, 0, .1);
		float wrist_voltage = wristPID->GetOutput(GetWristPot(), wrist_target, 0,
				.2) * -1;
		
		if(!towerLimit->Get() && tower_voltage < 0)
		{
			tower_voltage = 0;
			towerMotor->Set(0);
		}
		
		if(fabs(claw_voltage) > .5)
			wrist_voltage = 0;
		
		towerMotor->Set(tower_voltage);
		wristMotor->Set(wrist_voltage);
		clawMotor->Set(claw_voltage);
	}
	else if(ds->GetDigitalIn(3) && ds->GetDigitalIn(4))
	{
		float tower_voltage = towerPID->GetOutput(GetTowerPot(), TOWER_AUTO_DOWN, 0,
						.03) * -1;
		float claw_voltage = clawPID->GetOutput(GetClawPot(), CLAW_OPEN, 0, .1);
		float wrist_voltage = wristPID->GetOutput(GetWristPot(), WRIST_AUTO_DOWN, 0,
				.2) * -1;
		
		towerMotor->Set(tower_voltage);
		wristMotor->Set(wrist_voltage);
		clawMotor->Set(claw_voltage);
	}
	else if(!ds->GetDigitalIn(3) && ds->GetDigitalIn(4))
	{
		float tower_voltage = towerPID->GetOutput(GetTowerPot(), TOWER_AUTO_UP, 0,
						.03) * -1;
		float claw_voltage = clawPID->GetOutput(GetClawPot(), CLAW_CLOSED, 0, .1);
		float wrist_voltage = wristPID->GetOutput(GetWristPot(), WRIST_AUTO_UP, 0,
				.2) * -1;
		
		towerMotor->Set(tower_voltage);
		wristMotor->Set(wrist_voltage);
		clawMotor->Set(claw_voltage);
	}
}
