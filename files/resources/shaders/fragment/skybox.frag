precision mediump float;

varying vec4 vPosition;

uniform samplerCube cubemap;
uniform mat4 uWorldViewProjectionInverse;

void main() {
  vec4 t = uWorldViewProjectionInverse * vPosition;
  gl_FragColor = textureCube(cubemap, normalize(t.xyz / t.w));
}