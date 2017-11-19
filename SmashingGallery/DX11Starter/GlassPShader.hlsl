
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		      : SV_POSITION;	// XYZW position (System Value Position)
	float4 refractionPosition : TEXCOORD;
	float2 uv                 : TEXCOORD1;
	float3 normal             : TEXCOORD2;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};


cbuffer lightData : register(b2)
{
	DirectionalLight light1;
	DirectionalLight light2;
}

cbuffer glassBuffer // The GlassBuffer is used for setting the refractionScale. The refractionScale variable is used for scaling the amount of perturbation to the refraction texture.
{
	float refractionScale;
	float3 padding;
}

Texture2D diffuseTexture  : register(t0);
SamplerState basicSampler : register(s0);
Texture2D refractionTexture : register(t1);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float2 refractTexCoord;
float3 normal;
	
	// Normalize the incoming normal
	input.normal = normalize(input.normal);
	// Diffuse light calculation
	float NdotL = saturate(dot(input.normal, -light1.Direction));

	float4 pixelColor = light1.AmbientColor + (light1.DiffuseColor * NdotL);

	NdotL = saturate(dot(input.normal, -light2.Direction));
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);

	refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f; // Calculate the projected refraction texture coordinates by converting from (-1, +1) to (0, 1) for a texture
	refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;
	
	normal = (input.normal * 2.0f) - 1.0f; // Expand the range of the normal from (0,1) to (-1,+1).
	refractTexCoord = refractTexCoord + (normal.xy * refractionScale); // Re-position the texture coordinate sampling position by the normal map value to simulate light distortion through glass.
																	   
	float4 refractionColor = refractionTexture.Sample(basicSampler, refractTexCoord); // Sample the texture pixel from the refraction texture using the perturbed texture coordinates.

	float4 color = (pixelColor + (light2.AmbientColor + (light2.DiffuseColor * NdotL))) * surfaceColor;
	color = lerp(refractionColor, color, 0.5f);
	return color;

}