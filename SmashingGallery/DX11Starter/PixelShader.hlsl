
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
	float2 uv                 : TEXCOORD;
	float3 normal             : NORMAL;
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

Texture2D diffuseTexture  : register(t0); 
SamplerState basicSampler : register(s0);

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
	// Normalize the incoming normal
	input.normal = normalize(input.normal);
	// Diffuse light calculation
	float NdotL = saturate(dot(input.normal, -light1.Direction));

	float4 pixelColor = light1.AmbientColor + (light1.DiffuseColor * NdotL);

	NdotL = saturate(dot(input.normal, -light2.Direction));
	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);

	return (pixelColor + (light2.AmbientColor + (light2.DiffuseColor * NdotL))) * surfaceColor;

}