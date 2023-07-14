#version 100

precision highp float;

varying highp vec2 vFragmentTextureCoord;
varying vec3 vertPos;
varying vec4 transformedNormal;

uniform sampler2D sampler;
uniform vec3 uLightPosition;
uniform vec2 uTexImageSize;

// This function performs a 3x3 gausian blur on the texture
vec4 blur() {
  vec2 textureCoord = vFragmentTextureCoord;
  vec2 onePixel = vec2(1.0, 1.0) / uTexImageSize;

  vec4 t1 = texture2D(sampler, textureCoord + onePixel*vec2(-1.0, -1.0));
  vec4 t2 = texture2D(sampler, textureCoord + onePixel*vec2(0.0, -1.0));
  vec4 t3 = texture2D(sampler, textureCoord + onePixel*vec2(1.0, -1.0));
  vec4 t4 = texture2D(sampler, textureCoord + onePixel*vec2(-1.0, 0.0));
  vec4 t5 = texture2D(sampler, textureCoord + onePixel*vec2(0.0, 0.0));
  vec4 t6 = texture2D(sampler, textureCoord + onePixel*vec2(1.0, 0.0));
  vec4 t7 = texture2D(sampler, textureCoord + onePixel*vec2(-1.0, 1.0));
  vec4 t8 = texture2D(sampler, textureCoord + onePixel*vec2(0.0, 1.0));
  vec4 t9 = texture2D(sampler, textureCoord + onePixel*vec2(1.0, 1.0));

  return vec4(
    t1 * 0.045 +
    t2  * 0.122 +
    t3 * 0.045 +
    t4 * 0.122 +
    t5 * 0.332 +
    t6 * 0.122 +
    t7 * 0.045 +
    t8 * 0.122 +
    t9 * 0.045
  );
}

void main() {
  float shininess = 20.0;
  vec3 N = normalize(transformedNormal.xyz);
  vec3 L = normalize(uLightPosition - vertPos);

  float lambertian = max(dot(N, L), 0.0);
  float specular = 0.0;
  if (lambertian > 0.0) {
    vec3 R = reflect(-L, N);
    vec3 V = normalize(-vertPos);
    float specAngle = max(dot(R, V), 0.0);
    specular = pow(specAngle, shininess);
  }

  highp vec3 directionalLightColor = vec3(1,1,1);
  highp vec3 directionalVector = normalize(vec3(vertPos - uLightPosition));

  highp float directional = max(
    dot(normalize(transformedNormal.xyz), directionalVector), 0.0
  );

  vec3 lighting = (directionalLightColor * directional);

  vec4 texColor = blur();

  gl_FragColor = vec4(
    texColor.rgb +
    texColor.rbg * lighting +
    specular * vec3(1,1,1)
    , texColor.a);
}