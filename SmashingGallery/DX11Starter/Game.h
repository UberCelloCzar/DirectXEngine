#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "GameObject.h"
#include "Camera.h"
#include "Lights.h"
#include "target.h"
#include <vector>

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);

	int score; // Player score
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void LoadGeometry();

	// Meshes to hold actual geometry data
	Mesh* mesh1;

	Material* material;
	ID3D11ShaderResourceView* shaderResourceView1;
	ID3D11SamplerState* samplerState1;

	// Targets for the scene
	GameObject* targets[3];

	// The scene camera
	Camera* camera;

	DirectionalLight light1;
	DirectionalLight light2;


	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	float fireCD; // The cooldown for firing
	GameObject* bullets[20]; // Memory pool of bullets
	int inactiveBullets[20]; // Queue of inactive bullet indices
	int head; // Pointers to head and tail of queue
	int tail;
	int currentBullet; // Current bullet

	void ReloadBullet(int i);
	bool NoBullets(); // Returns a bool of whether there are any available bullets, ideally this never returns false
	void Fire(); // Fires a bullet
};

