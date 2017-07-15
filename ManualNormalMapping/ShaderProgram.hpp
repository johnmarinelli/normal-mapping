#ifndef ShaderProgram_hpp
#define ShaderProgram_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <OpenGL/gl3.h>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
  
  ShaderProgram();
  
  void loadViewPosition(const glm::vec3& viewPosition);
  void loadLightPosition(const glm::vec3& lightPosition);
  void loadModelMatrix(const glm::mat4& model);
  void loadViewMatrix(const glm::mat4& view);
  void loadProjectionMatrix(const glm::mat4& projection);
  
  void setInt(const std::string& name, GLuint val);
  
  void initFromFiles(std::string vertexShaderFilename, std::string fragmentShaderFilename);
  
  void initFromString(std::string vertexShaderSource, std::string fragmentShaderSource);
  
  // differentiate between programs and shaders
  enum class ObjectType {
    SHADER, PROGRAM
  };
  
  std::string getInfoLog(ObjectType type, int id);
  
  GLuint compileShader(std::string shaderSource, GLenum shaderType);
  
  // throws error if either vertex or fragment shader compilation fails.
  void initialize(std::string vertexShaderSource, std::string fragmentShaderSource);
  
  std::string loadShaderFromFile(const std::string& filename);
  
  inline void use() {
    if (mInitialized) {
      glUseProgram(mProgramID);
    }
    else {
      std::string msg = "Shader program " + std::to_string(mProgramID);
      msg += "not initialized.  Aborting.";
      throw std::runtime_error(msg);
    }
  }
  
  inline void disable() {
    glUseProgram(0);
  }
  
  // calls glDeleteProgram on this program.
  // SHOULD BE ONLY PLACE glDeleteProgram IS CALLED.
  void cleanUp();
  
  GLuint attribute(const std::string attributeName);
  GLuint uniform(const std::string uniformName);
  
  int registerAttribute(const std::string attributeName);
  int registerUniform(const std::string uniformName);
  
  //static const bool DEBUG = true;
  
  // individual shader ids
  GLint mProgramID;
  GLuint mVertexShaderID;
  GLuint mFragmentShaderID;
  
  // # of shaders attached to program
  GLuint mShaderCount;
  
  // map of attributes and their binding locations
  std::map<std::string, int> mAttributes;
  
  // map of uniforms and their binding locations
  std::map<std::string, int> mUniforms;
  
  // has this program been initialized?
  bool mInitialized;
};

#endif
