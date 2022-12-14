#include "ShooterAI.h"
REGISTER_COMPONENT(ShooterAI)
#include "PhysicsController.h"

void ShooterAI::Init() {

	//Path = getComponent<PathMaker>(PathMakerNode);
	//Path->InitPath();

	ObjPath = getComponent<PathMaker>(ObstacleNode);
	ObjPath->InitPath();

	Health = getComponent<HealthBar>(node);
	CurrentHealth = 15;
	DodgeArea = Unigine::static_ptr_cast<Unigine::PhysicalTrigger>(PhysicalTriggerNode.get());
	DodgeArea->addEnterCallback(Unigine::MakeCallback(this, &ShooterAI::GetObjectEnteredInArea));
	ObstaclePtr = Unigine::static_ptr_cast<Unigine::ObstacleBox>(ObstacleNode->getChild(0));
	NavMeshCheck();
}

void ShooterAI::Update() {

	// Calculate the Area, Position and Rotation
	Unigine::Math::mat4 Frustum(Unigine::Math::perspective(40, 1.4f, 0.05, FieldDistance));
		Unigine::Math::quat Rotation(node->getWorldRotation() * Unigine::Math::quat(90, 0, 0));
		Unigine::Math::Mat4 View(Rotation, node->getChild(0)->getWorldPosition());
	
		// Render
		Unigine::Visualizer::renderFrustum(Frustum, View, Unigine::Math::vec4_black);
		Unigine::Visualizer::renderVector(node->getWorldPosition(), node->getWorldPosition() + Unigine::Math::Vec3(node->getWorldDirection(Unigine::Math::AXIS_Y)) * 2, Unigine::Math::vec4_black);

		// Logic
		BFrustum = Unigine::Math::BoundFrustum(Frustum, Unigine::Math::inverse(Unigine::Math::mat4(View)));
		if (BFrustum.inside(Unigine::Math::vec3(MainCharacter->getWorldPosition()))) {

			Unigine::ObjectPtr x = Unigine::World::getIntersection(node->getWorldPosition(), MainCharacter->getWorldPosition(), 1);
			Unigine::Visualizer::renderLine3D(node->getChild(0)->getWorldPosition(), MainCharacter->getWorldPosition(), Unigine::Math::vec4_red);
			if (x->getName() == MainCharacter->getChild(0)->getName()) {
				isVisible = true;
				float distance = Unigine::Math::distance(Unigine::Math::vec3(MainCharacter->getWorldPosition()), Unigine::Math::vec3(node->getWorldPosition()));
				DistanceFactor = distance / FieldDistance;
			}
			else isVisible = false;
		}

		AiState();
		//Path->RenderPath();
		ObjPath->MoveAlongPath();
		ObjPath->MoveObject(ObstacleNode);
		Unigine::Visualizer::renderSphere(DodgeArea->getSize().x, DodgeArea->getWorldTransform(), Unigine::Math::vec4_red);
}

void ShooterAI::NavMeshCheck() {
	
	NavMesh = Unigine::static_ptr_cast<Unigine::NavigationMesh>(NavigationNode.get());

	PathPoints1 = NavigationNode->getChild(0)->getWorldPosition();
	PathPoints2 = NavigationNode->getChild(1)->getWorldPosition();
	PathPoints3 = NavigationNode->getChild(2)->getWorldPosition();
	PathPoints3 = NavigationNode->getChild(3)->getWorldPosition();

	Pathing1 = Unigine::PathRoute::create();
	Pathing2 = Unigine::PathRoute::create();
	Pathing3 = Unigine::PathRoute::create();
	Pathing4 = Unigine::PathRoute::create();

	Pathing1->setRadius()

}

