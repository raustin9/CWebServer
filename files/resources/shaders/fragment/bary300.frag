#version 300 es // These is an OpenGL ES 3.0 Shader!

// In's and Out's of a Fragment Shader
// an 'in' inside of a fragment shader is a varying var
// an 'out' inside of a fragment shader is _the_ output you wish to draw (typically a vec4 color)

// We need to tell the shader executable at what precision we want floats to be
// medium precision is a good balance of speed and accuracy.
precision mediump float;

// This is a varying var written to by our vertex shader
// since this is 3.0 we specify it in the fragment shader with "in"
in vec3 outColor;
in vec3 outBary;

// We also have to specify the "output" of the fragment shader
// Typically we only output RGBA color, and that is what I will do here!
out vec4 fragColor;

// Clamp a float value between 0 and 1
float saturateF(float f) {
  return clamp(f, 0.0, 1.0);
}

float isOnTriangleEdge(vec3 b, float e){
  vec3 howClose = smoothstep(vec3(0.0), vec3(e), b);
  return saturateF(1.0 - min(min(howClose.x, howClose.y), howClose.z));
}

void main() {
  float ef = isOnTriangleEdge(outBary, 0.05);
  vec3 newColor = outColor;
  newColor.g *= ef;
  fragColor = vec4(newColor, 1.0);
}