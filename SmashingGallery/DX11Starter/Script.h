#pragma once
class GameObject; // Eliminates circular dependency

class Script // Generic script class that all scripts inherit from
{
public:
	Script();
	~Script();

	virtual void Start(GameObject* parent); // Called to do initialization
	virtual void Update(); // Called by the parent GameObject each frame
};

