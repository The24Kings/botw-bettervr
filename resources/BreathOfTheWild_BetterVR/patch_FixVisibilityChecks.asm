[BetterVR_FirstPersonMode_V208]
moduleMatches = 0x6267BFD0

.origin = codecave

custom_checkIfCameraCanSeePos:
mflr r0
stw r0, 0x04(r1)
bla import.coreinit.hook_CheckIfCameraCanSeePos
lwz r0, 0x04(r1)
mtlr r0
blr

;0x0318FFA8 = ba custom_checkIfCameraCanSeePos

; always make AttCheck::ScreenRelated check return true
;0x035D67BC = li r3, 1