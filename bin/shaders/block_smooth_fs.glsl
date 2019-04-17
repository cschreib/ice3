uniform sampler2D light;
uniform sampler2D texture;

void main()
{
    vec4 cLight = texture2D(light, gl_TexCoord[1].xy);
    vec4 cTexture = texture2D(texture, gl_TexCoord[0].xy)*gl_Color;
    float fOcclusion = 1.0 - cLight.a*0.75*gl_TexCoord[1].z;
    gl_FragColor.rgb = cTexture.rgb*cLight.rgb*fOcclusion;
    gl_FragColor.a = cTexture.a;
}
