#version 330 core // Header for OpenGL 3.3

// Vertex shader - this code is executed for every vertex of the shape

// Inputs coming from VBOs
layout (location = 0) in vec3 vertex_position; // vertex position in local space (x,y,z)
layout (location = 1) in vec3 vertex_normal;   // vertex normal in local space   (nx,ny,nz)
layout (location = 2) in vec3 vertex_color;    // vertex color      (r,g,b)
layout (location = 3) in vec2 vertex_uv;       // vertex uv-texture (u,v)

// Output variables sent to the fragment shader
out struct fragment_data
{
    vec3 position; // vertex position in world space
    vec3 normal;   // normal position in world space
    vec3 color;    // vertex color
    vec2 uv;       // vertex uv
} fragment;

// Uniform variables expected to receive from the C++ program
uniform mat4 model; // Model affine transform matrix associated to the current shape
uniform mat4 view;  // View matrix (rigid transform) of the camera
uniform mat4 projection; // Projection (perspective or orthogonal) matrix of the camera


// Additional uniform variable representing the time for the deformation
uniform float time;


// Deformer function

//  Computes the procedural deformation z = 0.05*cos( (x^2+y^2)^0.5 - 3*t );
vec3 deformer(vec3 p0)
{
	float d1 = sqrt(p0.x*p0.x+p0.y*p0.y);
	float omega1 = 25.0*d1 - 3.0*time;
	//float d2 = sqrt((p0.x - 4.8f) * (p0.x - 4.8f) + (p0.y - 4.8f) * (p0.y - 4.8f));
	//float omega2 = 10.0*d2 - 3.0*time;
	//float offset_3 = 5.0f;
	//float d3 = sqrt((p0.x - offset_3) * (p0.x - offset_3) + (p0.y + offset_3) * (p0.y + offset_3));
	//float omega3 = 10.0*d3 - 3.0*time;

	float amplitude = 0.05 * (1.0 - smoothstep(0.0, 1.0, d1 / 15.0));

	vec3 p1 = vec3(p0.x, p0.y, amplitude*cos(omega1) );
	//vec3 p2 = vec3(p0.x, p0.y, amplitude*cos(omega2) );
	//vec3 p3 = vec3(p0.x, p0.y, amplitude*cos(omega3) );

	return p1;
}

// Deformer function for the normal
//  Compute the partial derivative of the function
vec3 deformer_normal(vec3 p0)
{
	float d1 = sqrt(p0.x*p0.x+p0.y*p0.y);
	float omega1 = 10.0*d1 - 3.0*time;
	
	//float d2 = sqrt((p0.x - 4.8f) * (p0.x - 4.8f) + (p0.y - 4.8f) * (p0.y - 4.8f));
	//float omega2 = 15.0*d2 - 1.0*time;
	//float offset_3 = 5.0f;
	//float d3 = sqrt((p0.x - offset_3) * (p0.x - offset_3) + (p0.y + offset_3) * (p0.y + offset_3));
	//float omega3 = 20.0*d3 - 5.0*time;

	float amplitude = 0.05 * (1.0 - smoothstep(0.0, 1.0, d1 / 15.0));

	// Compute exact normals after deformation
	vec3 dpdx1 = vec3(1.0, 0.0, -10.0*p0.x/d1*amplitude*sin(omega1) );
	vec3 dpdy1 = vec3(0.0, 1.0, -10.0*p0.y/d1*amplitude*sin(omega1) );
	//vec3 dpdx2 = vec3(1.0, 0.0, -15.0*p0.x/d2*amplitude*sin(omega2) );
	//vec3 dpdy2 = vec3(0.0, 1.0, -15.0*p0.y/d2*amplitude*sin(omega2) );
	//vec3 dpdx3 = vec3(1.0, 0.0, -20.0*p0.x/d3*amplitude*sin(omega3) );
	//vec3 dpdy3 = vec3(0.0, 1.0, -20.0*p0.y/d3*amplitude*sin(omega3) );
	vec3 n = normalize(cross(dpdx1,dpdy1));

	return n;
}

void main()
{
	// Apply deformation
	vec3 p_deformed = deformer(vertex_position);
	vec3 n_deformed = deformer_normal(vertex_position);

	// The position of the vertex in the world space
	vec4 position = model * vec4(p_deformed, 1.0);

	// The normal of the vertex in the world space
	mat4 modelNormal = transpose(inverse(model));
	vec4 normal = modelNormal * vec4(n_deformed, 0.0);

	// The projected position of the vertex in the normalized device coordinates:
	vec4 position_projected = projection * view * position;

	// Fill the parameters sent to the fragment shader
	fragment.position = position.xyz;
	fragment.normal   = normal.xyz;
	fragment.color = vertex_color;
	fragment.uv = vertex_uv;

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	gl_Position = position_projected; // gl_Position is the projected vertex position (in normalized device coordinates)
}
