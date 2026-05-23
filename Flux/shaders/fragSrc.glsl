#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in mat3 TBN;

uniform bool      hasTexture;
uniform bool      isSelected;
uniform sampler2D albedoMap;
uniform vec3      matColor;
uniform float     roughness;
uniform float     metallic;
uniform float     alpha;

uniform vec3 viewPos;

uniform sampler2D shadowMap;
uniform bool      hasShadowMap;

uniform bool  hasSunLight;
uniform vec3  sunLightDir;
uniform vec3  sunLightColor;
uniform float sunLightIntensity;

uniform bool  hasMoonLight;
uniform vec3  moonLightDir;
uniform vec3  moonLightColor;
uniform float moonLightIntensity;
uniform float timeOfDay;

#define MAX_POINT 8
uniform int   numPointLights;
uniform vec3  pointPos      [MAX_POINT];
uniform vec3  pointColor    [MAX_POINT];
uniform float pointIntensity[MAX_POINT];
uniform float pointRange    [MAX_POINT];

#define MAX_SPOT 4
uniform int   numSpotLights;
uniform vec3  spotPos      [MAX_SPOT];
uniform vec3  spotDir      [MAX_SPOT];
uniform vec3  spotColor    [MAX_SPOT];
uniform float spotIntensity[MAX_SPOT];
uniform float spotRange    [MAX_SPOT];
uniform float spotInner    [MAX_SPOT];
uniform float spotOuter    [MAX_SPOT];

uniform bool  hasSurface;
uniform vec3  surfaceColor;
uniform float surfaceIntensity;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdH  = max(dot(N, H), 0.0);
    float NdH2 = NdH * NdH;
    float denom = NdH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdV / (NdV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdV = max(dot(N, V), 0.0);
    float NdL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdV, roughness) * GeometrySchlickGGX(NdL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float PointAttenuation(float dist, float range) {
    float falloff = pow(clamp(1.0 - pow(dist / range, 4.0), 0.0, 1.0), 2.0);
    return falloff / (dist * dist + 0.0001);
}

float ShadowFactor(vec4 fragPosLS, vec3 N, vec3 L) {
    if (!hasShadowMap) return 1.0;

    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 1.0;

    float cosTheta = clamp(dot(N, L), 0.0, 1.0);
    float bias = mix(0.001, 0.0002, cosTheta);

    vec2 ts = 1.0 / vec2(textureSize(shadowMap, 0));

    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closest = texture(shadowMap, proj.xy + vec2(x, y) * ts).r;
            shadow += (proj.z - bias > closest) ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}

vec3 PBRContrib(vec3 N, vec3 V, vec3 L, vec3 lightColor,
                float attenuation, vec3 albedo, vec3 F0,
                float roughness, float metallic) {
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor * attenuation;

    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  numerator   = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3  specular    = numerator / denominator;

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    float NdL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdL;
}

void main() {
    vec3 albedo;
    if (hasTexture) {
        albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    } else {
        albedo = pow(max(matColor, vec3(0.001)), vec3(2.2));
    }

    vec3 N  = normalize(Normal);
    vec3 V  = normalize(viewPos - FragPos);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float tod = timeOfDay;
    float dayT = clamp(sin(((tod - 6.0) / 24.0) * PI * 2.0 - PI * 0.5) * 0.5 + 0.5, 0.0, 1.0);
    vec3 ambientColor = mix(vec3(0.02, 0.03, 0.08),
                            vec3(0.30, 0.32, 0.38),
                            dayT);
    float dawnDusk = (1.0 - abs(dayT * 2.0 - 1.0));
    ambientColor += dawnDusk * vec3(0.06, 0.03, 0.0);

    if (hasSurface)
        ambientColor += surfaceColor * surfaceIntensity * 0.3;

    vec3 Lo = vec3(0.0);

    if (hasSunLight) {
        vec3  L      = normalize(-sunLightDir);
        float shadow = ShadowFactor(FragPosLightSpace, N, L);
        Lo += PBRContrib(N, V, L,
                         sunLightColor * sunLightIntensity,
                         1.0, albedo, F0, roughness, metallic) * shadow;
    }

    if (hasMoonLight) {
        vec3 L = normalize(-moonLightDir);
        Lo += PBRContrib(N, V, L,
                         moonLightColor * moonLightIntensity,
                         1.0, albedo, F0, roughness, metallic);
    }

    for (int i = 0; i < numPointLights; i++) {
        vec3  diff = pointPos[i] - FragPos;
        float dist = length(diff);
        if (dist > pointRange[i]) continue;
        vec3  L    = normalize(diff);
        float att  = PointAttenuation(dist, pointRange[i]) * pointRange[i] * pointRange[i];
        Lo += PBRContrib(N, V, L,
                         pointColor[i] * pointIntensity[i],
                         att, albedo, F0, roughness, metallic);
    }

    for (int i = 0; i < numSpotLights; i++) {
        vec3  diff = spotPos[i] - FragPos;
        float dist = length(diff);
        if (dist > spotRange[i]) continue;
        vec3 L = normalize(diff);

        float cosInner = cos(radians(spotInner[i]));
        float cosOuter = cos(radians(spotOuter[i]));
        float theta    = dot(L, normalize(-spotDir[i]));
        float epsilon  = cosInner - cosOuter;
        float spotFactor = clamp((theta - cosOuter) / epsilon, 0.0, 1.0);
        if (spotFactor <= 0.0) continue;

        float att = PointAttenuation(dist, spotRange[i]) * spotRange[i] * spotRange[i];
        Lo += PBRContrib(N, V, L,
                         spotColor[i] * spotIntensity[i],
                         att * spotFactor, albedo, F0, roughness, metallic);
    }

    vec3 Fenv = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD_env = (vec3(1.0) - Fenv) * (1.0 - metallic);
    vec3 ambient = ambientColor * kD_env * albedo;

    float hemi = 0.5 + 0.5 * dot(N, vec3(0.0, 1.0, 0.0));
    ambient += Fenv * mix(ambientColor, ambientColor * 1.5, hemi) * (1.0 - roughness) * 0.15;

    vec3 color = ambient + Lo;

    color = clamp((color * (2.51 * color + 0.03)) /
                  (color * (2.43 * color + 0.59) + 0.14), 0.0, 1.0);
    color = pow(color, vec3(1.0 / 2.2));

    if (isSelected) {
        color += vec3(0.2, 0.2, 0.0); 
    }

    FragColor = vec4(color, alpha);
}
