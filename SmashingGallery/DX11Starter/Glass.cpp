#include "Glass.h"
#include "GlassMat.h"


Glass::~Glass()
{
}

int Glass::Draw(ID3D11DeviceContext* context)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	material->GetVertexShader()->SetMatrix4x4("world", worldMatrix);
	material->GetVertexShader()->CopyBufferData("perObjectData");
	material->GetPixelShader()->SetSamplerState("basicSampler", material->GetSamplerState());
	material->GetPixelShader()->SetShaderResourceView("diffuseTexture", material->GetShaderResourceView());
	material->GetPixelShader()->SetShaderResourceView("refractionTexture", ((GlassMat*)material)->GetRefractionShaderResourceView());
	material->GetPixelShader()->SetData("refractionScale", &refractionScale, 4);
	material->GetPixelShader()->SetData("padding", &padding, 12);
	material->GetPixelShader()->CopyBufferData("glassBuffer");
	ID3D11Buffer* vertBuff = mesh->GetVertexBuffer(); // Set the second object
	context->IASetVertexBuffers(0, 1, &vertBuff, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(
		mesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
	return 1;
}