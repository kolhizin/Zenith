#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 diffColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	vec2 tdx = vec2(0.001f, 0.0f);
	vec2 tdy = vec2(0.0f, 0.001f);

	float horScaleFactor = 100.0f;
	float vertScaleFactor = 10.0f;

	float h0 = texture(texSampler, inPosition).x * 0.4
		+ 0.15 * texture(texSampler, inPosition + tdx).x
		+ 0.15 * texture(texSampler, inPosition + tdy).x
		+ 0.15 * texture(texSampler, inPosition - tdx).x
		+ 0.15 * texture(texSampler, inPosition - tdy).x;

	vec2 hdx = inPosition + vec2(1.0f, 0.0f)*0.002f;
	vec2 hdy = inPosition + vec2(0.0f, 1.0f)*0.002f;

	float h0x = texture(texSampler, hdx).x;

	float h0y = texture(texSampler, hdy).x;

	

	vec3 vdx = vec3(hdx * horScaleFactor, -(h0x - h0) * vertScaleFactor);
	vec3 vdy = vec3(hdy * horScaleFactor, -(h0y - h0) * vertScaleFactor);
	vec3 nrm = normalize(cross(vdx, vdy));
	
	diffColor = vec3(1.0f, 1.0f, 1.0f) * max(0.0f, dot(nrm, normalize(vec3(-50.0f, 0.0f, 150.0f))));
	//diffColor = vec3(1.0f, 1.0f, 1.0f) * h0;
	
    gl_Position = ubo.proj * ubo.view * vec4(inPosition * horScaleFactor, -h0 * vertScaleFactor, 1.0f);
    fragTexCoord = inPosition;
}