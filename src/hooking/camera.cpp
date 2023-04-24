#include "cemu_hooks.h"
#include "rendering/openxr.h"
#include "instance.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/constants.hpp>

OpenXR::EyeSide CemuHooks::s_eyeSide = OpenXR::EyeSide::LEFT;

void CemuHooks::hook_UpdateProjectionMatrix(PPCInterpreter_t* hCPU) {
    hCPU->instructionPointer = hCPU->sprNew.LR;

    XrView currView = VRManager::instance().XR->GetPredictedView(s_eyeSide);
    checkAssert(currView.fov.angleLeft <= currView.fov.angleRight, "OpenXR gave a left FOV that is larger than the right FOV! Behavior is unexpected!");
    checkAssert(currView.fov.angleDown <= currView.fov.angleUp, "OpenXR gave a top FOV that is larger than the bottom FOV! Behavior is unexpected!");

    // Convert the 4 OpenXR FOV values into a FoV, aspect ratio and projection center offset
    float totalHorizontalFov = currView.fov.angleRight - currView.fov.angleLeft;
    float totalVerticalFov = currView.fov.angleUp - currView.fov.angleDown;

    float aspectRatio = totalHorizontalFov / totalVerticalFov;
    float fovY = totalVerticalFov;
    float projectionCenter_offsetX = (currView.fov.angleRight + currView.fov.angleLeft) / 2.0f;
    float projectionCenter_offsetY = (currView.fov.angleUp + currView.fov.angleDown) / 2.0f;

    data_VRProjectionMatrixOut projectionMatrixOut = {
        .aspectRatio = aspectRatio,
        .fovY = fovY,
        .offsetX = projectionCenter_offsetX,
        .offsetY = projectionCenter_offsetY,
    };
    //Log::print("Updated camera projection matrix: FOV = {}, aspectRatio = {}, offsetX = {}, offsetY = {}!", projectionMatrixOut.fovY, projectionMatrixOut.aspectRatio, projectionMatrixOut.offsetX, projectionMatrixOut.offsetY);
    swapEndianness(projectionMatrixOut.aspectRatio);
    swapEndianness(projectionMatrixOut.fovY);
    swapEndianness(projectionMatrixOut.offsetX);
    swapEndianness(projectionMatrixOut.offsetY);
    uint32_t ppc_projectionMatrixOut = hCPU->gpr[28];
    writeMemory(ppc_projectionMatrixOut, &projectionMatrixOut);

}

