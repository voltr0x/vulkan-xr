#include "Util.h"



#include <boxer/boxer.h>

#include <cstring>
#include <fstream>
#include <sstream>

void util::error(Error error, const std::string& details)
{
  std::stringstream s;

  switch (error)
  {
  case Error::FeatureNotSupported:
    s << "Required feature is not supported";
    break;
  case Error::FileMissing:
    s << "Failed to find file";
    break;
  case Error::GenericGLFW:
    s << "Program encountered a generic GLFW error";
    break;
  case Error::GenericOpenXR:
    s << "Program encountered a generic OpenXR error";
    break;
  case Error::GenericVulkan:
    s << "Program encountered a generic Vulkan error";
    break;
  case Error::HeadsetNotConnected:
    s << "No headset detected.\nPlease make sure that your headset is connected and running";
    break;
  case Error::ModelLoadingFailure:
    s << "Failed to load model";
    break;
  case Error::OutOfMemory:
    s << "Program ran out of memory";
    break;
  case Error::VulkanNotSupported:
    s << "Vulkan is not supported";
    break;
  case Error::WindowFailure:
    s << "Failed to create window";
    break;
  }

  if (details.empty())
  {
    s << ".";
  }
  else
  {
    s << ": " << details << ".";
  }

  boxer::show(s.str().c_str(), "Error", boxer::Style::Error);
}

bool util::loadXrExtensionFunction(XrInstance instance, const std::string& name, PFN_xrVoidFunction* function)
{
  const XrResult result = xrGetInstanceProcAddr(instance, name.c_str(), function);
  if (XR_FAILED(result))
  {
    return false;
  }

  return true;
}

PFN_vkVoidFunction util::loadVkExtensionFunction(VkInstance instance, const std::string& name)
{
  return vkGetInstanceProcAddr(instance, name.c_str());
}

std::vector<const char*> util::unpackExtensionString(const std::string& string)
{
  std::vector<const char*> out;
  std::istringstream stream(string);
  std::string extension;
  while (getline(stream, extension, ' '))
  {
    const size_t len = extension.size() + 1u;
    char* str = new char[len];
    memcpy(str, extension.c_str(), len);
    out.push_back(str);
  }

  return out;
}

bool util::loadShaderFromFile(VkDevice device, const std::string& filename, VkShaderModule& shaderModule)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open())
  {
    return false;
  }

  const size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> code(fileSize);
  file.seekg(0);
  file.read(code.data(), fileSize);
  file.close();

  VkShaderModuleCreateInfo shaderModuleCreateInfo;
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pNext = nullptr;
  shaderModuleCreateInfo.flags = 0u;
  shaderModuleCreateInfo.codeSize = code.size();
  shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
  if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
  {
    return false;
  }

  return true;
}

bool util::findSuitableMemoryTypeIndex(VkPhysicalDevice physicalDevice,
                                       VkMemoryRequirements requirements,
                                       VkMemoryPropertyFlags properties,
                                       uint32_t& typeIndex)
{
  VkPhysicalDeviceMemoryProperties supportedMemoryProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &supportedMemoryProperties);

  const VkMemoryPropertyFlags typeFilter = requirements.memoryTypeBits;
  for (uint32_t memoryTypeIndex = 0u; memoryTypeIndex < supportedMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
  {
    const VkMemoryPropertyFlags propertyFlags = supportedMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
    if (typeFilter & (1u << memoryTypeIndex) && (propertyFlags & properties) == properties)
    {
      typeIndex = memoryTypeIndex;
      return true;
    }
  }

  return false;
}

VkDeviceSize util::align(VkDeviceSize value, VkDeviceSize alignment)
{
  if (value == 0u)
  {
    return value;
  }

  return (value + alignment - 1u) & ~(alignment - 1u);
}

XrPath util::stringToPath(XrInstance instance, const std::string& string)
{
  XrPath path;
  const XrResult result = xrStringToPath(instance, string.c_str(), &path);
  if (XR_FAILED(result))
  {
    return XR_NULL_PATH;
  }

  return path;
}

