#pragma once

// Shader của viewer. Với đèn mặc định (màu trắng, hướng -normalize(1,2,3)) và uModel = đơn vị,
// kết quả giống hệt shader cũ: base * (0.35 + 0.65 * |dot(N, normalize(1,2,3))|).
namespace Domain::Scene::ViewerShader {
	constexpr const char* VERTEX_SOURCE = R"(#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;
layout(location=3) in vec3 aColor;
uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;
out vec3 worldPosition;
out vec3 normal;
out vec2 texCoords;
out vec3 vertexColor;
void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    worldPosition = world.xyz;
    gl_Position = uViewProjection * world;
    normal = uNormalMatrix * aNormal;
    texCoords = aTexCoords;
    vertexColor = aColor;
})";

	constexpr const char* FRAGMENT_SOURCE = R"(#version 330 core
#define MAX_LIGHTS 8
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define LIGHT_SPOT 2
in vec3 worldPosition;
in vec3 normal;
in vec2 texCoords;
in vec3 vertexColor;
struct Material { sampler2D diffuse[1]; };
struct Light {
    int type;
    vec3 position;
    vec3 direction;   // hướng ánh sáng đi tới
    vec3 color;       // đã nhân cường độ
    float cosInner;
    float cosOuter;
};
uniform Material material;
uniform bool uHasDiffuseTexture;
uniform vec4 uBaseColor;
uniform bool uEnableTextures;
uniform bool uEnableLighting;
uniform int uLightCount;
uniform Light uLights[MAX_LIGHTS];
out vec4 color;
const float AMBIENT = 0.35;
const float DIFFUSE = 0.65;
void main() {
    vec4 base = uBaseColor * vec4(vertexColor, 1.0);
    if (uEnableTextures && uHasDiffuseTexture) base *= texture(material.diffuse[0], texCoords);
    if (!uEnableLighting) {
        color = base;
        return;
    }
    vec3 n = normalize(normal);
    vec3 lit = vec3(AMBIENT);
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i) {
        vec3 toLight;
        float attenuation = 1.0;
        if (uLights[i].type == LIGHT_DIRECTIONAL) {
            toLight = -uLights[i].direction;
        } else {
            vec3 offset = uLights[i].position - worldPosition;
            float distanceSquared = max(dot(offset, offset), 1e-4);
            toLight = offset * inversesqrt(distanceSquared);
            attenuation = 1.0 / distanceSquared; // suy giảm theo bình phương khoảng cách như glTF
            if (uLights[i].type == LIGHT_SPOT) {
                float cosAngle = dot(-toLight, uLights[i].direction);
                attenuation *= uLights[i].cosInner > uLights[i].cosOuter
                    ? smoothstep(uLights[i].cosOuter, uLights[i].cosInner, cosAngle)
                    : step(uLights[i].cosOuter, cosAngle);
            }
        }
        // abs(): mặt có pháp tuyến ngược vẫn được chiếu sáng, giống shader cũ.
        lit += DIFFUSE * uLights[i].color * abs(dot(n, toLight)) * attenuation;
    }
    color = vec4(base.rgb * lit, base.a);
})";
}
