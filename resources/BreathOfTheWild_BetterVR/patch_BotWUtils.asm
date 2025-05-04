[BetterVR_Utils_V208]
moduleMatches = 0x6267BFD0

.origin = codecave

; String Comparison Function (r3 = string1, r4 = string2, sets comparison register so use beq for true, bne for false)
_compareString:
stwu r1, -0x20(r1)
mflr r0
stw r0, 0x04(r1)
stw r3, 0x0C(r1)
stw r4, 0x10(r1)
stw r5, 0x14(r1)
stw r6, 0x18(r1)


startLoop:
lbz r5, 0(r3)
lbz r6, 0(r4)

cmpwi r5, 0
bne checkForMatch
cmpwi r6, 0
bne checkForMatch
b foundMatch

checkForMatch:
cmpw r5, r6
bne noMatch
addi r3, r3, 1
addi r4, r4, 1
b startLoop

noMatch:
; this sets the comparison register to 0 aka false
li r5, 0
cmpwi r5, 1337
b end

foundMatch:
li r5, 1337
cmpwi r5, 1337
b end

end:
lwz r3, 0x0C(r1)
lwz r4, 0x10(r1)
lwz r5, 0x14(r1)
lwz r6, 0x18(r1)
lwz r0, 0x04(r1)
mtlr r0
addi r1, r1, 0x20
blr


; Log to OSReport using format string
loadLineCharacter:
.int 10
.align 4
; r0 should be modifiable
; r3 = format string
; r4 = int arg1
; r5 = int arg2
; f1 = float arg1
; f2 = float arg2
printToLog:
mflr r0
stwu r1, -0x40(r1)
stw r0, 0x14(r1)
stw r5, 0x8(r1)
stw r6, 0xC(r1)

lis r6, loadLineCharacter@ha
lwz r6, loadLineCharacter@l(r6)
crxor 4*cr1+eq, 4*cr1+eq, 4*cr1+eq
bl import.coreinit.OSReport

lwz r6, 0xC(r1)
lwz r5, 0x8(r1)
lwz r0, 0x14(r1)
mtlr r0
addi r1, r1, 0x40 ; this was set to 0x10 before, but that makes no sense?
blr










0x397A9CC = jumpBranchStart:
0x1047EC78 = agl__lyr__Renderer__sInstance:

drawInstanceIdx:
.int 1

newLineFormat:
.string "Rendering layer {}..."
.align 4

registerR0:
.int 0
registerR3:
.int 0
registerR4:
.int 0
registerR5:
.int 0
registerR6:
.int 0
registerR7:
.int 0
registerR8:
.int 0

selectiveDrawRendering:
lwz r11, 0(r30)

; === Print stuff ===
lis r12, registerR3@ha
stw r3, registerR3@l(r12)
lis r12, registerR4@ha
stw r4, registerR4@l(r12)
lis r12, registerR5@ha
stw r5, registerR5@l(r12)
lis r12, registerR6@ha
stw r6, registerR6@l(r12)
lis r12, registerR7@ha
stw r7, registerR7@l(r12)
lis r12, registerR8@ha
stw r8, registerR8@l(r12)

lis r3, newLineFormat@ha
addi r3, r3, newLineFormat@l
mr r4, r11
mflr r5
bl import.coreinit.hook_OSReportToConsole2
mtlr r5

lis r12, registerR3@ha
lwz r3, registerR3@l(r12)
lis r12, registerR4@ha
lwz r4, registerR4@l(r12)
lis r12, registerR5@ha
lwz r5, registerR5@l(r12)
lis r12, registerR6@ha
lwz r6, registerR6@l(r12)
lis r12, registerR7@ha
lwz r7, registerR7@l(r12)
lis r12, registerR8@ha
lwz r8, registerR8@l(r12)
; === Print stuff ===

; r11 holds current draw rendering step number
cmpwi r11, 10
bne checkIfSkip

endOfFrame:
lis r12, drawInstanceIdx@ha
lwz r12, drawInstanceIdx@l(r12)
cmpwi r12, 1
beq endOfFrameReset

endOfFrameIterate:
li r0, 1
b dontSkip ; b checkIfSkip

endOfFrameReset:
li r0, 0
b doSkips


checkIfSkip:
lis r12, drawInstanceIdx@ha
lwz r12, drawInstanceIdx@l(r12)
cmpwi r12, 1
beq dontSkip

doSkips:
# cmpwi r11, 4
# bne .+0x08
# li r11, 3
# cmpwi r11, 4
# bne .+0x08
# li r12, 3
# cmpwi r11, 5
# bne .+0x08
# li r12, 3


dontSkip:
lis r12, agl__lyr__Renderer__sInstance@ha
lwz r12, agl__lyr__Renderer__sInstance@l(r12)
blr

0x0397A9A0 = bla selectiveDrawRendering