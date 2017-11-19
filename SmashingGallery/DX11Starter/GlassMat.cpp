#include "GlassMat.h"

GlassMat::~GlassMat()
{
}

ID3D11ShaderResourceView* GlassMat::GetRefractionShaderResourceView()
{
	return refractShaderResourceView;
}
