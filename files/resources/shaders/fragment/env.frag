#version 300 es

precision mediump float;

in vec3 vWorldNormal;
in vec3 vWorldPosition;

out vec4 fragColor;

uniform samplerCube cubemap;
uniform vec3 uCameraPosition;

void main() {
  vec3 worldNormal = normalize(vWorldNormal);
  vec3 eyeToSurface = normalize(vWorldPosition - uCameraPosition);
  vec3 direction = reflect(eyeToSurface, worldNormal);

  fragColor = texture(cubemap, direction);
}