bool util::createAction(XrActionSet actionSet,
                        const std::vector<XrPath>& paths,
                        const std::string& actionName,
                        const std::string& localizedActionName,
                        XrActionType type,
                        XrAction& action)
{
  XrActionCreateInfo actionCreateInfo{ XR_TYPE_ACTION_CREATE_INFO };
  actionCreateInfo.actionType = type;
  actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(paths.size());
  actionCreateInfo.subactionPaths = paths.data();

  //strcpy_s(actionCreateInfo.actionName, actionName.c_str());
  //strcpy_s(actionCreateInfo.localizedActionName, localizedActionName.c_str());
  memcpy(actionCreateInfo.actionName, actionName.data(), actionName.length() + 1u);
  memcpy(actionCreateInfo.localizedActionName, localizedActionName.data(), localizedActionName.length() + 1u);

  XrResult result = xrCreateAction(actionSet, &actionCreateInfo, &action);
  if (XR_FAILED(result))
  {
    std::string cPlusPlusRemainsDumbUntilV23 = "XrResult: ";
    cPlusPlusRemainsDumbUntilV23.append(std::to_string((int)result));
    cPlusPlusRemainsDumbUntilV23.append("\n");
    cPlusPlusRemainsDumbUntilV23.append(actionCreateInfo.actionName);
    cPlusPlusRemainsDumbUntilV23.append("\n");
    cPlusPlusRemainsDumbUntilV23.append(actionCreateInfo.localizedActionName);
    cPlusPlusRemainsDumbUntilV23.append("\n");
    cPlusPlusRemainsDumbUntilV23.append(std::to_string(paths.size()));
    cPlusPlusRemainsDumbUntilV23.append("\n");

    boxer::show(cPlusPlusRemainsDumbUntilV23.c_str(), "CreateAction Error", boxer::Style::Error);
    return false;
  }

  return true;
}

XrPosef util::makeIdentity()
{
  XrPosef identity;
  identity.position = { 0.0f, 0.0f, 0.0f };
  identity.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
  return identity;
}

glm::mat4 util::poseToMatrix(const XrPosef& pose)
{
  const glm::mat4 translation =
    glm::translate(glm::mat4(1.0f), glm::vec3(pose.position.x, pose.position.y, pose.position.z));

  const glm::mat4 rotation =
    glm::toMat4(glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));

  return translation * rotation;
}

glm::mat4 util::poseToMatrix(const util::posef& pose)
{
  const glm::mat4 translation =
    glm::translate(glm::mat4(1.0f), glm::vec3(pose.position.x, pose.position.y, pose.position.z));

  const glm::mat4 rotation =
    glm::toMat4(glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));

  return translation * rotation;
}

util::posef util::xrPosefToGlmPosef(const XrPosef& xrPosef){
  util::posef pose = {
    .orientation = util::xrQuaternionfToGlmQuat(xrPosef.orientation), 
    .position = util::xrVector3fToGlmVec3(xrPosef.position)
  };
  return pose;
}

glm::mat4 util::createProjectionMatrix(XrFovf fov, float nearClip, float farClip)
{
  const float l = glm::tan(fov.angleLeft);
  const float r = glm::tan(fov.angleRight);
  const float d = glm::tan(fov.angleDown);
  const float u = glm::tan(fov.angleUp);

  const float w = r - l;
  const float h = d - u;

  glm::mat4 projectionMatrix;
  projectionMatrix[0] = { 2.0f / w, 0.0f, 0.0f, 0.0f };
  projectionMatrix[1] = { 0.0f, 2.0f / h, 0.0f, 0.0f };
  projectionMatrix[2] = { (r + l) / w, (u + d) / h, -(farClip + nearClip) / (farClip - nearClip), -1.0f };
  projectionMatrix[3] = { 0.0f, 0.0f, -(farClip * (nearClip + nearClip)) / (farClip - nearClip), 0.0f };
  return projectionMatrix;
}

glm::quat util::xrQuaternionfToGlmQuat(const XrQuaternionf& src){
  return glm::quat(src.x, src.y, src.z, src.w);
}

XrQuaternionf util::glmQuatToXrQuaternionf(const glm::quat& src){
  return XrQuaternionf(src.x, src.y, src.z, src.w);
}

glm::vec3 util::xrVector3fToGlmVec3(const XrVector3f& src){
  return glm::vec3(src.x, src.y, src.z);
}

XrVector3f util::glmVec3ToXrVector3f(const glm::vec3& src){
  return XrVector3f(src.x, src.y, src.z);
}

glm::quat util::lerpMix(const glm::quat& from, const glm::quat& to, float t) {
	glm::quat src = from * (1.0f - t);
	glm::quat dst = to * t;

	glm::quat q = src + dst;
	return q;
}

