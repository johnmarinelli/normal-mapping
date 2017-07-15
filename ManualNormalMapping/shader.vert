#version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vBiTangent;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out VS_OUT {
  vec3 fPos;
  vec2 fTexCoords;
  vec3 fTangentLightPosition;
  vec3 fTangentViewPosition;
  vec3 fTangentFragPosition;
} vs_out;

const vec3 lightPosition = vec3(0.5, 1.0, 0.3);

void main() {
  vs_out.fPos = vec3(modelMatrix * vec4(vPos, 1.0));
  vs_out.fTexCoords = vTexCoords;
  // extract view position from viewMatrix
  vec3 viewPosition = vec3(inverse(viewMatrix)[3]);
  
  // http://web.archive.org/web/20120228095346/http://www.arcsynthesis.org/gltut/Illumination/Tut09%20Normal%20Transformation.html
  mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
  vec3 T = normalize(normalMatrix * vTangent);
  vec3 N = normalize(normalMatrix * vNormal);
  
  // Gram-Schmidt process to re orthogonalize T wrt N
  T = normalize(T - dot(T,N) * N);
  vec3 B = cross(T, N);
  
  mat3 TBN = transpose(mat3(T,B,N));
  vs_out.fTangentLightPosition = TBN * lightPosition;
  vs_out.fTangentViewPosition = TBN * viewPosition;
  vs_out.fTangentFragPosition = TBN * vs_out.fPos;
  
  gl_Position = projectionMatrix * viewMatrix * vec4(vs_out.fPos, 1.0);
}
