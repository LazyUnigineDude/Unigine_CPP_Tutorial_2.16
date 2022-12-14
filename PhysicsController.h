#pragma once

#include <Unigine.h>


class PhysicsController : public Unigine::ComponentBase
{ 

public:
	COMPONENT_DEFINE(PhysicsController, ComponentBase)
		COMPONENT_INIT(Init)
		COMPONENT_UPDATE_PHYSICS(UpdatePhysics)

		PROP_PARAM(Float, Max_Speed, 10)
		PROP_PARAM(Vec4, FBLR_Speed)
		double getSpeed();

protected:

	void Init();
	void UpdatePhysics();


private:

	Unigine::BodyRigidPtr MainCharacter;
	void AutoRotate();
};
