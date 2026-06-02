#version 330 core
out vec4 FragColor;
in  vec3 TexCoords;

uniform vec3  sunDir;
uniform float timeOfDay;
uniform bool  hasLightingNode;
const vec3  RAYLEIGH_COLOR = vec3(0.18867780436772762, 0.4978442963618773, 0.6616065586417131);
const float RAYLEIGH_STR   = 1.0;
const float MIE_COEFF      = 0.076;
const float MIE_G          = 0.76;
const float SUN_POWER      = 20.0;

float miePhase(float cosAngle, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * 3.14159 * pow(1.0 + g2 - 2.0 * g * cosAngle, 1.5));
}

vec3 drawMoon(vec3 viewDir, vec3 moonDir) {
    float cosA    = dot(viewDir, moonDir);
    float moonDisk  = smoothstep(0.9985, 0.9992, cosA);
    float moonHalo  = pow(max(cosA, 0.0), 80.0) * 0.12;
    vec3 moonColor  = vec3(0.96, 0.97, 1.00) * moonDisk
                    + vec3(0.60, 0.65, 0.80) * moonHalo;
    return moonColor;
}

float hash(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}
float stars(vec3 dir, float night) {
    vec3 q = floor(dir * 180.0);
    float h = hash(q);
    float twinkle = 0.85 + 0.15 * sin(h * 100.0);
    return step(0.997, h) * twinkle * night * night;
}

void main() {
    vec3 viewDir = normalize(TexCoords);

    if (!hasLightingNode) {
        vec3 moonDir  = normalize(vec3(0.3, 0.6, -0.5));
        vec3 nightSky = mix(vec3(0.01, 0.01, 0.04), vec3(0.04, 0.05, 0.12),
                            clamp(viewDir.y * 0.5 + 0.5, 0.0, 1.0));
        nightSky     += drawMoon(viewDir, moonDir);
        nightSky     += stars(viewDir, 1.0);
        nightSky      = nightSky / (nightSky + vec3(1.0));
        nightSky      = pow(max(nightSky, 0.0), vec3(1.0 / 2.2));
        FragColor     = vec4(nightSky, 1.0);
        return;
    }

    float tod = timeOfDay;
    float sunElev = -dot(normalize(sunDir), vec3(0.0, 1.0, 0.0));
    float daytime  = clamp(sunElev * 2.0 + 0.5, 0.0, 1.0);
    float night    = clamp(1.0 - sunElev * 3.0, 0.0, 1.0);
    float dawnDusk = smoothstep(0.0, 0.4, daytime) * smoothstep(1.0, 0.4, daytime);

    float vUp = clamp(viewDir.y, 0.0, 1.0);
    float vDn = clamp(-viewDir.y, 0.0, 1.0);

    vec3 zenith  = mix(vec3(0.03, 0.04, 0.10), vec3(0.10, 0.28, 0.75), daytime);
    vec3 horizon = mix(vec3(0.10, 0.12, 0.20), vec3(0.52, 0.75, 0.95), daytime);
    vec3 dawnTint = vec3(1.00, 0.45, 0.12);
    horizon = mix(horizon, dawnTint, dawnDusk * 0.7);
    zenith  = mix(zenith,  vec3(0.28, 0.18, 0.38), dawnDusk * 0.4);

    float haze   = pow(1.0 - vUp, 4.0);
    vec3  sky    = mix(zenith, horizon, haze);

    vec3 ground  = mix(horizon * 0.4, vec3(0.08, 0.07, 0.06), vDn);
    sky           = mix(sky, ground, vDn * 0.8);

    vec3 lightDir  = normalize(-sunDir);
    float cosAngle = dot(viewDir, lightDir);

    float mie     = miePhase(cosAngle, MIE_G) * MIE_COEFF * daytime;
    float sunDisk  = pow(max(cosAngle, 0.0), 2000.0) * SUN_POWER * daytime;
    float sunGlow  = pow(max(cosAngle, 0.0), 64.0) * 0.6 * daytime;

    vec3 sunColor  = mix(vec3(1.00, 0.75, 0.30), vec3(1.00, 0.98, 0.90), daytime);
    vec3 sunFinal  = sunColor * (sunDisk + sunGlow) + RAYLEIGH_COLOR * mie;

    sky += sunFinal;

    vec3 moonDir = -lightDir;
    moonDir.y = abs(moonDir.y);
    sky += drawMoon(viewDir, moonDir) * night;

    sky += stars(viewDir, night);

    sky = sky / (sky + vec3(1.0));
    sky = pow(max(sky, 0.0), vec3(1.0 / 2.2));

    FragColor = vec4(sky, 1.0);
}
