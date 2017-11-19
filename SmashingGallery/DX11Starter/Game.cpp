#include "Game.h"
#include "Vertex.h"
#include <iostream>
#include "WICTextureLoader.h"
#include "Bullet.h"
#include <math.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class
	delete mesh1;
	//delete mesh2;
	delete camera;
	delete material;
	delete glassMaterial;
	shaderResourceView1->Release();
	samplerState1->Release();

	for (int i = 0; i < 20; i++) // Clean up
	{
		delete bullets[i];
	}

	for (int i = 0; i < 3; i++)
	{
		delete targets[i];
	}

	delete glassTarget;

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete glassVertexShader;
	delete glassPixelShader;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	camera = new Camera(width, height);
	LoadGeometry();

	CreateWICTextureFromFile(device, context, L"images\\StoneAlbedo.tif", 0, &shaderResourceView1, 0);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &samplerState1);

	material = new Material(vertexShader, pixelShader, shaderResourceView1, samplerState1);
	glassMaterial = new GlassMat(glassVertexShader, glassPixelShader, shaderResourceView1, refractShaderResourceView1, samplerState1);
	targets[0] = new GameObject(mesh1, material, { new target() });
	targets[1] = new GameObject(mesh1, material, { new target() });
	targets[2] = new GameObject(mesh1, material, { new target() });
	prevMousePos.x = NULL;
	light1 = { XMFLOAT4(0.1f, 0.1f, 0.08f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.35f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0.0f)};
	light2 = { XMFLOAT4(0.1f, 0.1f, 0.08f, 1.0f), XMFLOAT4(0.4f, 0.4f, 0.35f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) };

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	head = 0;
	tail = 0; // Initialize queue tracers

	for (int i = 0; i < 20; i++) // Initialize the memory pool and the inactive queue
	{
		bullets[i] = new GameObject(mesh1, material, {new Bullet()});
		bullets[i]->SetScale(.3, .3, .3);
		inactiveBullets[i] = i;
	}
	score = 0;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");
	
	glassVertexShader = new SimpleVertexShader(device, context);
	glassVertexShader->LoadShaderFile(L"GlassVShader.cso");

	glassPixelShader = new SimplePixelShader(device, context);
	glassPixelShader->LoadShaderFile(L"GlassPShader.cso");
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::LoadGeometry()
{
	mesh1 = new Mesh("models\\sphere.obj", device);
            head = 0;
            tail = 0; // Initialize queue tracers
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	camera->UpdateProjectionMatrix(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{

	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	for (int i = 0; i < 20; i++) // Update active bullets
	{
		if (dynamic_cast<Bullet*>(bullets[i]->scripts[0])->isActive)
		{
			bullets[i]->Update(deltaTime);
			for (int j = 0; j < 3; j++) // Checks this bullet against all targets (will turn into octree if extra performance is needed later)
			{
				if (dynamic_cast<target*>(targets[j]->scripts[0])->isActive)
				{
					if (bullets[i]->collider.collidesWith(*bullets[i], *targets[j]))
					{
						dynamic_cast<Bullet*>(bullets[i]->scripts[0])->isActive = false;
						dynamic_cast<target*>(targets[j]->scripts[0])->isActive = false;
						score += 1;
						std::cout << score << std::endl;
					}
				}
			}
			if (bullets[i]->collider.checkBounds(*bullets[i]))
			{
				dynamic_cast<Bullet*>(bullets[i]->scripts[0])->isActive = false;

			}
			if (!(dynamic_cast<Bullet*>(bullets[i]->scripts[0]))->isActive) // If the bullet went out of bounds or was otherwise destroyed, add it back to the inactive queue
			{
				ReloadBullet(i);
			}
			else if (bullets[i]->GetChanged())
			{
				bullets[i]->CalculateWorldMatrix();
			}
		}


	}

	float right = 0;
	float forward = 0;
	if (GetAsyncKeyState('A') & 0x8000) 
	{ 
		right -= 5*deltaTime;
	}
	if (GetAsyncKeyState('D') & 0x8000) 
	{
		right += 5*deltaTime;
	}
	if (GetAsyncKeyState('W') & 0x8000)
	{
		forward += 5*deltaTime;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		forward -= 5*deltaTime;
	}
	if (fireCD <= 0)
	{
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			Fire();
		}
	}
	else
	{
		fireCD -= deltaTime;
	}

	if (forward != 0 || right != 0)
	{
		camera->MoveRelative(forward, right, 0);
	}

	//has to be updated with the amount of targets added
	for (int i = 0; i < 3; i++)
	{
		if (dynamic_cast<target*>(targets[i]->scripts[0])->isActive)
		{
			targets[i]->SetPosition(sin(totalTime * 1.6f + (2 * i)) * 2, targets[i]->GetPosition().y, targets[i]->GetPosition().z);
			if (targets[i]->GetChanged()) { targets[i]->CalculateWorldMatrix(); }
		}
	}


	camera->Update();
}

void Game::ReloadBullet(int i) // Adds the index i of a freshly inactive bullet back to the inactive queue and moves the tail
{
	inactiveBullets[tail] = i;
	tail++;
	if (tail > 20)
	{
		tail = 0;
	}
}

bool Game::NoBullets()
{
	for (int i = 0; i < 20; i++)
	{
		if (inactiveBullets[i] != 42)
		{
			return false; // If at least one bullet is inactive, say so
		}
	}
	return true;
}

void Game::Fire() // Fires a bullet
{
	if (NoBullets())
	{
		return; // If this happens the cooldown has been set poorly.
	}

	currentBullet = inactiveBullets[head]; // Move the bullet to the character's position
	inactiveBullets[head] = 42;
	head++;
	if (head>19)
	{
		head = 0;
	}

	XMFLOAT3 pos = camera->GetPosition();
	XMFLOAT3 dir = camera->GetDirection();
	bullets[currentBullet]->SetPosition(pos.x, pos.y, pos.z); // Set the position and direction of the bullet, then fire it
	bullets[currentBullet]->SetVelocity(dir.x * 100, dir.y * 100, dir.z * 100);
	dynamic_cast<Bullet*>(bullets[currentBullet]->scripts[0])->isActive = true;
	fireCD = 1.0f;
	std::cout << "Firing: " << currentBullet << std::endl;
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	/* Plain Objects (Shader Set 1) */
	vertexShader->SetShader();
	pixelShader->SetShader();
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vertexShader->CopyBufferData("cameraData");

	pixelShader->SetData("light1", &light1, 44);
	pixelShader->SetData("light2", &light2, 44);
	pixelShader->CopyBufferData("lightData");


	//has to be updated with the amount of targets added
	for (int i = 0; i < 3; i++)
	{
		if (dynamic_cast<target*>(targets[i]->scripts[0])->isActive)
		{
			targets[i]->Draw(context);
		}
	}

	targets[0]->SetPosition(0, 1.5, 0);
	targets[1]->SetPosition(0, 0, 0);
	targets[2]->SetPosition(0, -1.5, 0);

	for (int i = 0; i < 20; i++) // Draw active bullets
	{
		if (dynamic_cast<Bullet*>(bullets[i]->scripts[0])->isActive)
		{
			bullets[i]->Draw(context);
		}
	}

	/* Glass Objects (Glass Shader Set) */
	//glassVertexShader->SetShader();
	//glassPixelShader->SetShader();

	//glassVertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	//glassVertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	//glassVertexShader->CopyBufferData("cameraData");

	//glassPixelShader->SetData("light1", &light1, 44);
	//glassPixelShader->SetData("light2", &light2, 44);
	//glassPixelShader->CopyBufferData("lightData");


	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	if (prevMousePos.x != NULL)
	{
		camera->MouseRotate(x - prevMousePos.x, y - prevMousePos.y); // Rotate the camera
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion