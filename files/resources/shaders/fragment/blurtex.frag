precision highp float;

varying highp vec2 vFragmentTextureCoord;

uniform sampler2D sampler;
uniform vec2 uTexImageSize;

void main() {
  // vec2 textureCoord = gl_FragCoord.xy / uTexImageSize;
  vec2 textureCoord = vFragmentTextureCoord;
  vec2 onePixel = vec2(1.0, 1.0) / uTexImageSize;

  gl_FragColor = (
    texture2D(sampler, textureCoord + onePixel*vec2(-1.0, -1.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(0.0, -1.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(1.0, -1.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(-1.0, 0.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(0.0, 0.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(1.0, 0.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(-1.0, 1.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(0.0, 1.0)) +
    texture2D(sampler, textureCoord + onePixel*vec2(1.0, 1.0)) 
  ) / 9.0;
}