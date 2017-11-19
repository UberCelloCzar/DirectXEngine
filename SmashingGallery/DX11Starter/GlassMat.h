#pragma once
#include "Material.h"
class GlassMat :
	public Material
{
public:
	GlassMat(SimpleVertexShader* vShader, SimplePixelShader* pShader, ID3D11ShaderResourceView* SRV, ID3D11ShaderResourceView* normalSRV, ID3D11ShaderResourceView* refractSRV, ID3D11SamplerState* SS) :
		Material(vShader, pShader, SRV, normalSRV, SS) 
	{
		refractShaderResourceView = refractSRV; // Set up the resource for the refraction texture
	};
	~GlassMat();

	ID3D11ShaderResourceView* GetRefractionShaderResourceView();

private:
	ID3D11ShaderResourceView* refractShaderResourceView;
};

