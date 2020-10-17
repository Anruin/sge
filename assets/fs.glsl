#version 330 core

// Interpolated values from the vertex shaders
in vec3 norm;
in vec3 pos;
in vec3 eyepos;
//Tex format: 4 smudge texNrs + texNr,U,V
in vec4 tex1;
in vec3 tex4;

// Output data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texture;

void main()
{

//Jod, main texture
	float grad = abs(1.0-tex4.y-tex4.z);
	bool odd = tex4.y+tex4.z > 1.0;
	float shft = 0.125*round(tex4.x/grad);
	float maintx = shft;
	
	float def = 0.002 + 0.121*tex4.y;
	float def2 = 0.002 + 0.121*tex4.z;
	
	vec2 texcrd;
	texcrd.x = def + shft;
	texcrd.y = def2 + 0.125*floor(shft);

	vec3 N = normalize(norm);
	vec3 EyeDir = normalize(eyepos - pos);
	float NdotV = clamp(dot(EyeDir, N),0,1);
	
 	if ( shft == 0.75 )
	{
		vec2 ArcExy; ArcExy.x = EyeDir.x; ArcExy.y = EyeDir.y;
		vec2 ArcEyz; ArcEyz.x = EyeDir.y; ArcEyz.y = EyeDir.z;
		vec2 ArcNxy; ArcNxy.x = N.x; ArcNxy.y = N.y;
		vec2 ArcNyz; ArcNyz.x = N.y; ArcNyz.y = N.z;
		normalize(ArcExy); normalize(ArcEyz); normalize(ArcNxy); normalize(ArcNyz); 
		texcrd.x = 0.001 + 0.123*(0.5 + mod(asin(ArcExy.x) - asin(ArcNxy.x),1)*0.5) + shft;
		texcrd.y = 0.001 + 0.123*(0.5 + mod(asin(ArcEyz.x) - asin(ArcNyz.x),1)*0.5);
	}
	
	vec3 color1 = texture2D(texture, texcrd).rgb;
	float edge;
	if ( shft == 0.25 )
	{
		edge = clamp(pow(NdotV, 15), 0.0, 1.0);
		if ( edge > 0.8 ) color1 *= (1 + edge*0.5);
	}

 
//Jod, smudge textures
	
	def = 0.125 - def;
	def2 = 0.125 - def2;
	
	float modo;
	float modn;
	if (odd)
	{
		modo = 1.0-tex4.z;
		modn = 1.0-tex4.y;
	}
	else
	{
		modo = tex4.y;
		modn = tex4.z;
	}
	
	int tcount = 0;
	shft = 0.125*round(tex1.z/modo); if ( shft > 0 ) tcount++; if (( shft == 0.75 )&&(shft != maintx)) shft=0.625; 
	if ( shft != 0.75 ){ texcrd.x = def + shft; texcrd.y = def2 + 0.125*floor(shft); }
	vec3 color2 = texture2D(texture, texcrd).rgb;
	shft = 0.125*round(tex1.w/modo); if ( shft > 0 ) tcount++; if (( shft == 0.75 )&&(shft != maintx)) shft=0.625;
	if ( shft != 0.75 ){ texcrd.x = def + shft; texcrd.y = def2 + 0.125*floor(shft); }
	color2 += texture2D(texture, texcrd).rgb;

	if (tcount > 0) color2/=tcount;
	
	tcount=0;
	shft = 0.125*round(tex1.x/modn); if ( shft > 0 ) tcount++; if (( shft == 0.75 )&&(shft != maintx)) shft=0.625;
	if ( shft != 0.75 ){ texcrd.x = def + shft; texcrd.y = def2 + 0.125*floor(shft); }
	vec3 color3 = texture2D(texture, texcrd).rgb;
	shft = 0.125*round(tex1.y/modn); if ( shft > 0 ) tcount++; if (( shft == 0.75 )&&(shft != maintx)) shft=0.625;
	if ( shft != 0.75 ){ texcrd.x = def + shft; texcrd.y = def2 + 0.125*floor(shft); }
	color3 += texture2D(texture, texcrd).rgb;
	
	if (tcount > 0) color3/=tcount;

//Jod, output color
	edge = 1.0-NdotV;
	color = clamp( (color1*grad + color2*modo + color3*modn)*(1.0-edge*0.5), 0, 1 );

}