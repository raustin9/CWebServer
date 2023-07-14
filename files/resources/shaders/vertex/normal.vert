precision highp float;

attribute vec4 aVertexPosition;
attribute vec2 aVertexTexCoord;
attribute vec3 aVertexNormal;
attribute vec3 aBarycentricCoord;

uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uWorldViewProjection;
uniform vec3 uLightPosition;

varying highp vec2 vFragmentTextureCoord;
varying vec2 frag_uv;
varying vec3 vertPos;
varying vec4 transformedNormal;
varying vec3 ts_light_pos; // Tangent space values
varying vec3 ts_view_pos;  //
varying vec3 ts_frag_pos;  //

void main(void)
{
    vec4 vertPos4 = uWorldViewProjection * aVertexPosition;
    vertPos = vec3(vertPos4) / vertPos4.w;

    transformedNormal = uNormalMatrix * vec4(aVertexNormal, 1.0);

    gl_Position = uWorldViewProjection * aVertexPosition;
    ts_frag_pos = vec3(uModelViewMatrix * aVertexPosition);

    ts_light_pos = vec3(3, 2, 1);
 
    frag_uv = aVertexTexCoord;
    vFragmentTextureCoord = aVertexTexCoord;
    vec3 temp = aBarycentricCoord * vec3(0.0, 0.0, 0.0);
}