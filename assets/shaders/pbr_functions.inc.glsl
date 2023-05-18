vec3 fresnelSchlick ( vec3 f0, float costheta ) {
	//const float baked_spec_occl = saturate( 50.0 * dot( f0, vec3( 0.3333 ) ) );
	return f0 + ( 1.0 - f0 ) * ApproxPow( saturate( 1.0 - costheta ), 5.0 );
}

float DistributionGGX(float roughness, float NdotH)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float roughness, float NdotV, float NdotL)
{
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 specBRDF_doom ( vec3 N, vec3 V, vec3 L, vec3 f0, float smoothness, out vec3 Fout ) {
	const vec3 H = normalize( V + L );
	float m = ( 1 - smoothness * 0.8 );
	m *= m;
	m *= m;
	float m2 = m * m;
	float NdotH = saturate( dot( N, H ) );
	float spec = (NdotH * NdotH) * (m2 - 1) + 1;
	spec = m2 / ( spec * spec + 1e-8 );
	float Gv = saturate( dot( N, V ) ) * (1.0 - m) + m;
	float Gl = saturate( dot( N, L ) ) * (1.0 - m) + m;
	spec /= ( 4.0 * Gv * Gl + 1e-8 );
    Fout = fresnelSchlick( f0, dot( L, H ) );
	return Fout * spec;
}

vec3 specBRDF(vec3 N, vec3 V, vec3 L, vec3 F0, float roughness, out vec3 Fout)
{
    vec3 H = normalize(V + L);
    float NdotL = max( dot( N, L ), 0.0 );
    float NdotV = max( dot( N, V ), 0.0 );
    float NdotH = max( dot( N, H ), 0.0 );
    float HdotV = max( dot( H, V ), 0.0 );

    float   NDF = DistributionGGX(roughness, NdotH);
    float   G = GeometrySmith(roughness, NdotV, NdotL);
    vec3    F = fresnelSchlick(F0, HdotV);
    vec3    numerator = NDF * G * F;
    float   denominator = 4.0 * NdotV * NdotL  + 1e-8;
    vec3    specular = numerator / denominator;

    Fout = F;
    return specular;    
}
