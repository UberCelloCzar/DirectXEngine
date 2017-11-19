#include "Material.h"



Material::Material(SimpleVertexShader* vShader, SimplePixelShader* pShader, ID3D11ShaderResourceView* SRV, ID3D11ShaderResourceView* normalSRV, ID3D11SamplerState* SS)
{
	vertexShader = vShader;
	pixelShader = pShader;
	shaderResourceView = SRV;
	samplerState = SS;
	normalShaderResourceView = normalSRV;
}

SimpleVertexShader* Material::GetVertexShader()
{
	return vertexShader;
}

SimplePixelShader* Material::GetPixelShader()
{
	return pixelShader;
}

ID3D11ShaderResourceView* Material::GetShaderResourceView()
{
	return shaderResourceView;
}

ID3D11ShaderResourceView* Material::GetNormalShaderResourceView()
{
	return normalShaderResourceView;
}

ID3D11SamplerState* Material::GetSamplerState()
{
	return samplerState;
}


Material::~Material()
{
	vertexShader = NULL;
	pixelShader = NULL;
	samplerState = NULL;
	shaderResourceView = NULL;
}