void ShooterAI::GetObjectEnteredInArea(Unigine::BodyPtr Body) {

	if (Body->getObject()->getName() != node->getName()) {
		Unigine::Log::message("%s Entered\n", Body->getObject()->getName());

		Unigine::ObjectPtr x = Unigine::World::getIntersection(Body->getPosition(), Body->getPosition() + Unigine::Math::Vec3(Body->getObject()->getBodyLinearVelocity()), 1);
		Unigine::Visualizer::renderLine3D(Body->getPosition(), Body->getPosition() + Unigine::Math::Vec3(Body->getObject()->getBodyLinearVelocity()), Unigine::Math::vec4_red, 1);

		if (x && x->getName() == node->getName() && STATE == DODGE) {

			Unigine::Math::Vec3 MainVector = Unigine::Math::Vec3(Body->getObject()->getBodyLinearVelocity()).normalize();
			float Angle = Unigine::Math::getAngle(Unigine::Math::vec3(node->getWorldPosition()) + node->getWorldDirection(), Unigine::Math::vec3(MainVector) ,Unigine::Math::vec3_up);
			Unigine::Log::message("Angle: %f\n", Angle);
			(Angle > 0) ? Angle = 90.0f : Angle = -90.0f;
			Unigine::Log::message("New Angle: %f\n", Angle);
			Unigine::Math::Vec3 RotatedVector = Unigine::Math::Vec3(
				MainVector.x * Unigine::Math::cos(Angle) - MainVector.y * Unigine::Math::sin(Angle),
				MainVector.x * Unigine::Math::sin(Angle) + MainVector.y * Unigine::Math::cos(Angle),
				0).normalize();
			Unigine::Visualizer::renderVector(Body->getPosition(), Body->getPosition() - MainVector, Unigine::Math::vec4_green, 0.25f, true, 1);
			Unigine::Visualizer::renderVector(Body->getPosition(), Body->getPosition() - RotatedVector, Unigine::Math::vec4_green, 0.25f, true, 1);
			node->setWorldPosition(node->getWorldPosition() + RotatedVector * 10);
		}
	}
}

void ShooterAI::AiState() {

	//if (CurrentHealth != Health->GetHealth()) { ChangeState(AGGRESSIVE); Weight = 1.0f; CurrentHealth = Health->GetHealth(); }

	switch (STATE)
	{
	case ShooterAI::IDLE:
		//Unigine::Log::message("IDLE\n");
		Weight = Unigine::Math::clamp(Weight -= Unigine::Game::getIFps(), 0.0f, 1.0f);
		//if (isVisible) { ChangeState(CurrentState::ALERT); }
			/*if (Unigine::Math::distance(Unigine::Math::vec3(node->getWorldPosition()), Unigine::Math::vec3(Path->GetCurrentPathPosition())) > 0.1f) {
				RotateTowards(Path->GetCurrentPathPosition(), node, 0.05f);
				MoveTowards(Path->GetCurrentPathPosition(), node, 1);
			}
			else {
				Path->MoveAlongPath();
				Path->MoveObject(node);
			}*/
		Pathing1->create2D(PathPoints1, PathPoints2);
		Pathing2->create2D(PathPoints2, PathPoints3);
		Pathing3->create2D(PathPoints3, PathPoints4);
		Pathing4->create2D(PathPoints4, PathPoints1);
		NavMesh->renderVisualizer();

		if (Pathing1->isReached()) {
			Pathing1->renderVisualizer(Unigine::Math::vec4_white);
		}
		if (Pathing2->isReached()) {
			Pathing2->renderVisualizer(Unigine::Math::vec4_white);
		}
		if (Pathing3->isReached()) {
			Pathing3->renderVisualizer(Unigine::Math::vec4_white);
		}
		if (Pathing4->isReached()) {
			Pathing4->renderVisualizer(Unigine::Math::vec4_white);
		}
		ObstaclePtr->renderVisualizer();
		break;
	case ShooterAI::ALERT:
		//Unigine::Log::message("ALRT\n");
		Weight = Unigine::Math::clamp(Weight += Unigine::Game::getIFps() / DistanceFactor, 0.0f, 1.0f);
		if (Weight == 1.0f) { ChangeState(CurrentState::AGGRESSIVE); }
		if (!isVisible) { ChangeState(CurrentState::IDLE); }
			RotateTowards(MainCharacter->getWorldPosition(), node, 0.005f);
		break;
	case ShooterAI::SEARCH:
		//Unigine::Log::message("SRCH\n");
		Weight = Unigine::Math::clamp(Weight -= Unigine::Game::getIFps() / 5, 0.0f, 1.0f);
		if (Weight == 0.0f) { ChangeState(CurrentState::IDLE); }
		if (isVisible) { ChangeState(CurrentState::AGGRESSIVE); Weight = 1; }
			RotateTowards(MainCharacter->getWorldPosition(), node, 0.05f);
			MoveTowards(MainCharacter->getWorldPosition(), node, 3);
		break;
	case ShooterAI::AGGRESSIVE:
		//Unigine::Log::message("AGRO\n");
		if (!isVisible) { ChangeState(CurrentState::SEARCH); }
			RotateTowards(MainCharacter->getWorldPosition(), node, 0.05f);
			MoveTowards(MainCharacter->getWorldPosition(), node, 5);
			if (Unigine::Math::distance(Unigine::Math::vec3(node->getWorldPosition()), Unigine::Math::vec3(MainCharacter->getWorldPosition())) < 20.0f) { ChangeState(SHOOT); CurrentTime = Unigine::Game::getTime(); }
		break;
	case ShooterAI::SHOOT:
			if (CurrentTime + 1 < Unigine::Game::getTime()) { Shoot(); ChangeState(AGGRESSIVE); }
			RotateTowards(MainCharacter->getWorldPosition(), node, 0.02f);
			Unigine::Visualizer::renderLine3D(node->getWorldPosition(), node->getWorldPosition() + Unigine::Math::Vec3(node->getWorldDirection(Unigine::Math::AXIS_Y)) * 20, Unigine::Math::vec4_blue);
		break;
	case ShooterAI::DODGE:
		break;
	default: break;
	}
}

