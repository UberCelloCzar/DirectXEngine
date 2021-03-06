#include "Game.h"
#include "Vertex.h"
#include <iostream>
#include "WICTextureLoader.h"
#include "Bullet.h"
#include <math.h>
#include <string>

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
	shadowVS = 0;
	glassVertexShader = 0;
	glassPixelShader = 0;
	renderToTextureTexture = 0;

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
	delete mesh2;
	delete mesh3;
	delete camera;
	delete material;
	delete wallMat;
	delete bulletMaterial;
	delete specialWallMat;
	delete goldMat;
	delete metalMat;
	delete glassMaterial;
	delete glassMaterial2;
	delete spriteBatch;
	delete font;
	uiSRV->Release();
	particleTexture->Release();
	particleBlendState->Release();
	particleDepthState->Release();

	for (int i = 0; i < 6; i++)
	{
		shaderResourceViews[i]->Release();
	}

	for (int i = 0; i < 7; i++)
	{
		normalShaderResourceViews[i]->Release();
	}
	renderToTextureSRV->Release();
	samplerState1->Release();
	renderToTextureTexture->Release();
	renderTargetView->Release();

	for (int i = 0; i < 20; i++) // Clean up
	{
		delete bullets[i];
	}

	for (int i = 0; i < 3; i++)
	{
		delete targets[i];
	}

	for (int i = 0; i < 2; i++)
	{
		delete glassTargets[i];
	}

	for (int i = 0; i < 5; i++)
	{
		delete walls[i];
	}

	//delete glassTarget;

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete glassVertexShader;
	delete glassPixelShader;
	delete particleVertexShader;
	delete particlePixelShader;
	delete emitter;
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

	mousePos = XMFLOAT3(0, 0, -5);

	CreateWICTextureFromFile(device, context, L"images\\ui.jpg", 0, &uiSRV);
	CreateWICTextureFromFile(device, context, L"images\\rock.jpg", 0, &shaderResourceViews[0]);
	CreateWICTextureFromFile(device, context, L"images\\rockNormals.jpg", 0, &normalShaderResourceViews[0]);
	CreateWICTextureFromFile(device, context, L"images\\wall.jpg", 0, &shaderResourceViews[1]);
	CreateWICTextureFromFile(device, context, L"images\\wallNormal.jpg", 0, &normalShaderResourceViews[1]);
	CreateWICTextureFromFile(device, context, L"images\\GlassClear.png", 0, &shaderResourceViews[2]);
	CreateWICTextureFromFile(device, context, L"images\\GlassNormalRough.png", 0, &normalShaderResourceViews[2]);
	CreateWICTextureFromFile(device, context, L"images\\GlassNormal.png", 0, &normalShaderResourceViews[3]);
	CreateWICTextureFromFile(device, context, L"images\\particle.jpg", 0, &particleTexture);
	CreateWICTextureFromFile(device, context, L"images\\shootingGallery.jpg", 0, &shootingGalleryTexture);
	CreateWICTextureFromFile(device, context, L"images\\SnowTexture.tif", 0, &shaderResourceViews[3]);
	CreateWICTextureFromFile(device, context, L"images\\SnowNormal.tif", 0, &normalShaderResourceViews[4]);
	CreateWICTextureFromFile(device, context, L"images\\gold.jpg", 0, &shaderResourceViews[4]);
	CreateWICTextureFromFile(device, context, L"images\\goldNormal.jpg", 0, &normalShaderResourceViews[5]);
	CreateWICTextureFromFile(device, context, L"images\\metal.jpg", 0, &shaderResourceViews[5]);
	CreateWICTextureFromFile(device, context, L"images\\metalNormal.jpg", 0, &normalShaderResourceViews[6]);

	//load and set UI stuff
	spriteBatch = new SpriteBatch(context);
	font = new SpriteFont(device, L"Fonts/Arial.spritefont");

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &samplerState1);

	//particles depth state
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepthState);

	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	emitter = new Emitter(
		10,// Max particles
		1,// Particles per second
		2,// Particle lifetime
		3,// Start size
		0.1f,// End size
		XMFLOAT4(1, .5, .1, 1.0f),// Start color
		XMFLOAT4(.7, .2, .05, 0),// End color
		XMFLOAT3(0.5f, 0, 0),// Start velocity
		XMFLOAT3(4, 3.5, 5),// Start position
		XMFLOAT3(0, 0.5f, 0),// Start acceleration
		device, particleVertexShader, particlePixelShader, particleTexture);




	shadowMapSize = 1024;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	device->CreateTexture2D(&textureDesc, 0, &renderToTextureTexture);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(renderToTextureTexture, &renderTargetViewDesc, &renderTargetView);
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(renderToTextureTexture, &shaderResourceViewDesc, &renderToTextureSRV);
	

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapSize;
	shadowDesc.Height = shadowMapSize;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, &shadowTexture);

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture, &shadowDSDesc, &shadowDSV);

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture, &srvDesc, &shadowSRV);

	// Release the texture reference since we don't need it
	shadowTexture->Release();

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Create my shadow map matrices
	XMMATRIX shadowView = XMMatrixLookAtLH(
		XMVectorSet(0, 4, -10, 0), // Eye position
		XMVectorSet(0, 0, 0, 0),	// Looking at (0,0,0)
		XMVectorSet(0, 1, 0, 0));	// Up (0,1,0)

	XMStoreFloat4x4(&shadowViewMatrix, XMMatrixTranspose(shadowView));

	XMMATRIX shadowProj = XMMatrixOrthographicLH(
		10.0f,		// Width of the projection in world units
		10.0f,		// Height of the projection in world units
		0.1f,		// Near clip
		100.0f);	// Far clip

	XMStoreFloat4x4(&shadowProjectionMatrix, XMMatrixTranspose(shadowProj));


	material = new Material(vertexShader, pixelShader, shaderResourceViews[0], normalShaderResourceViews[0], samplerState1);
	wallMat = new Material(vertexShader, pixelShader, shaderResourceViews[1], normalShaderResourceViews[1], samplerState1);
	specialWallMat = new Material(vertexShader, pixelShader, shootingGalleryTexture, normalShaderResourceViews[1], samplerState1);
	glassMaterial = new GlassMat(glassVertexShader, glassPixelShader, shaderResourceViews[2], normalShaderResourceViews[2], renderToTextureSRV, samplerState1);
	glassMaterial2 = new GlassMat(glassVertexShader, glassPixelShader, shaderResourceViews[2], normalShaderResourceViews[3], renderToTextureSRV, samplerState1);
	bulletMaterial = new Material(vertexShader, pixelShader, shaderResourceViews[3], normalShaderResourceViews[4], samplerState1);
	goldMat = new Material(vertexShader, pixelShader, shaderResourceViews[4], normalShaderResourceViews[5], samplerState1);
	metalMat = new Material(vertexShader, pixelShader, shaderResourceViews[5], normalShaderResourceViews[6], samplerState1);



	targets[0] = new GameObject(mesh1, material, { new target() });
	targets[1] = new GameObject(mesh1, goldMat, { new target() });
	targets[2] = new GameObject(mesh1, metalMat, { new target() });
	walls[0] = new GameObject(mesh2, specialWallMat, { new Script() });
	walls[1] = new GameObject(mesh2, wallMat, { new Script() });
	walls[2] = new GameObject(mesh2, wallMat, { new Script() });
	walls[3] = new GameObject(mesh2, wallMat, { new Script() });
	walls[4] = new GameObject(mesh2, wallMat, { new Script() });
	glassTargets[0] = new Glass(mesh1, glassMaterial, { new target() });
	glassTargets[1] = new Glass(mesh3, glassMaterial2, { new target() });


	targets[0]->SetPosition(0, 1.5f, 2);
	targets[1]->SetPosition(0, 0, 2);
	targets[2]->SetPosition(0, -1.5f, 2);
	glassTargets[0]->SetPosition(0, 1.0f, 1.0f);
	glassTargets[1]->SetPosition(0, -.8f, 3.0f);

	//back wall
	walls[0]->SetPosition(0, 0, 5);
	walls[0]->SetRotation(0, 3.14f, 0);
	walls[0]->SetScale(5.1, 5.1, 5.1);

	//left wall
	walls[1]->SetPosition(-5, 0, 0);
	walls[1]->SetRotation(0, 1.57f, 0);
	walls[1]->SetScale(5.1, 5.1, 5.1);

	//right wall
	walls[2]->SetPosition(5, 0, 0);
	walls[2]->SetRotation(0, -1.57f, 0);
	walls[2]->SetScale(5.1, 5.1, 5.1);


	//ceiling
	walls[3]->SetPosition(0, 5, 0);
	walls[3]->SetRotation(1.57, 0, 0);
	walls[3]->SetScale(5.1, 5.1, 5.1);


	//floor
	walls[4]->SetPosition(0, -5, 0);
	walls[4]->SetRotation(-1.57f, 0, 3.14);
	walls[4]->SetScale(5, 5, 5);


	for (int i = 0; i < 5; i++)
	{
		walls[i]->CalculateWorldMatrix();
	}

	prevMousePos.x = NULL;
	light1 = { XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.6f, 0.6f, 0.57f, 1.0f), XMFLOAT3(0.0f, -0.5f, 1.0f) };
	light2 = { XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.1f, 0.1f, 0.05f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) };

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	head = 0;
	tail = 0; // Initialize queue tracers

	for (int i = 0; i < 20; i++) // Initialize the memory pool and the inactive queue
	{
		bullets[i] = new GameObject(mesh1, bulletMaterial, { new Bullet() });
		bullets[i]->SetScale(0.3f, 0.3f, 0.3f);
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

	shadowVS = new SimpleVertexShader(device, context);
	shadowVS->LoadShaderFile(L"ShadowVS.cso");

	particleVertexShader = new SimpleVertexShader(device, context);
	particleVertexShader->LoadShaderFile(L"ParticleVertexShader.cso");

	particlePixelShader = new SimplePixelShader(device, context);
	particlePixelShader->LoadShaderFile(L"ParticlePixelShader.cso");
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::LoadGeometry()
{
	mesh1 = new Mesh("models\\sphere.obj", device);
	mesh2 = new Mesh("models\\quad.obj", device);
	mesh3 = new Mesh("models\\cube.obj", device);
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
						//std::cout << score << std::endl;
					}
				}
			}
			for (int j = 0; j < 2; j++) // Checks this bullet against all targets (will turn into octree if extra performance is needed later)
			{
				if (dynamic_cast<target*>(glassTargets[j]->scripts[0])->isActive)
				{
					if (bullets[i]->collider.collidesWith(*bullets[i], *glassTargets[j]))
					{
						dynamic_cast<Bullet*>(bullets[i]->scripts[0])->isActive = false;
						dynamic_cast<target*>(glassTargets[j]->scripts[0])->isActive = false;
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

		emitter->Update(deltaTime);

	}

	float right = 0;
	float forward = 0;
	
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

	for (int i = 0; i < 2; i++)
	{
		if (dynamic_cast<target*>(glassTargets[i]->scripts[0])->isActive)
		{
			glassTargets[i]->SetPosition(sin(totalTime * 1.6f + (2 * (i+4))) * 2, glassTargets[i]->GetPosition().y, glassTargets[i]->GetPosition().z);
			if (glassTargets[i]->GetChanged()) { glassTargets[i]->CalculateWorldMatrix(); }
		}
	}


	camera->Update();
}

void Game::RenderToTexture()
{
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	context->ClearRenderTargetView(renderTargetView, color);

	// Clear the depth buffer.
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Render all of the entities in the scene
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	vertexShader->SetShader();
	pixelShader->SetShader();
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	// We need to pass the shadow "creation" matrices in here
	// so we can reconstruct the shadow map position
	vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
	vertexShader->SetMatrix4x4("shadowProj", shadowProjectionMatrix);
	vertexShader->CopyBufferData("cameraData");

	pixelShader->SetData("light1", &light1, 44);
	pixelShader->SetData("light2", &light2, 44);
	pixelShader->CopyBufferData("lightData");
	pixelShader->SetSamplerState("ShadowSampler", shadowSampler);
	pixelShader->SetShaderResourceView("ShadowMap", shadowSRV);
	//wallMat->shaderResourceView = shadowSRV;


	//has to be updated with the amount of targets added
	for (int i = 0; i < 3; i++)
	{
		if (dynamic_cast<target*>(targets[i]->scripts[0])->isActive)
		{
			targets[i]->Draw(context);
		}
	}

	for (int i = 0; i < 5; i++)
	{
		walls[i]->Draw(context);
	}

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

}

void Game::RenderShadowMap()
{
	// Change which depth buffer I'm rendering into
	context->OMSetRenderTargets(0, 0, shadowDSV);
	context->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set up any required rendering states
	context->RSSetState(shadowRasterizer);

	// Create a viewport that matches our render target size
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Turn on our shadow mapping shaders
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);

	// Turn OFF the pixel shader
	context->PSSetShader(0, 0, 0);

	// Render all of the entities in the scene
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (int i = 0; i < 3; i++)
	{
		if (dynamic_cast<target*>(targets[i]->scripts[0])->isActive)
		{
			// Grab the data from the first entity's mesh
			GameObject* ge = targets[i];
			ID3D11Buffer* vb = ge->GetMesh()->GetVertexBuffer();
			ID3D11Buffer* ib = ge->GetMesh()->GetIndexBuffer();

			// Set buffers in the input assembler
			context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
			context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

			// Use the SHADOW VERT SHADER
			shadowVS->SetMatrix4x4("world", ge->GetWorldMatrix());
			shadowVS->CopyAllBufferData();

			// Finally do the actual drawing
			context->DrawIndexed(ge->GetMesh()->GetIndexCount(), 0, 0);
		}
	}

	// Now that shadow rendering is done, put back all
	// of the states and other render options we've changed
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetState(0);

	viewport.Width = this->width;
	viewport.Height = this->height;
	context->RSSetViewports(1, &viewport);
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

	XMFLOAT3 pos = mousePos;
	XMFLOAT3 dir = camera->GetDirection();
	bullets[currentBullet]->SetPosition(pos.x, pos.y, pos.z); // Set the position and direction of the bullet, then fire it
	bullets[currentBullet]->SetVelocity(dir.x * 20, dir.y * 20, dir.z * 20);
	dynamic_cast<Bullet*>(bullets[currentBullet]->scripts[0])->isActive = true;
	fireCD = 1.0f;
	std::cout << "Firing: " << currentBullet << std::endl;
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{

	// Before we do anything the user can see, render
	// the shadow map from the light's point of view
	RenderShadowMap();

	RenderToTexture();
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
	// We need to pass the shadow "creation" matrices in here
	// so we can reconstruct the shadow map position
	vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
	vertexShader->SetMatrix4x4("shadowProj", shadowProjectionMatrix);
	vertexShader->CopyBufferData("cameraData");

	pixelShader->SetData("light1", &light1, 44);
	pixelShader->SetData("light2", &light2, 44);
	pixelShader->CopyBufferData("lightData");
	pixelShader->SetSamplerState("ShadowSampler", shadowSampler);
	pixelShader->SetShaderResourceView("ShadowMap", shadowSRV);
	//wallMat->shaderResourceView = renderToTextureSRV;


	//has to be updated with the amount of targets added
	for (int i = 0; i < 3; i++)
	{
		if (dynamic_cast<target*>(targets[i]->scripts[0])->isActive)
		{
			targets[i]->Draw(context);
		}
	}

	for (int i = 0; i < 5; i++)
	{
		walls[i]->Draw(context);
	}

	for (int i = 0; i < 20; i++) // Draw active bullets
	{
		if (dynamic_cast<Bullet*>(bullets[i]->scripts[0])->isActive)
		{
			bullets[i]->Draw(context);
		}
	}
	//wallMat->shaderResourceView = shaderResourceView2;

	//particles draw
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);
	context->OMSetDepthStencilState(particleDepthState, 0);
	emitter->Draw(context, camera);
	context->OMSetBlendState(0, blend, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
	

	/* Glass Objects (Glass Shader Set) */
	glassVertexShader->SetShader();
	glassPixelShader->SetShader();

	glassVertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	glassVertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	glassVertexShader->CopyBufferData("cameraData");

	glassPixelShader->SetData("light1", &light1, 44);
	glassPixelShader->SetData("light2", &light2, 44);
	glassPixelShader->CopyBufferData("lightData");

	for (int i = 0; i < 2; i++)
	{
		if (dynamic_cast<target*>(glassTargets[i]->scripts[0])->isActive)
		{
			glassTargets[i]->Draw(context);
		}
	}

	// Check out the texture that is stored in the font
	ID3D11ShaderResourceView* fontTexture;
	font->GetSpriteSheet(&fontTexture);

	//rectangles for backgrounds
	RECT scoreRect = { 0, this->height - 100, 200, this->height };
	//RECT timeRect = { this->width - 200, this->height - 100, this->width, this->height };

	// Begin, draw, and then end the sprite batch
	spriteBatch->Begin();
	spriteBatch->Draw(uiSRV, scoreRect);
	//spriteBatch->Draw(uiSRV, timeRect);
	font->DrawString(
		spriteBatch,
		L"Score:",
		XMFLOAT2(0, this->height - 100));
	font->DrawString(
		spriteBatch,
		(std::to_wstring(score)).c_str(),
		XMFLOAT2(80, this->height - 50));

	spriteBatch->End();

	// Release the new reference we just made
	// to the font texture (a few lines above)
	fontTexture->Release();

	// Reset changes from spritebatch for next frame
	float blendFactors[4] = { 1,1,1,1 };
	context->OMSetBlendState(0, blendFactors, 0xFFFFFFFF);
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

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
	
	//save mouse position for firing
	POINT p;

	GetCursorPos(&p);

	ScreenToClient(this->hWnd, &p);

	float pX = p.x;
	float pY = p.y;
	pX = (pX / this->width) * 7 - 3.5;
	pY = -((pY / this->height) * 5 - 2.5);

	mousePos.x = pX;
	mousePos.y = pY;

	/*if (prevMousePos.x != NULL)
	{
		camera->MouseRotate(x - prevMousePos.x, y - prevMousePos.y); // Rotate the camera
	}*/

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