precision highp float;

uniform sampler2D tex_norm;
uniform sampler2D tex_diffuse;
uniform vec3 uLightPosition;

varying vec2 frag_uv;
varying vec3 ts_light_pos;
varying vec3 ts_view_pos;
varying vec3 ts_frag_pos;

varying vec4 transformedNormal;
varying vec3 vertPos;

void main(void)
{
    float shininess = 20.0;
    vec3 N = normalize(transformedNormal.xyz);
    vec3 L = normalize(uLightPosition);
    vec3 light_dir = normalize(ts_light_pos - ts_frag_pos);
    
    float lambertian = max(dot(N, L), 0.0);
    float specular = 0.0;
    if (lambertian > 0.0) {
        vec3 R = reflect(-L, N);
        vec3 V = normalize(-vertPos);
        float specAngle = max(dot(R, V), 0.0);
        specular = pow(specAngle, shininess);
    }

    highp vec3 directionalLightColor = vec3(1,1,1);
    highp vec3 directionalVector = normalize(vec3(uLightPosition));

    highp float directional = max(
        dot(normalize(transformedNormal.xyz), directionalVector), 0.0
    );

    vec3 lighting = (directionalLightColor * directional);

    vec3 albedo = texture2D(tex_diffuse, frag_uv).rgb;
    vec3 ambient = 0.3 * albedo;

    // Normal mapping
    vec3 norm = normalize(texture2D(tex_norm, frag_uv).rgb * 2.0 - 1.1);
    float diffuse = max(dot(light_dir, norm), 0.0);

    vec4 texColor = vec4(
        diffuse * albedo + ambient * lighting
    , 1.0);

    gl_FragColor = vec4(
        0.15 * texColor.rgb + 
        texColor.rgb * lighting + 
        specular * vec3(1,1,1)
        ,texColor.a
    );
}