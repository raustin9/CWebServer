#version 300 es

precision mediump float;

in vec4 aVertexPosition;
in vec3 aVertexNormal;

out vec3 outLightPos;
out vec3 vWorldNormal;
out vec3 vWorldPosition;

uniform mat4 uNormalMatrix;
uniform vec3 uCameraPosition;
uniform mat4 uWorldViewProjection;
uniform int uRandom;

void main() {
  vec3 cPos = vec3(0, 0, 0);
  gl_Position = uWorldViewProjection * aVertexPosition;

  vWorldPosition = (uWorldViewProjection * aVertexPosition).xyz;

  vWorldNormal = vec3(-uNormalMatrix * vec4(aVertexNormal, 1.0));
  outLightPos = uCameraPosition - aVertexPosition.xyz;
}