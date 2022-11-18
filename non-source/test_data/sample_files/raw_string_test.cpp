/*
    New File;;;;
    Stuff
*/

inline void
GLSL_InitSpriteSheetProgram(memory_arena *Arena, glsl_program *TheSpriteSheetProgram)
{
 //   s32 VertexAttributeCount = ArrayCount(VertexAttributeStrings);
#if R42_SHADER_SET_HOVER && R42_SHADER_SET_POS_DIM
    char *Version = R"(
      #version 100
      #version 100
      #version 100
      #version 100
      #version 100
      #define R42_SHADER_SET_POS_DIM 1
      #define R42_SHADER_SET_HOVER 1
      )";
#endif
}; 

internal void
CompileZBiasProgram(zbias_program *Result, b32x DepthPeel)
{
    char Defines[1024];
    FormatString(sizeof(Defines), Defines,
                 "#version 130\n"
                 "#define ShaderSimTexWriteSRGB %d\n"
                 "#define ShaderSimTexReadSRGB %d\n"
                 "#define DepthPeel %d\n",
                 OpenGL.ShaderSimTexWriteSRGB,
                 OpenGL.ShaderSimTexReadSRGB,
                 DepthPeel);
    
    char *VertexCode = R"FOO(
// Vertex code
uniform m4x4 Transform;

uniform v3 CameraP;
uniform v3 FogDirection;

in v4 VertP;
in v3 VertN;
in v2 VertUV;
in v4 VertColor;
smooth out v2 FragUV;
smooth out v4 FragColor;
smooth out f32 FogDistance;
smooth out v3 WorldP;
smooth out v3 WorldN;
void main(void)
{
v4 InVertex = V4(VertP.xyz, 1.0);
f32 ZBias = VertP.w;

v4 ZVertex = InVertex;
ZVertex.z += ZBias;

v4 ZMinTransform = Transform*InVertex;
v4 ZMaxTransform = Transform*ZVertex;

f32 ModifiedZ = (ZMinTransform.w / ZMaxTransform.w)*ZMaxTransform.z;

gl_Position = vec4(ZMinTransform.x, ZMinTransform.y, ModifiedZ, ZMinTransform.w);

FragUV = VertUV.xy;
FragColor = VertColor;

FogDistance = Inner(ZVertex.xyz - CameraP, FogDirection);
WorldP = ZVertex.xyz;
WorldN = VertN;
}
)FOO";
    
    char *Fragment = R"FOO(
// Fragment code
//uniform v4 FogColor;
uniform sampler2D TextureSampler;
#if DepthPeel
uniform sampler2D DepthSampler;
#endif
uniform v3 FogColor;
uniform f32 AlphaThreshold;
uniform f32 FogStartDistance;
uniform f32 FogEndDistance;
uniform f32 ClipAlphaStartDistance;
uniform f32 ClipAlphaEndDistance;
uniform v3 CameraP;
uniform v3 LightP;

smooth in vec2 FragUV;
smooth in vec4 FragColor;
smooth in f32 FogDistance;
smooth in vec3 WorldP;
smooth in vec3 WorldN;

layout(location = 0) out v4 BlendUnitColor[3];

void main(void)
{
#if DepthPeel
f32 ClipDepth = texelFetch(DepthSampler, ivec2(gl_FragCoord.xy), 0).r;
if(gl_FragCoord.z <= ClipDepth)
{
discard;
}
#endif

vec4 TexSample = texture(TextureSampler, FragUV);
#if ShaderSimTexReadSRGB
TexSample.rgb *= TexSample.rgb;
#endif

f32 FogAmount = Clamp01MapToRange(FogStartDistance, FogDistance, FogEndDistance);
f32 AlphaAmount = Clamp01MapToRange(ClipAlphaStartDistance, FogDistance, ClipAlphaEndDistance);
v4 ModColor = AlphaAmount*FragColor*TexSample;

if(ModColor.a > AlphaThreshold)
{
v4 SurfaceReflect;
SurfaceReflect.rgb = Lerp(ModColor.rgb, FogAmount, FogColor.rgb);
SurfaceReflect.a = ModColor.a;

#if ShaderSimTexWriteSRGB
SurfaceReflect.rgb = sqrt(SurfaceReflect.rgb);
#endif

// TODO(casey): Some way of specifying light params
f32 Lp0 = 0;
f32 Lp1 = 0;
f32 EmitSpread = 1.0f;
v3 Emit = V3(0, 0, 0);
if(length(LightP - WorldP) < 2.0f)
{
Emit = V3(1, 1, 1);
}

v2 N2 = PackNormal2(WorldN.xy);

BlendUnitColor[0] = SurfaceReflect;
BlendUnitColor[1] = V4(Emit.r, Emit.g, Emit.b, EmitSpread);
BlendUnitColor[2] = V4(N2.x, N2.y, Lp0, Lp1);
}
else
{
discard;
}
}
)FOO";
    
    GLuint Prog = OpenGLCreateProgram(Defines, GlobalShaderHeaderCode, VertexCode, FragmentCode, &Result->Common);
    OpenGLLinkSamplers(&Result->Common, "TextureSampler", "DepthSampler");
    
    Result->TransformID = glGetUniformLocation(Prog, "Transform");
    Result->CameraP = glGetUniformLocation(Prog, "CameraP");
    Result->FogDirection = glGetUniformLocation(Prog, "FogDirection");
    Result->FogColor = glGetUniformLocation(Prog, "FogColor");
    Result->FogStartDistance = glGetUniformLocation(Prog, "FogStartDistance");
    Result->FogEndDistance = glGetUniformLocation(Prog, "FogEndDistance");
    Result->ClipAlphaStartDistance = glGetUniformLocation(Prog, "ClipAlphaStartDistance");
    Result->ClipAlphaEndDistance = glGetUniformLocation(Prog, "ClipAlphaEndDistance");
    Result->AlphaThreshold = glGetUniformLocation(Prog, "AlphaThreshold");
    
    Result->DebugLightP = glGetUniformLocation(Prog, "LightP");
}


