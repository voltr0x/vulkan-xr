#pragma once
#include "Context.h"
#include "Input.h"
#include "InputData.h"
#include "Headset.h"
#include "MeshData.h"
#include "MirrorView.h"
#include "GameData.h"
#include "Renderer.h"
#include "gameMechanics/GameBehaviour.h"
#include "gameMechanics/HandsBehaviour.h"
#include "gameMechanics/InputTesterBehaviour.h"
#include "gameMechanics/WorldObjectsMiscBehaviour.h"
#include "gameMechanics/LocomotionBehaviour.h"

#include <chrono>

#include <stdio.h>

namespace
{
constexpr float flySpeedMultiplier = 2.5f;
}

int main()
{
  glm::mat4 cameraMatrix = glm::mat4(1.0f); // Transform from world to stage space

  Context context;
  if (!context.isValid())
  {
    return EXIT_FAILURE;
  }

  MirrorView mirrorView(&context);
  if (!mirrorView.isValid())
  {
    return EXIT_FAILURE;
  }

  if (!context.createDevice(mirrorView.getSurface()))
  {
    return EXIT_FAILURE;
  }

  Headset headset(&context);
  if (!headset.isValid())
  {
    return EXIT_FAILURE;
  }

  
  Inputspace::Input inputSystem(context.getXrInstance(), headset.getXrSession());
  if (!inputSystem.isValid())
  {
    return EXIT_FAILURE;
  }

  Model gridModel, ruinsModel, carModelLeft, carModelRight, beetleModel, bikeModel, handModelLeft, handModelRight,
    logoModel = {};
  std::vector<Model*> models = { &gridModel, &ruinsModel,    &carModelLeft,   &carModelRight, &beetleModel,
                                 &bikeModel, &handModelLeft, &handModelRight, &logoModel };
  
  Material gridMaterial, diffuseMaterial, bikeMaterial, logoMaterial, locomotionMaterial, skyMaterial = {};
  gridMaterial.vertShaderName = "shaders/Grid.vert.spv";
  gridMaterial.fragShaderName = "shaders/Grid.frag.spv";
  gridMaterial.dynamicUniformData.colorMultiplier = glm::vec4(1.0f);
  diffuseMaterial.vertShaderName = "shaders/Diffuse.vert.spv";
  diffuseMaterial.fragShaderName = "shaders/Diffuse.frag.spv";
  diffuseMaterial.dynamicUniformData.colorMultiplier = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  bikeMaterial.vertShaderName = "shaders/DiffuseTransparent.vert.spv";
  bikeMaterial.fragShaderName = "shaders/DiffuseTransparent.frag.spv";
  bikeMaterial.pipelineData.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;
  bikeMaterial.dynamicUniformData.colorMultiplier = glm::vec4(1.0f, 0.0f, 0.1f, 0.66f);
  logoMaterial.vertShaderName = "shaders/Diffuse.vert.spv";
  logoMaterial.fragShaderName = "shaders/Diffuse.frag.spv";
  logoMaterial.dynamicUniformData.colorMultiplier = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  logoMaterial.pipelineData.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;
  std::vector<Material*> materials = { &gridMaterial, &diffuseMaterial, &bikeMaterial, &logoMaterial, &locomotionMaterial, &skyMaterial};
  
  GameObject head = GameObject();
  head.worldMatrix = glm::inverse(cameraMatrix);
  GameObject handLeft = GameObject(&handModelLeft, &logoMaterial, true, "handLeft");
  GameObject handRight = GameObject(&handModelRight, &logoMaterial, true, "handRight");
  GameObject grid = GameObject(&gridModel, &gridMaterial, true, "grid");
  GameObject ruins = GameObject(&ruinsModel, &diffuseMaterial, true, "ruins");
  GameObject carLeft = GameObject(&carModelLeft, &diffuseMaterial, true, "carLeft");
  GameObject carRight = GameObject(&carModelRight, &diffuseMaterial, true, "handRight");
  GameObject beetle = GameObject(&beetleModel, &diffuseMaterial, true, "beetle");
  GameObject bike = GameObject(&bikeModel, &bikeMaterial, true, "bike");
  GameObject logo = GameObject(&logoModel, &logoMaterial, true, "logo");
  std::vector<GameObject*> gameObjects = { &grid, &ruins, &carLeft, &carRight, &beetle, &bike, &handLeft, &handRight, &logo };
  PlayerObject playerObject = PlayerObject("XR Player 1", &head, &handLeft, &handRight);

  carLeft.worldMatrix =
    glm::rotate(glm::translate(glm::mat4(1.0f), { -3.5f, 0.0f, -7.0f }), glm::radians(75.0f), { 0.0f, 1.0f, 0.0f });
  carRight.worldMatrix =
    glm::rotate(glm::translate(glm::mat4(1.0f), { 8.0f, 0.0f, -15.0f }), glm::radians(-15.0f), { 0.0f, 1.0f, 0.0f });
  beetle.worldMatrix =
    glm::rotate(glm::translate(glm::mat4(1.0f), { -3.5f, 0.0f, -0.5f }), glm::radians(-125.0f), { 0.0f, 1.0f, 0.0f });
  logo.worldMatrix = glm::translate(glm::mat4(1.0f), { 0.0f, 3.0f, -10.0f });
  bike.worldMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), { 0.5f, 0.0f, -4.5f }), 0.2f, { 0.0f, 1.0f, 0.0f });

  MeshData* meshData = new MeshData;
  if (!meshData->loadModel("models/Grid.obj", MeshData::Color::FromNormals, models, 0u, 1u)) {
    return EXIT_FAILURE;
  }

  if (!meshData->loadModel("models/Ruins.obj", MeshData::Color::White, models, 1u, 1u)) {
    return EXIT_FAILURE;
  }

  if (!meshData->loadModel("models/Car.obj", MeshData::Color::White, models, 2u, 2u)) {
    return EXIT_FAILURE;
  }

  if (!meshData->loadModel("models/Beetle.obj", MeshData::Color::White, models, 4u, 1u)) {
    return EXIT_FAILURE;
  }

  if (!meshData->loadModel("models/Bike.obj", MeshData::Color::White, models, 5u, 1u)) {
    return EXIT_FAILURE;
  }

  if (!meshData->loadModel("models/Hand.obj", MeshData::Color::White, models, 6u, 2u)) {
    return EXIT_FAILURE;
  }

  if (!meshData->loadModel("models/Logo.obj", MeshData::Color::White, models, 8u, 1u)) {
    return EXIT_FAILURE;
  }

  Renderer renderer(&context, &headset, meshData, materials, gameObjects);
  if (!renderer.isValid())
  {
    return EXIT_FAILURE;
  }

  delete meshData;

  if (!mirrorView.connect(&headset, &renderer))
  {
    return EXIT_FAILURE;
  }

  std::vector<GameBehaviour*> gameBehaviours = {
    new LocomotionBehaviour(playerObject, 1, 3, 1),
    new HandsBehaviour(playerObject),
    new WorldObjectsMiscBehaviour(bike, logoMaterial)
  };
    
  static float gameTime = 0.0f;
  
  // Main loop
  std::chrono::high_resolution_clock::time_point previousTime = std::chrono::high_resolution_clock::now();
  while (!headset.isExitRequested() && !mirrorView.isExitRequested())
  {
    // Calculate the delta time in seconds
    const std::chrono::high_resolution_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
    const long long elapsedNanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - previousTime).count();
    const float deltaTime = static_cast<float>(elapsedNanoseconds) / 1e9f;
    previousTime = nowTime;

    mirrorView.processWindowEvents();
    
    uint32_t swapchainImageIndex;
    const Headset::BeginFrameResult frameResult = headset.beginFrame(swapchainImageIndex);
    if (frameResult == Headset::BeginFrameResult::Error)
    {
      return EXIT_FAILURE;
    }
    else if (frameResult == Headset::BeginFrameResult::RenderFully)
    {
      
      if (!inputSystem.Sync(headset.getXrSpace(), headset.getXrFrameState().predictedDisplayTime, 
                            headset.getEyePoses(), headset.getSessionState()))
      {
        return EXIT_FAILURE;
      }
      const Inputspace::InputData& inputData = inputSystem.GetInputData();
      Inputspace::InputHaptics& inputHaptics = inputSystem.GetInputHaptics();
      
      gameTime += deltaTime;

      for(size_t i = 0; i < gameBehaviours.size(); i++){
        gameBehaviours[i]->Update(deltaTime, gameTime, inputData, inputHaptics);
      }
      inputSystem.ApplyHapticFeedbackRequests(inputHaptics);

      // Render
      renderer.render(glm::inverse(head.worldMatrix), swapchainImageIndex, gameTime);

      const MirrorView::RenderResult mirrorResult = mirrorView.render(swapchainImageIndex);
      if (mirrorResult == MirrorView::RenderResult::Error)
      {
        return EXIT_FAILURE;
      }

      const bool mirrorViewVisible = (mirrorResult == MirrorView::RenderResult::Visible);
      renderer.submit(mirrorViewVisible);

      if (mirrorViewVisible)
      {
        mirrorView.present();
      }
    }

    if (frameResult == Headset::BeginFrameResult::RenderFully || frameResult == Headset::BeginFrameResult::SkipRender)
    {
      headset.endFrame();
    }
  }

  for(size_t i=0; i<gameBehaviours.size(); i++){
    delete(gameBehaviours[i]);
  }
  
  // Sync before destroying so that resources are free
  context.sync(); 
  return EXIT_SUCCESS;
}