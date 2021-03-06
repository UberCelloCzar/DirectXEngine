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
#include "GlassMat.h"
#include "Glass.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "Emitter.h"

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
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);

	int score; // Player score
private:

	// Particle
	ID3D11ShaderResourceView* particleTexture;
	SimpleVertexShader* particleVertexShader;
	SimplePixelShader* particlePixelShader;
	ID3D11DepthStencilState* particleDepthState;
	ID3D11BlendState* particleBlendState;
	Emitter* emitter;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders();
	void LoadGeometry();

	// Meshes to hold actual geometry data
	Mesh* mesh1;
	Mesh* mesh2;
	Mesh* mesh3;

	//for UI
	DirectX::SpriteBatch* spriteBatch;
	DirectX::SpriteFont* font;

	Material* material;
	Material* wallMat;
	Material* metalMat;
	Material* goldMat;
	Material* specialWallMat;
	Material* bulletMaterial;
	GlassMat* glassMaterial;
	GlassMat* glassMaterial2;
	ID3D11ShaderResourceView* uiSRV;
	ID3D11ShaderResourceView* shaderResourceViews[6];
	ID3D11ShaderResourceView* renderToTextureSRV;
	ID3D11SamplerState* samplerState1;
	ID3D11ShaderResourceView* normalShaderResourceViews[7];
	ID3D11RenderTargetView* renderTargetView;
	ID3D11Texture2D* renderToTextureTexture;
	ID3D11ShaderResourceView* shootingGalleryTexture;

	// Targets for the scene
	GameObject* targets[3];
	GameObject* walls[5];
	Glass* glassTargets[2];

	// The scene camera
	Camera* camera;

	DirectionalLight light1;
	DirectionalLight light2;

	void RenderShadowMap();
	void RenderToTexture();

	// Shadow stuff ---------------------------
	int shadowMapSize;
	ID3D11DepthStencilView* shadowDSV;
	ID3D11ShaderResourceView* shadowSRV;
	ID3D11SamplerState* shadowSampler;
	ID3D11RasterizerState* shadowRasterizer;
	SimpleVertexShader* shadowVS;
	DirectX::XMFLOAT4X4 shadowViewMatrix;
	DirectX::XMFLOAT4X4 shadowProjectionMatrix;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	SimpleVertexShader* glassVertexShader;
	SimplePixelShader* glassPixelShader;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	float fireCD; // The cooldown for firing
	GameObject* bullets[20]; // Memory pool of bullets
	int inactiveBullets[20]; // Queue of inactive bullet indices
	int head; // Pointers to head and tail of queue
	int tail;
	int currentBullet; // Current bullet

	XMFLOAT3 mousePos;

	void ReloadBullet(int i);
	bool NoBullets(); // Returns a bool of whether there are any available bullets, ideally this never returns false
	void Fire(); // Fires a bullet
};