float util::dot(const glm::quat& a, const glm::quat& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

glm::vec3 util::lerpMix(const glm::vec3& a, const glm::vec3& b, float w){
  glm::vec3 res = {};
  res.x = a.x + w*(b.x-a.x);
  res.y = a.y + w*(b.y-a.y);
  res.z = a.z + w*(b.z-a.z);
  return res;
}

float util::lerpMix(float a, float b, float w) {
  return a + w*(b-a);
}

float util::dot(const glm::vec3& a, const glm::vec3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

glm::vec3 util::crossProductVector3(const glm::vec3& vector_a, const glm::vec3& vector_b) {
  glm::vec3 temp;
  temp.x = vector_a.y * vector_b.z - vector_a.z * vector_b.y;
  temp.y = -(vector_a.x * vector_b.z - vector_a.z * vector_b.x);
  temp.z = vector_a.x * vector_b.y - vector_a.y * vector_b.x;
  return temp;
}

float util::clampf(float num, float left, float right){
  if(num < left)
    num = left;
  else if (num > right)
    num = right;
  return num;
}

float util::lengthSquared(const glm::vec3& vec){
  return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}

float util::length(const glm::vec3& vec){
  return glm::sqrt(util::lengthSquared(vec));
}

glm::vec3 util::normalize(const glm::vec3& vec){
  float len  = util::length(vec);
  return vec / len;
}

float util::distanceSquared(const glm::vec3& vec1, const glm::vec3& vec2)
{    
  return glm::pow(vec1.x - vec2.x, 2) + glm::pow(vec1.y - vec2.y, 2);
}

float util::distance(const glm::vec3& vec1, const glm::vec3& vec2)
{    
  return glm::sqrt(util::distanceSquared(vec1, vec2));
}

glm::quat util::slerp(const glm::quat& start, const glm::quat& end, float percent)
{
  float cosTheta = util::dot(start, end);
	glm::quat temp(end);


	if (cosTheta < 0.0f) {
		cosTheta *= -1.0f;
		temp = temp * -1.0f;
	}

	float theta = glm::acos(cosTheta);
	float sinThetaDenom = 1.0f / glm::sin(theta);

  glm::quat res = (
		((glm::quat)(start * glm::sin(theta * (1.0f - percent)))) + 
		((glm::quat)(temp * glm::sin(percent * theta)))
		) / sinThetaDenom;
    
	return res;
}

glm::vec3 util::slerp(const glm::vec3& start, const glm::vec3& end, float percent)
{
    // Dot product - the cosine of the angle between 2 vectors.
    float dot = util::dot(start, end);

    // Clamp it to be in the range of Acos()
    // This may be unnecessary, but floating point
    // precision can be a fickle mistress.
    util::clampf(dot, -1.0f, 1.0f);

    // Acos(dot) returns the angle between start and end,
    // And multiplying that by percent returns the angle between
    // start and the final result.
    float theta = glm::acos(dot) * percent;
    glm::vec3 startDotted = start * dot;
    
    glm::vec3 relativeVec = {};
    relativeVec = end - startDotted;

    util::normalize(relativeVec);

    // Orthonormal basis
    // The final result.
    return ((start*glm::cos(theta)) + (relativeVec * glm::sin(theta)));
}

// signed radian angle between 2 vectors, around a normal. vectors must be normalized.
float util::vectorAngleAroundNormal(const glm::vec3& vec1, const glm::vec3& vec2, const glm::vec3& norm)
{
  float dot = vec1.x*vec2.x + vec1.y*vec2.y + vec1.z*vec2.z;
  float det = vec1.x*vec2.y*norm.z + vec2.x*norm.y*vec1.z + norm.x*vec1.y*vec2.z - vec1.z*vec2.y*norm.x - vec2.z*norm.y*vec1.x - norm.z*vec1.y*vec2.x;
  return atan2(det, dot);
}

glm::mat4 util::rotationAroundPoint(glm::vec3 point, glm::mat4 rotationMatrix)
{
  glm::mat4 translateToPoint = glm::translate(glm::mat4(), 
                                                 glm::vec3(point.x, point.y, point.z)
                                                 );
  glm::mat4 inverseTranslate = glm::inverse( translateToPoint );
  return translateToPoint * rotationMatrix * inverseTranslate;
}

void util::quaternionToAngleAxis(const glm::quat& quat, float& angle, glm::vec3& axis)
{
  angle = 2 * acos(quat.w);

  axis.x = quat.x / sqrt(1-quat.w*quat.w);
  axis.y = quat.y / sqrt(1-quat.w*quat.w);
  axis.z = quat.z / sqrt(1-quat.w*quat.w);
}

glm::quat util::quaternionFromAngleAxis(const float& angle, const glm::vec3& axis)
{
  /*glm::quat quat = glm::quat_identity;

  quat.x = axis.x * sin(angle/2);
  quat.y = axis.y * sin(angle/2);
  quat.z = axis.z * sin(angle/2);
  quat.w = cos(angle/2);

  return quat;*/

  return glm::angleAxis(angle, axis);
}