void ShooterAI::RotateTowards(Unigine::Math::Vec3 RotateTowards, Unigine::NodePtr Obj2Move, float RoatateSpeed) {

	Unigine::Math::vec3 Vec1 = Obj2Move->getWorldDirection(Unigine::Math::AXIS_Y),
						Vec2 = Unigine::Math::vec3((RotateTowards - Obj2Move->getWorldPosition())).normalize();

	float Angle = Unigine::Math::getAngle(Vec1, Vec2, Unigine::Math::vec3_up);
	Obj2Move->rotate(-Obj2Move->getWorldRotation().x, -Obj2Move->getWorldRotation().y, Angle * RoatateSpeed);
}

void ShooterAI::MoveTowards(Unigine::Math::Vec3 MoveTowards, Unigine::NodePtr Obj2Move, int Speed) {

	Unigine::Math::Vec3 Pos = Unigine::Math::lerp(
		Obj2Move->getWorldPosition(),
		MoveTowards,
		Unigine::Game::getIFps() * Speed /
		Unigine::Math::distance(Unigine::Math::vec3(Obj2Move->getWorldPosition()), Unigine::Math::vec3(MoveTowards)));
	Obj2Move->setWorldPosition(Pos);
}

void ShooterAI::Shoot() {

	Unigine::Math::Vec3 FuturePoint = MainCharacter->getWorldPosition() + Unigine::Math::Vec3(MainCharacter->getBodyLinearVelocity().normalize());
	FuturePoint.z = 1;
	Unigine::Visualizer::renderCapsule(0.5f, 1.5f, Unigine::Math::dmat4(MainCharacter->getWorldRotation(), FuturePoint), Unigine::Math::vec4_black,2);
	Unigine::Visualizer::renderPoint3D(FuturePoint, 0.1f, Unigine::Math::vec4_red,false,2);

	Unigine::NodePtr _Bullet = Unigine::World::loadNode(BulletPrefab.get());
	_Bullet->setWorldPosition(node->getChild(0)->getWorldPosition());

	Bullet* bullet = getComponent<Bullet>(_Bullet);
	bullet->setDamage(1);
	Unigine::BodyRigidPtr _BulletPhysics = _Bullet->getObjectBodyRigid();

	double FutureDistance = Unigine::Math::distance(Unigine::Math::vec3(MainCharacter->getWorldPosition()), Unigine::Math::vec3(FuturePoint)),
		Distance = Unigine::Math::distance(Unigine::Math::vec3(node->getWorldPosition()), Unigine::Math::vec3(MainCharacter->getWorldPosition())),
		speed = getComponent<PhysicsController>(MainCharacter)->getSpeed();


	if (speed <= 1 && speed > 0) {

		_Bullet->worldLookAt(Unigine::Math::lerp(MainCharacter->getWorldPosition(), FuturePoint, speed / FutureDistance));
		_BulletPhysics->addLinearImpulse(_Bullet->getWorldDirection(Unigine::Math::AXIS_Y) * Distance);
	}
	else if (speed > 1)
	{
		_Bullet->worldLookAt(FuturePoint);
		_BulletPhysics->addLinearImpulse(_Bullet->getWorldDirection(Unigine::Math::AXIS_Y) * Distance * (speed / FutureDistance));
	}
	else
	{
		_Bullet->worldLookAt(node->getChild(0)->getWorldPosition() + Unigine::Math::Vec3(node->getWorldDirection(Unigine::Math::AXIS_Y)));
		_BulletPhysics->addLinearImpulse(_Bullet->getWorldDirection(Unigine::Math::AXIS_Y) * Distance);
	}
}