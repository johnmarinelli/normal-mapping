#include <iostream>
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "stb_image.h"
#include "ShaderProgram.hpp"

Camera camera;
bool firstMouse = true;
const int VIEWPORT_WIDTH = 640;
const int VIEWPORT_HEIGHT = 480;
float lastX = VIEWPORT_WIDTH / 2.0f;
float lastY = VIEWPORT_HEIGHT / 2.0f;
GLuint loadTexture(const std::string&);

void scrollCallback(GLFWwindow* window, double xOff, double yOff) {
  camera.ProcessMouseScroll(yOff);
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, 0.16);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, 0.16);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, 0.16);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, 0.16);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
  if (firstMouse)
   {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
   }
  
  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
  
  lastX = xpos;
  lastY = ypos;
  
  camera.ProcessMouseMovement(xoffset, yoffset);
}

float quadVertices[] = {
  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0, 1.0,
  -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0, 0.0,
  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0, 0.0,
  
  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0, 1.0,
  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0, 0.0,
  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0, 1.0
};

GLuint loadQuad() {
  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
  glBindVertexArray(0);
  return vao;
}

GLuint loadNormalMappedQuad() {
  GLuint vao, vbo;
  
  // positions
  glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
  glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
  glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
  glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
  // texture coordinates
  glm::vec2 uv1(0.0f, 1.0f);
  glm::vec2 uv2(0.0f, 0.0f);
  glm::vec2 uv3(1.0f, 0.0f);
  glm::vec2 uv4(1.0f, 1.0f);
  // normal vector
  glm::vec3 nm(0.0f, 0.0f, 1.0f);
  
  // calculate tangent/bitangent vectors of both triangles
  glm::vec3 tangent1, bitangent1;
  glm::vec3 tangent2, bitangent2;
  // triangle 1
  // ----------
  glm::vec3 edge1 = pos2 - pos1;
  glm::vec3 edge2 = pos3 - pos1;
  glm::vec2 deltaUV1 = uv2 - uv1;
  glm::vec2 deltaUV2 = uv3 - uv1;
  
  GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  
  tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tangent1 = glm::normalize(tangent1);
  
  bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitangent1 = glm::normalize(bitangent1);
  
  // triangle 2
  // ----------
  edge1 = pos3 - pos1;
  edge2 = pos4 - pos1;
  deltaUV1 = uv3 - uv1;
  deltaUV2 = uv4 - uv1;
  
  f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  
  tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  tangent2 = glm::normalize(tangent2);
  
  
  bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  bitangent2 = glm::normalize(bitangent2);
  
  
  float quadVertices[] = {
    // positions, normal, texcoords, tangent, bitangent
    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
    pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
    
    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
  };
  
  // configue vao
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*) (3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*) (6 * sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*) (8 * sizeof(float)));
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*) (11 * sizeof(float)));
  
  return vao;
}

int main(int argc, const char * argv[]) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* window = glfwCreateWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, "Manual Normal Mapping", NULL, NULL);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetScrollCallback(window, scrollCallback);
  glfwSetCursorPosCallback(window, mouseCallback);
  
  if (nullptr == window) {
    std::cout << "Failed to create GLFWwindow.\n";
    glfwTerminate();
    return -1;
  }
  
  glfwMakeContextCurrent(window);
  
  glEnable(GL_DEPTH_TEST);
  
  camera.Position.z = 5.0f;
  
  ShaderProgram shader;
  shader.initFromFiles("shader.vert", "shader.frag");
  //shader.registerUniform("viewPosition");
  shader.registerUniform("projectionMatrix");
  shader.registerUniform("viewMatrix");
  shader.registerUniform("modelMatrix");
  shader.registerUniform("brickTexture");
  shader.registerUniform("normalMap");

  shader.setInt("brickTexture", 0);
  shader.setInt("normalMap", 1);
  
  GLuint vao = loadNormalMappedQuad();
  
  GLuint brickTexture = loadTexture("brickwall.jpg");
  GLuint normalMap = loadTexture("brickwall_normal.jpg");
  
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    processInput(window);
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(camera.Zoom, 640.0f / 480.0f, 0.1f, 100.0f);
    glm::mat4 model;
    model = glm::rotate(model, (float) glfwGetTime() * 0.5f, glm::vec3{0.0f, 1.0f, 0.0f});
    
    shader.use();
    shader.loadProjectionMatrix(projection);
    shader.loadViewMatrix(view);
    shader.loadModelMatrix(model);    
        
    glBindVertexArray(vao); 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    shader.disable();
    
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  
  return 0;
}

GLuint loadTexture(const std::string& path) {
  auto pathCStr = path.c_str();
  GLuint texID;
  glGenTextures(1, &texID);
  
  int width, height, numComponents;
  
  unsigned char* data = stbi_load(pathCStr, &width, &height, &numComponents, 0);
  
  if (data) {
    GLenum format;
    if (1 == numComponents) {
      format = GL_RED;
    }
    else if (3 == numComponents) {
      format = GL_RGB;
    }
    else if (4 == numComponents) {
      format = GL_RGBA;
    }
    
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(data);
  }
  else {
    std::cout << "Texture failed to load at path " << path << '\n';
    stbi_image_free(data);
  }
  
  
  return texID;
}

