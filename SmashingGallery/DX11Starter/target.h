#pragma once
#include "Script.h"
class target :
	public Script
{
public:
	target();
	~target();

	GameObject* gameObject;
	bool isActive;

	void Start(GameObject* parent) override; // Called to do initialization
	void Update() override; // Called by the parent GameObject each frame
};

