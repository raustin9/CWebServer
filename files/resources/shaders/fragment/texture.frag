#version 100

precision highp float;

varying highp vec2 vFragmentTextureCoord;
varying vec3 vertPos;
varying vec4 transformedNormal;

uniform sampler2D sampler;
uniform vec3 uLightPosition;

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
  highp vec3 directionalVector = normalize(vec3(uLightPosition - vertPos));

  highp float directional = max(
    dot(normalize(transformedNormal.xyz), directionalVector), 0.0
  );

  vec3 lighting = (directionalLightColor * directional);

  vec4 texColor = texture2D(sampler, vFragmentTextureCoord);
  gl_FragColor = vec4(
    0.15 * texColor.rgb + 
    texColor.rbg * lighting +
    specular * vec3(1,1,1)
    , texColor.a);
}