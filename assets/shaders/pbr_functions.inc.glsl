/*
PbrUtilities.ConvertToMetallicRoughness = function (specularGlossiness) {
            function solveMetallic(diffuse, specular, oneMinusSpecularStrength) {
                if (specular < dielectricSpecular.r) {
                    return 0;
                }

                var a = dielectricSpecular.r;
                var b = diffuse * oneMinusSpecularStrength / (1 - dielectricSpecular.r) + specular - 2 * dielectricSpecular.r;
                var c = dielectricSpecular.r - specular;
                var D = b * b - 4 * a * c;
                return BABYLON.Scalar.Clamp((-b + Math.sqrt(D)) / (2 * a), 0, 1);
            }

            var diffuse = specularGlossiness.diffuse;
            var opacity = specularGlossiness.opacity;
            var specular = specularGlossiness.specular;
            var glossiness = specularGlossiness.glossiness;

            var oneMinusSpecularStrength = 1 - specular.getMaxComponent();
            var metallic = solveMetallic(diffuse.getPerceivedBrightness(), specular.getPerceivedBrightness(), oneMinusSpecularStrength);

            var baseColorFromDiffuse = diffuse.scale(oneMinusSpecularStrength / (1 - dielectricSpecular.r) / Math.max(1 - metallic, epsilon));
            var baseColorFromSpecular = specular.subtract(dielectricSpecular.scale(1 - metallic)).scale(1 / Math.max(metallic, epsilon));
            var baseColor = BABYLON.Color3.Lerp(baseColorFromDiffuse, baseColorFromSpecular, metallic * metallic).clamp();

            return {
                baseColor: baseColor,
                opacity: opacity,
                metallic: metallic,
                roughness: 1 - glossiness
            };
        }
*/


float GammaIEC(float c)
{
    return c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1/2.4) -0.055;
}

vec3 GammaIEC(vec3 c)
{
    return vec3(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b));
}

vec3 Gamma(vec3 c)
{
    return pow(c, vec3(1.0/2.2));
}

float SRGBlinear ( float value ) { 

if ( value <= 0.04045 ) {
		return ( value / 12.92 );
	} else {
		return pow( ( value / 1.055 ) + 0.0521327, 2.4 );
	}
}
vec3 SRGBlinear ( vec3 sRGB ) {
	vec3 outLinear;
	outLinear.r = SRGBlinear( sRGB.r );
	outLinear.g = SRGBlinear( sRGB.g );
	outLinear.b = SRGBlinear( sRGB.b );
	return outLinear;
}
vec4 SRGBlinear ( vec4 sRGBA ) {
	vec4 outLinear = vec4( SRGBlinear( sRGBA.rgb ), 1 );
	outLinear.a = SRGBlinear( sRGBA.a );
	return outLinear;
}
float DeGamma ( float value ) {
	return SRGBlinear( value );
}
vec4 DeGamma ( vec4 color ) {
	return SRGBlinear( color );
}
vec3 DeGamma ( vec3 color ) {
	return SRGBlinear( color );
}


float GetLuma ( vec3 c ) {
	return dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
}


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
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
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


float NDF = DistributionGGX(N, H, roughness);       
float G   = GeometrySmith(N, V, L, roughness);    
