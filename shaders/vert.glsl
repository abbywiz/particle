#version 410 core
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;
layout(location=1) in vec3 vertexColors;

// Uniform variables
//uniform mat4 u_ModelMatrix;
//uniform mat4 u_ViewMatrix;
//uniform mat4 u_Projection; // We'll use a perspective projection
uniform mat4 MVP;

// Pass vertex colors into the fragment shader
out vec3 v_vertexColors;

void main()
{
  v_vertexColors = vertexColors;

  // TODO: Utilize your MVP matrix
  vec4 newPosition =  MVP  * vec4(position,1.0f);
                                    // Don't forget 'w=1.0f'
	gl_Position = vec4(newPosition.x, newPosition.y, newPosition.z, newPosition.w);
}


