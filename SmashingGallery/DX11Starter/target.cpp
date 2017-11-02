#include "target.h"



target::target()
{
	isActive = true;
}


target::~target()
{
}

void target::Start(GameObject* parent)
{
	gameObject = parent;
}

void target::Update()
{

}

