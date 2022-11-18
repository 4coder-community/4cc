#version 120

varying vec4 v_TexCoord;

uniform sampler2D u_Sampler;
uniform sampler2D u_NormalMap;

uniform vec4 u_Color;
uniform float u_SpriteSize;
uniform vec2 u_PixelPosition;
uniform float u_Rotation;

float VecLen (vec2 Vec)
{
	return sqrt((Vec.x*Vec.x)+(Vec.y*Vec.y));
}

void main () {

	// vec2 center = vec2(1280/2, 720/2);
	// float light = (500 - distance(center, gl_FragCoord.xy)) / 500;
	// gl_FragColor = texture2D(u_sampler, v_texcoord.xy) * vec4(light, light, light, 1);
	
	// vec4 Color = texture2D(u_Sampler, v_TexCoord.xy);
	// vec4 ColorAvg = vec4((Color.r + Color.g + Color.b) / 3.0f,
	// 	(Color.r + Color.g + Color.b) / 3.0f,
	// 	(Color.r + Color.g + Color.b) / 3.0f,
	// 	Color.a);

	float XDiff = (u_PixelPosition.x-u_SpriteSize) - gl_FragCoord.x;
	float YDiff = (u_PixelPosition.y+u_SpriteSize) - gl_FragCoord.y;
	float DiffLength = sqrt((XDiff*XDiff)+(YDiff*YDiff));

	float LightX = -100.0f;
	float LightY = -100.0f;

	//float Color = 1.0f - (((XDiff)+(YDiff)) / u_SpriteSize);
	float Color = 1.0f - (DiffLength / (u_SpriteSize*2.0f));
	if (Color < 0.1f) Color = 0.1f;
	vec4 LightingColor = vec4(Color, Color, Color, 1.0f);

	vec4 Texture0 = texture2D(u_Sampler, v_TexCoord.xy);
	vec4 Texture1 = texture2D(u_NormalMap, v_TexCoord.xy);
	vec3 Normal = (2.0f*Texture1.xyz)-1.0f;

	vec3 LightPos;
	LightPos.x = (sin(radians(u_Rotation))*-u_SpriteSize)+(cos(radians(u_Rotation))*-u_SpriteSize);
	LightPos.y = (sin(radians(u_Rotation))*-u_SpriteSize)+(cos(radians(u_Rotation))*-u_SpriteSize);
	LightPos.z = 0.0f;
	float Dot = dot(Normal.xyz, LightPos);

	//LightingColor = vec4(vec3(1.0f-(Dot/u_SpriteSize)), 1.0f);
	LightingColor = vec4(1.0f);
	
	gl_FragColor = (Texture0 * u_Color) * (LightingColor);
	//gl_FragColor = vec4(Texture0 * vec4(0.0f, 0.0f, 1.0f, 1.0f));
}
