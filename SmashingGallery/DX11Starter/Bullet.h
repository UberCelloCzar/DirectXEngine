#pragma once
#include "Script.h"
class Bullet :
	public Script
{
public:
	Bullet();
	~Bullet();

	GameObject* gameObject;
	bool isActive;

	void Start(GameObject* parent) override; // Called to do initialization
	void Update() override; // Called by the parent GameObject each frame
};

