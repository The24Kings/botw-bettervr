[BetterVR_UpdateProjectionMatrix_V208]
moduleMatches = 0x6267BFD0

.origin = codecave


; Custom sead::PerspectiveProjection::PerspectiveProjection() initializer
initCustomProjectionMatrix:
lis r12, data_projectionMatrix@ha
addi r12, r12, data_projectionMatrix@l
mflr r11
bl import.coreinit.hook_UpdateProjectionMatrix
mtlr r11

lis r11, aspectRatio@ha
lfs f0, aspectRatio@l(r11)
stfs f0, 0xAC(r31)

lis r11, offsetX@ha
lfs f1, offsetX@l(r11)
stfs f1, 0xB0(r31)

lis r11, offsetY@ha
lfs f1, offsetY@l(r11)
stfs f1, 0xB4(r31)

lis r12, fovY@ha
lfs f1, fovY@l(r12)
mr r3, r31
blr

; skip existing FOV setting code to allow returning to LR
; 0x030C17B0 = b skipExistingFOVInit_1
; 0x030C17C8 = skipExistingFOVInit_1:
; 0x030C17C8 = bla initCustomProjectionMatrix
;
; 0x030C1878 = b skipExistingFOVInit_2
; 0x030C1890 = skipExistingFOVInit_2:
; 0x030C1890 = bla initCustomProjectionMatrix


; ----------------------------
overwriteSetAspectRatio:
lis r30, aspectRatio@ha
lfs f31, aspectRatio@l(r30)
stfs f31, 0xAC(r31)
blr

; 0x030C1934 = bla overwriteSetAspectRatio

; ----------------------------
overwriteSetFovY:
lis r12, fovY@ha
lfs f1, fovY@l(r12)
blr

; 0x030C16F8 = bla overwriteSetFovY