#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram() :
mInitialized(false),
mProgramID(-1) {
  mShaderCount = 0;
}

void ShaderProgram::loadViewPosition(const glm::vec3& viewPosition) {
  auto location = mUniforms["viewPosition"];
  //glUniform3fv(location, 1, glm::value_ptr(viewPosition));
  glUniform3f(location, viewPosition.x, viewPosition.y, viewPosition.z);
}

void ShaderProgram::loadLightPosition(const glm::vec3& lightPosition) {
  auto location = mUniforms["lightPosition"];
  glUniform3fv(location, 1, glm::value_ptr(lightPosition));
}

void ShaderProgram::loadModelMatrix(const glm::mat4 &model) {
  auto location = mUniforms["modelMatrix"];
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(model));
}

void ShaderProgram::loadViewMatrix(const glm::mat4 &view) {
  auto location = mUniforms["viewMatrix"];
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(view));
}

void ShaderProgram::loadProjectionMatrix(const glm::mat4 &projection) {
  auto location = mUniforms["projectionProjection"];
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));
}

void ShaderProgram::setInt(const std::string& name, GLuint val) {
  auto loc = mUniforms[name];
  glUniform1i(loc, val);
}

void ShaderProgram::initFromFiles(std::string vertexShaderFilename, std::string fragmentShaderFilename) {
  if (-1 == mProgramID) mProgramID = glCreateProgram();
  std::string vertexShaderSource = loadShaderFromFile(vertexShaderFilename);
  std::string fragmentShaderSource = loadShaderFromFile(fragmentShaderFilename);
  
  initialize(vertexShaderSource, fragmentShaderSource);
  use();
}

void ShaderProgram::initFromString(std::string vertexShaderSource, std::string fragmentShaderSource) {
  if (-1 == mProgramID) mProgramID = glCreateProgram();
  initialize(vertexShaderSource, fragmentShaderSource);
  use();
}

GLuint ShaderProgram::attribute(const std::string attributeName) {
  static std::map<std::string, int>::const_iterator attributeItr;
  
  attributeItr = mAttributes.find(attributeName);
  if (mAttributes.end() == attributeItr) {
    throw std::runtime_error("Could not find attribute in shader program: " + attributeName);
  }
  
  return mAttributes[attributeName];
}

GLuint ShaderProgram::uniform(const std::string uniformName) {
  static std::map<std::string, int>::const_iterator uniformItr;
  
  uniformItr = mUniforms.find(uniformName);
  
  if (mUniforms.end() == uniformItr) {
    throw std::runtime_error("Could not find uniform in shader program: " + uniformName);
  }
  
  return mUniforms[uniformName];
}

int ShaderProgram::registerAttribute(const std::string attributeName) {
  // add attribute location value for attributeName key
  mAttributes[attributeName] = glGetAttribLocation(mProgramID, attributeName.c_str());
  
  if (-1 == mAttributes[attributeName]) {
    throw std::runtime_error("Could not add attribute: " + attributeName + " - location returned -1.");
  }
  else {
    std::cout << "Attribute " << attributeName << " bound to location: " << mAttributes[attributeName] << '\n';
  }
  
  return mAttributes[attributeName];
}

int ShaderProgram::registerUniform(const std::string uniformName) {
  mUniforms[uniformName] = glGetUniformLocation(mProgramID, uniformName.c_str());
  
  if (-1 == mUniforms[uniformName]) {
    throw std::runtime_error("Could not add uniform: " + uniformName + " - location returned -1.");
  }
  else {
    std::cout << "Uniform " << uniformName << " bound to location: " << mUniforms[uniformName] << '\n';
  }
  
  return mUniforms[uniformName];
}

//static const bool DEBUG = true;

// differentiate between programs and shaders
enum class ObjectType {
  SHADER, PROGRAM
};

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

std::string ShaderProgram::getInfoLog(ObjectType type, int id) {
  GLint infoLogLength;
  if (ObjectType::SHADER == type) {
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
  }
  else {
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
  }
  
  GLchar* infoLog = new GLchar[infoLogLength + 1];
  
  if (ObjectType::SHADER == type) {
    glGetShaderInfoLog(id, infoLogLength, NULL, infoLog);
  }
  else {
    glGetProgramInfoLog(id, infoLogLength, NULL, infoLog);
  }
  
  std::string infoLogString{infoLog};
  
  delete[] infoLog;
  
  
  return infoLogString;
}

GLuint ShaderProgram::compileShader(std::string shaderSource, GLenum shaderType) {
  std::string shaderTypeString;
  switch(shaderType) {
    case GL_VERTEX_SHADER:
      shaderTypeString = "GL_VERTEX_SHADER";
      break;
    case GL_FRAGMENT_SHADER:
      shaderTypeString = "GL_FRAGMENT_SHADER";
      break;
    case GL_GEOMETRY_SHADER:
      throw std::runtime_error("Geometry shaders are currently unsupported.");
      break;
    default:
      throw std::runtime_error("Bad shader type enum in ShaderProgram::compileShader");
      break;
  }
  
  GLuint shaderID = glCreateShader(shaderType);
  
  if (0 == shaderID) {
    throw std::runtime_error("Couldn't create shader of type " + shaderTypeString + getInfoLog(ObjectType::SHADER, shaderID));
  }
  
  // get source string as char*
  const char* shaderSourceChars = shaderSource.c_str();
  
  glShaderSource(shaderID, 1, &shaderSourceChars, NULL);
  
  glCompileShader(shaderID);
  
  GLint shaderStatus;
  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderStatus);
  if (GL_FALSE == shaderStatus) {
    throw std::runtime_error(shaderTypeString + " compilation failed: " + getInfoLog(ObjectType::SHADER, shaderID));
  }
  else {
    std::cout << shaderTypeString << " shader compilation successful." << '\n';
  }
  
  return shaderID;
}

// throws error if either vertex or fragment shader compilation fails.
void ShaderProgram::initialize(std::string vertexShaderSource, std::string fragmentShaderSource) {
  mVertexShaderID = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
  mFragmentShaderID = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
  
  glAttachShader(mProgramID, mVertexShaderID);
  glAttachShader(mProgramID, mFragmentShaderID);
  
  glLinkProgram(mProgramID);
  
  glDetachShader(mProgramID, mVertexShaderID);
  glDetachShader(mProgramID, mFragmentShaderID);
  
  GLint programLinkSuccess = GL_FALSE;
  glGetProgramiv(mProgramID, GL_LINK_STATUS, &programLinkSuccess);
  
  if (GL_TRUE == programLinkSuccess) {
    std::cout << "Shader program link successful." << '\n';
  }
  else {
    throw std::runtime_error("Shader program link failed: " + getInfoLog(ObjectType::PROGRAM, mProgramID));
  }
  
  glValidateProgram(mProgramID);
  
  /*GLint programValidationStatus;
   
   glGetProgramiv(mProgramID, GL_VALIDATE_STATUS, &programValidationStatus);
   if (GL_TRUE == programValidationStatus) {
   std::cout << "Shader program validation successful. " << '\n';
   }
   else {
   throw std::runtime_error("Shader program validation failed: " + getInfoLog(ObjectType::PROGRAM, mProgramID));
   }
   */
  
  mInitialized = true;
}

std::string ShaderProgram::loadShaderFromFile(const std::string& filename) {
  std::ifstream file(filename.c_str());
  
  if (!file.good()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }
  
  std::stringstream stream;
  stream << file.rdbuf();
  file.close();
  return stream.str();
}

void ShaderProgram::cleanUp() {
  glDeleteProgram(mProgramID);
}
