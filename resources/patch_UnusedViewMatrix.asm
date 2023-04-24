[BetterVR_UpdateViewMatrix_V208]
moduleMatches = 0x6267BFD0

.origin = codecave

doUpdateMatrix_1:
stwu r1,-16(r1)
stw r31,12(r1)
mr r31,r1
lfs f10,148(r3)
fadds f10,f10,f10
lfs f0,168(r3)
fmuls f0,f10,f0
lfs f12,172(r3)
fmuls f12,f0,f12
lfs f7,176(r3)
fmuls f7,f12,f7
lfs f6,180(r3)
fmuls f6,f0,f6
lis r9,const_05@ha
lfs f11,const_05@l(r9)
fmuls f0,f0,f11
fmuls f12,f12,f11
fadds f5,f0,f6
fsubs f9,f6,f0
fsubs f8,f7,f12
fadds f12,f12,f7
lis r9,const_004@ha
lfs f11,const_004@l(r9)
fcmpu cr7,f0,f11
BNG cr7, _cc03cc03cc03cc03_L2
lis r9,const_005@ha
lfs f11,const_005@l(r9)
fcmpu cr7,f0,f11
BNL cr7, _cc03cc03cc03cc03_L2
lis r9,newFOV_up@ha
lfs f9,newFOV_up@l(r9)
fadds f5,f9,f6
lis r9,newFOV_down@ha
lfs f0,newFOV_down@l(r9)
fadds f9,f0,f6
lis r9,newFOV_left@ha
lfs f8,newFOV_left@l(r9)
fadds f8,f8,f7
lis r9,newFOV_right@ha
lfs f12,newFOV_right@l(r9)
fadds f12,f12,f7
_cc03cc03cc03cc03_L2:
fsubs f11,f12,f8
lis r9,const_1@ha
lfs f0,const_1@l(r9)
fdivs f11,f0,f11
fmuls f10,f10,f11
stfs f10,0(r4)
li r9,0
stw r9,4(r4)
fadds f8,f12,f8
fmuls f11,f8,f11
stfs f11,8(r4)
stw r9,12(r4)
fsubs f12,f5,f9
fdivs f12,f0,f12
stw r9,16(r4)
lfs f11,148(r3)
fadds f11,f11,f11
fmuls f11,f11,f12
stfs f11,20(r4)
fadds f9,f5,f9
fmuls f12,f9,f12
stfs f12,24(r4)
stw r9,28(r4)
lfs f12,152(r3)
lfs f11,148(r3)
fsubs f12,f12,f11
fdivs f0,f0,f12
stw r9,32(r4)
stw r9,36(r4)
lfs f11,152(r3)
lfs f12,148(r3)
fadds f12,f11,f12
fneg f12,f12
fmuls f12,f12,f0
stfs f12,40(r4)
lfs f12,152(r3)
fadds f11,f12,f12
lfs f12,148(r3)
fmuls f12,f11,f12
fneg f12,f12
fmuls f0,f12,f0
stfs f0,44(r4)
stw r9,48(r4)
stw r9,52(r4)
lis r10,const_neg_1@ha
lfs f0,const_neg_1@l(r10)
stfs f0,56(r4)
stw r9,60(r4)
addi r11,r31,16
lwz r31,-4(r11)
mr r1,r11
blr
const_05:
.float 0.5
const_005:
.float 0.05
const_004:
.float 0.04
const_1:
.float 1.0
const_neg_1:
.float -1.0


doUpdateMatrix_2:
	stwu r1,-16(r1)
	stw r31,12(r1)
	mr r31,r1
	lfs f9,148(r3)
	fadds f9,f9,f9
	lfs f0,168(r3)
	fmuls f0,f9,f0
	lfs f11,172(r3)
	fmuls f11,f0,f11
	lfs f10,176(r3)
	fmuls f10,f11,f10
	lfs f12,180(r3)
	fmuls f12,f0,f12
	lis r9,_cc03cc03cc03cc03_LC0@ha
	lfs f8,_cc03cc03cc03cc03_LC0@l(r9)
	fmuls f0,f0,f8 ; clip_height *= 0.5f = 0.466308f
	fmuls f11,f11,f8 ; clip_height *= 0.5f = 0.828991f
	fadds f7,f0,f12 ; up
	fsubs f0,f12,f0 ; down
	fsubs f8,f10,f11 ; left
	fadds f11,f11,f10 ; right
	fsubs f10,f11,f8 ; left - right
	lis r9,_cc03cc03cc03cc03_LC1@ha
	lfs f12,_cc03cc03cc03cc03_LC1@l(r9)
	fdivs f10,f12,f10
	fmuls f9,f9,f10
	stfs f9,0(r4)
	li r9,0
	stw r9,4(r4)
	fadds f11,f11,f8
	fmuls f10,f11,f10
	stfs f10,8(r4)
	stw r9,12(r4)
	fsubs f11,f7,f0
	fdivs f11,f12,f11
	stw r9,16(r4)
	lfs f10,148(r3)
	fadds f10,f10,f10
	fmuls f10,f10,f11
	stfs f10,20(r4)
	fadds f0,f7,f0
	fmuls f0,f0,f11
	stfs f0,24(r4)
	stw r9,28(r4)
	lfs f0,152(r3)
	lfs f11,148(r3)
	fsubs f0,f0,f11
	fdivs f12,f12,f0
	stw r9,32(r4)
	stw r9,36(r4)
	lfs f11,152(r3)
	lfs f0,148(r3)
	fadds f0,f11,f0
	fneg f0,f0
	fmuls f0,f0,f12
	stfs f0,40(r4)
	lfs f0,152(r3)
	fadds f11,f0,f0
	lfs f0,148(r3)
	fmuls f0,f11,f0
	fneg f0,f0
	fmuls f12,f0,f12
	stfs f12,44(r4)
	stw r9,48(r4)
	stw r9,52(r4)
	lis r10,_cc03cc03cc03cc03_LC2@ha
	lfs f0,_cc03cc03cc03cc03_LC2@l(r10)
	stfs f0,56(r4)
	stw r9,60(r4)
	addi r11,r31,16
	lwz r31,-4(r11)
	mr r1,r11
	blr

_cc03cc03cc03cc03_LC0:
	.int	1056964608
_cc03cc03cc03cc03_LC1:
	.int	1065353216
_cc03cc03cc03cc03_LC2:
	.int	-1082130432

#0x030C1990 = ba doUpdateMatrix_1