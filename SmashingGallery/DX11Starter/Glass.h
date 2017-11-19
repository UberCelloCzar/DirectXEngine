#pragma once
#include "GameObject.h"
#include "GlassMat.h"
class Glass :
	public GameObject
{
public:
	Glass(Mesh* meshPtr, GlassMat* matPtr, std::vector<Script*> pScripts) :
		GameObject(meshPtr, matPtr, pScripts) 
	{
		refractionScale = .03f;
		padding.x = 0.0f;
		padding.y = 0.0f;
		padding.z = 0.0f;
	};
	~Glass();
	int Draw(ID3D11DeviceContext* context);
private:
	float refractionScale;
	XMFLOAT3 padding;
};

