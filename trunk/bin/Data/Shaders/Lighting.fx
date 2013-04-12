// ================================================================================================
// Computes the specular contribution of a light using normalized Blinn-Phong
// ================================================================================================
float CalculateSpecular(in float3 normal, in float3 halfVector, in float roughness) {
    // Convert roughness to a specular power
    float specPower = pow(2.0f, saturate(1.0f - roughness) * 12.0f);

    float nDotH = saturate(dot(normal, halfVector));
    return pow(nDotH, specPower) * ((specPower + 8.0f) / (8.0f * 3.14159265f));
}

// ================================================================================================
// Calculate the Fresnel factor using Schlick's approximation
// ================================================================================================
float3 CalculateFresnel(in float3 specAlbedo, in float3 halfVector, in float3 lightDir) {
    float3 fresnel = specAlbedo;
    fresnel += (1.0f - specAlbedo) * pow((1.0f - saturate(dot(halfVector, lightDir))), 5.0f);
    return saturate(fresnel);
}

// ================================================================================================
// Calculate an approximate Fresnel factor using N dot V instead of L dot H
// ================================================================================================
float3 CalculateAmbientFresnel(in float3 specAlbedo, in float3 normal,
                                in float3 view, in float roughness) {
    float smooth = 1.0f - roughness;
    float powerterm = pow((1.0f - saturate(dot(normal, view))), 5.0f);
    float3 fresnel = specAlbedo + (max(smooth.xxx, specAlbedo) - specAlbedo) * powerterm;
    return saturate(fresnel);
}

// ================================================================================================
// Calculate the lighting contribution given a light direction and material properties
// ================================================================================================
float3 CalcLighting(in float3 normal, in float3 lightDir, in float3 lightColor,
					in float3 diffuseAlbedo, in float3 specularAlbedo, in float roughness,
					in float3 surfacePos, in float3 cameraPos)
{
	float3 diffuse = saturate(dot(normal, lightDir)) * lightColor;
	float3 view = normalize(cameraPos - surfacePos);
    float3 halfVector = normalize(view + lightDir);
	float3 specular = CalculateSpecular(normal, halfVector, roughness) * diffuse;
	float3 fresnel = CalculateFresnel(specularAlbedo, halfVector, lightDir);

	return diffuse * diffuseAlbedo * (1.0f / 3.14159f) + specular * fresnel;
}

// ================================================================================================
// Calculate the lighting contribution for a single point light
// ================================================================================================
float3 CalcPointLight(in float3 normal, in float3 lightColor, in float3 diffuseAlbedo,
					  in float3 specularAlbedo, in float roughness, in float3 surfacePos,
					  in float3 lightPos, in float lightFalloff, in float3 cameraPos)
{
	float3 pixelToLight = lightPos - surfacePos;
	float lightDist = length(pixelToLight);
	float3 lightDir = pixelToLight / lightDist;
    float attenuation = saturate(1.0f - (lightDist / lightFalloff));
	return CalcLighting(normal, lightDir, lightColor, diffuseAlbedo, specularAlbedo,
						roughness, surfacePos, cameraPos) * attenuation;
}