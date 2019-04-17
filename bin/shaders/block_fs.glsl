uniform sampler2D light;
uniform sampler2D texture;

void main()
{
    vec4 cLight = texture2D(light, gl_TexCoord[0].zw);
    vec4 cTexture = texture2D(texture, gl_TexCoord[0].xy)*gl_Color;
    gl_FragColor.rgb = cTexture.rgb*cLight.rgb;
    gl_FragColor.a = cTexture.a;
}
