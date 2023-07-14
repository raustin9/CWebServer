attribute vec4 aVertexPosition;

varying vec4 vPosition;

void main() {
  gl_Position = aVertexPosition;
  gl_Position.z = 1.0;
  vPosition = aVertexPosition;
  vPosition.z = 1.0;

}