void CemuHooks::hook_UpdateCamera(PPCInterpreter_t* hCPU) {
    //Log::print("Updated camera!");
    hCPU->instructionPointer = hCPU->gpr[7];

    // Read the camera matrix from the game's memory
    uint32_t ppc_cameraMatrixOffsetIn = hCPU->gpr[30];
    data_VRCameraIn origCameraMatrix = {};

    readMemory(ppc_cameraMatrixOffsetIn, &origCameraMatrix);
    swapEndianness(origCameraMatrix.posX);
    swapEndianness(origCameraMatrix.posY);
    swapEndianness(origCameraMatrix.posZ);
    swapEndianness(origCameraMatrix.targetX);
    swapEndianness(origCameraMatrix.targetY);
    swapEndianness(origCameraMatrix.targetZ);
    swapEndianness(origCameraMatrix.fov);

    data_VRSettingsIn settings = VRManager::instance().Hooks->GetSettings();

    // Current VR headset camera matrix
    // fixme: Update poses close to usage aka here: VRManager::instance().XR->UpdatePoses(m_eyeSide);
    XrView currView = VRManager::instance().XR->GetPredictedView(s_eyeSide);
    glm::fvec3 currEyePos(currView.pose.position.x, currView.pose.position.y, currView.pose.position.z);
    glm::fquat currEyeQuat(currView.pose.orientation.w, currView.pose.orientation.x, currView.pose.orientation.y, currView.pose.orientation.z);
    //Log::print("Headset View: x={}, y={}, z={}, orientW={}, orientX={}, orientY={}, orientZ={}", currEyePos.x, currEyePos.y, currEyePos.z, currEyeQuat.w, currEyeQuat.x, currEyeQuat.y, currEyeQuat.z);

    // Current in-game camera matrix
    glm::fvec3 oldCameraPosition(origCameraMatrix.posX, origCameraMatrix.posY, origCameraMatrix.posZ);
    glm::fvec3 oldCameraTarget(origCameraMatrix.targetX, origCameraMatrix.targetY, origCameraMatrix.targetZ);
    float oldCameraDistance = glm::distance(oldCameraPosition, oldCameraTarget);
    //Log::print("Original Game Camera: x={}, y={}, z={}, targetX={}, targetY={}, targetZ={}", oldCameraPosition.x, oldCameraPosition.y, oldCameraPosition.z, oldCameraTarget.x, oldCameraTarget.y, oldCameraTarget.z);

    // Calculate game view directions
    glm::fvec3 forwardVector = glm::normalize(oldCameraTarget - oldCameraPosition);
    glm::fquat lookAtQuat = glm::quatLookAtRH(forwardVector, {0.0, 1.0, 0.0});

    // Calculate new view direction
    glm::fquat combinedQuat = glm::normalize(lookAtQuat * currEyeQuat);
    glm::fmat3 combinedMatrix = glm::toMat3(combinedQuat);

    // Calculate the camera rotation
    glm::fvec3 rotatedHmdPos = combinedQuat * currEyePos;

    // Convert the calculated parameters into the new camera matrix provided by the game
    data_VRCameraOut updatedCameraMatrix = {
        .posX = oldCameraPosition.x + rotatedHmdPos.x,
        .posY = oldCameraPosition.y + rotatedHmdPos.y,
        .posZ = oldCameraPosition.z + rotatedHmdPos.z,
        .targetX = oldCameraPosition.x + ((combinedMatrix[2][0] * -1.0f) * oldCameraDistance) + rotatedHmdPos.x,
        .targetY = oldCameraPosition.y + ((combinedMatrix[2][1] * -1.0f) * oldCameraDistance) + rotatedHmdPos.y,
        .targetZ = oldCameraPosition.z + ((combinedMatrix[2][2] * -1.0f) * oldCameraDistance) + rotatedHmdPos.z,
        .rotX = combinedMatrix[1][0],
        .rotY = combinedMatrix[1][1],
        .rotZ = combinedMatrix[1][2],
        .fov = (currView.fov.angleUp - currView.fov.angleDown),
    };

    // Write the camera matrix to the game's memory
    //Log::print("New Game Camera: x={}, y={}, z={}, targetX={}, targetY={}, targetZ={}, rotX={}, rotY={}, rotZ={}", updatedCameraMatrix.posX, updatedCameraMatrix.posY, updatedCameraMatrix.posZ, updatedCameraMatrix.targetX, updatedCameraMatrix.targetY, updatedCameraMatrix.targetZ, updatedCameraMatrix.rotX, updatedCameraMatrix.rotY, updatedCameraMatrix.rotZ);
    swapEndianness(updatedCameraMatrix.posX);
    swapEndianness(updatedCameraMatrix.posY);
    swapEndianness(updatedCameraMatrix.posZ);
    swapEndianness(updatedCameraMatrix.targetX);
    swapEndianness(updatedCameraMatrix.targetY);
    swapEndianness(updatedCameraMatrix.targetZ);
    swapEndianness(updatedCameraMatrix.rotX);
    swapEndianness(updatedCameraMatrix.rotY);
    swapEndianness(updatedCameraMatrix.rotZ);
    swapEndianness(updatedCameraMatrix.fov);
    uint32_t ppc_cameraMatrixOffsetOut = hCPU->gpr[31];
    writeMemory(ppc_cameraMatrixOffsetOut, &updatedCameraMatrix);
}