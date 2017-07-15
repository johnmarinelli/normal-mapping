#version 330

uniform sampler2D brickTexture;
uniform sampler2D normalMap;

in VS_OUT {
  vec3 fPos;
  vec2 fTexCoords;
  vec3 fTangentLightPosition;
  vec3 fTangentViewPosition;
  vec3 fTangentFragPosition;
} fs_in;

out vec4 fragColor;

void main() {
  vec3 normal = texture(normalMap, fs_in.fTexCoords).rgb;
  // map [0,1] to [-1,1]
  // now we are in tangent space
  normal = normal * 2.0 - 1.0;
    
  vec3 brickColor = texture(brickTexture, fs_in.fTexCoords).rgb;
  
  /*
   * Lighting calculations in tangent space
   */
  // ambient
  vec3 ambient = 0.1 * brickColor;
  
  // diffuse
  vec3 lightDir = normalize(fs_in.fTangentLightPosition - fs_in.fTangentFragPosition);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * brickColor;
  
  // specular
  vec3 viewDir = normalize(fs_in.fTangentViewPosition - fs_in.fTangentFragPosition);
  vec3 reflectDir = reflect(-lightDir, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
  vec3 specular = vec3(0.2) * spec;
  
  fragColor = vec4(ambient + diffuse + specular, 1.0);
}
