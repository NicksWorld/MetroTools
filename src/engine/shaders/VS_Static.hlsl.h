#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyz         0     NONE   float   xyz 
// NORMAL                   0   xyzw        1     NONE   float   xyzw
// TANGENT                  0   xyzw        2     NONE   float   xyzw
// COLOR                    0   xyzw        3     NONE   float       
// TEXCOORD                 0   xy          4     NONE   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xyzw
// TEXCOORD                 0   xyzw        1     NONE   float   xyzw
// TEXCOORD                 1   xyzw        2     NONE   float   xyzw
// TEXCOORD                 2   xyzw        3     NONE   float   xyzw
// TEXCOORD                 3   xyzw        4     NONE   float   xyzw
//
vs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[23], immediateIndexed
dcl_input v0.xyz
dcl_input v1.xyzw
dcl_input v2.xyzw
dcl_input v4.xy
dcl_output_siv o0.xyzw, position
dcl_output o1.xyzw
dcl_output o2.xyzw
dcl_output o3.xyzw
dcl_output o4.xyzw
dcl_temps 3
mul r0.xyzw, v0.yyyy, cb0[17].xyzw
mad r0.xyzw, v0.zzzz, cb0[16].xyzw, r0.xyzw
mad r0.xyzw, v0.xxxx, cb0[18].xyzw, r0.xyzw
add o0.xyzw, r0.xyzw, cb0[19].xyzw
mad r0.xyz, v1.xzyx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
mul r1.xyz, r0.zzzz, cb0[21].xyzx
mad r1.xyz, r0.yyyy, cb0[20].xyzx, r1.xyzx
mad r1.xyz, r0.xxxx, cb0[22].xyzx, r1.xyzx
dp3 r0.w, r1.xyzx, r1.xyzx
rsq r0.w, r0.w
mul o1.xyz, r0.wwww, r1.xyzx
mov o1.w, v1.w
mad r1.xyzw, v2.xyzw, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
mul r2.xyz, r1.yyyy, cb0[21].xyzx
mad r2.xyz, r1.zzzz, cb0[20].xyzx, r2.xyzx
mad r2.xyz, r1.xxxx, cb0[22].xyzx, r2.xyzx
dp3 r0.w, r2.xyzx, r2.xyzx
rsq r0.w, r0.w
mul o2.xyz, r0.wwww, r2.xyzx
mov o2.w, l(0)
mul r2.xyz, r0.xyzx, r1.yxzy
mad r0.xyz, r0.zxyz, r1.xzyx, -r2.xyzx
mul r0.xyz, r1.wwww, r0.xyzx
mul r1.xyz, r0.yyyy, cb0[21].xyzx
mad r0.xyw, r0.xxxx, cb0[20].xyxz, r1.xyxz
mad r0.xyz, r0.zzzz, cb0[22].xyzx, r0.xywx
dp3 r0.w, r0.xyzx, r0.xyzx
rsq r0.w, r0.w
mul o3.xyz, r0.wwww, r0.xyzx
mov o3.w, l(0)
mov o4.xy, v4.xyxx
mov o4.zw, l(0,0,0,0)
ret 
// Approximately 0 instruction slots used
#endif

const BYTE sVS_StaticData[] =
{
     68,  88,  66,  67, 100, 236, 
    253, 230, 107,  55,  35,  14, 
      9, 225,   8, 160,  96, 153, 
    241,  31,   1,   0,   0,   0, 
     40,   6,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
    220,   0,   0,   0, 124,   1, 
      0,   0,  73,  83,  71,  78, 
    168,   0,   0,   0,   5,   0, 
      0,   0,   8,   0,   0,   0, 
    128,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   7,   7,   0,   0, 
    137,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,  15,  15,   0,   0, 
    144,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   2,   0, 
      0,   0,  15,  15,   0,   0, 
    152,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   3,   0, 
      0,   0,  15,   0,   0,   0, 
    158,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   4,   0, 
      0,   0,   3,   3,   0,   0, 
     80,  79,  83,  73,  84,  73, 
     79,  78,   0,  78,  79,  82, 
     77,  65,  76,   0,  84,  65, 
     78,  71,  69,  78,  84,   0, 
     67,  79,  76,  79,  82,   0, 
     84,  69,  88,  67,  79,  79, 
     82,  68,   0, 171,  79,  83, 
     71,  78, 152,   0,   0,   0, 
      5,   0,   0,   0,   8,   0, 
      0,   0, 128,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0, 140,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,  15,   0, 
      0,   0, 140,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      2,   0,   0,   0,  15,   0, 
      0,   0, 140,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      3,   0,   0,   0,  15,   0, 
      0,   0, 140,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      4,   0,   0,   0,  15,   0, 
      0,   0,  83,  86,  95,  80, 
    111, 115, 105, 116, 105, 111, 
    110,   0,  84,  69,  88,  67, 
     79,  79,  82,  68,   0, 171, 
    171, 171,  83,  72,  69,  88, 
    164,   4,   0,   0,  80,   0, 
      1,   0,  41,   1,   0,   0, 
    106,   8,   0,   1,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,  23,   0, 
      0,   0,  95,   0,   0,   3, 
    114,  16,  16,   0,   0,   0, 
      0,   0,  95,   0,   0,   3, 
    242,  16,  16,   0,   1,   0, 
      0,   0,  95,   0,   0,   3, 
    242,  16,  16,   0,   2,   0, 
      0,   0,  95,   0,   0,   3, 
     50,  16,  16,   0,   4,   0, 
      0,   0, 103,   0,   0,   4, 
    242,  32,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   2,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   3,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   4,   0,   0,   0, 
    104,   0,   0,   2,   3,   0, 
      0,   0,  56,   0,   0,   8, 
    242,   0,  16,   0,   0,   0, 
      0,   0,  86,  21,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     17,   0,   0,   0,  50,   0, 
      0,  10, 242,   0,  16,   0, 
      0,   0,   0,   0, 166,  26, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,  16,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  50,   0,   0,  10, 
    242,   0,  16,   0,   0,   0, 
      0,   0,   6,  16,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     18,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   8, 242,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,  19,   0, 
      0,   0,  50,   0,   0,  15, 
    114,   0,  16,   0,   0,   0, 
      0,   0, 134,  17,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,  64, 
      0,   0,   0,  64,   0,   0, 
      0,  64,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128, 191,   0,   0, 128, 191, 
      0,   0, 128, 191,   0,   0, 
      0,   0,  56,   0,   0,   8, 
    114,   0,  16,   0,   1,   0, 
      0,   0, 166,  10,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
     21,   0,   0,   0,  50,   0, 
      0,  10, 114,   0,  16,   0, 
      1,   0,   0,   0,  86,   5, 
     16,   0,   0,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,  20,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    114,   0,  16,   0,   1,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
     22,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     16,   0,   0,   7, 130,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  68,   0, 
      0,   5, 130,   0,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   7, 114,  32, 
     16,   0,   1,   0,   0,   0, 
    246,  15,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   5, 130,  32,  16,   0, 
      1,   0,   0,   0,  58,  16, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  15, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  30,  16,   0,   2,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  64,   0,   0, 
      0,  64,   0,   0,   0,  64, 
      0,   0,   0,  64,   2,  64, 
      0,   0,   0,   0, 128, 191, 
      0,   0, 128, 191,   0,   0, 
    128, 191,   0,   0, 128, 191, 
     56,   0,   0,   8, 114,   0, 
     16,   0,   2,   0,   0,   0, 
     86,   5,  16,   0,   1,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,  21,   0, 
      0,   0,  50,   0,   0,  10, 
    114,   0,  16,   0,   2,   0, 
      0,   0, 166,  10,  16,   0, 
      1,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
     20,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     50,   0,   0,  10, 114,   0, 
     16,   0,   2,   0,   0,   0, 
      6,   0,  16,   0,   1,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,  22,   0, 
      0,   0,  70,   2,  16,   0, 
      2,   0,   0,   0,  16,   0, 
      0,   7, 130,   0,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     70,   2,  16,   0,   2,   0, 
      0,   0,  68,   0,   0,   5, 
    130,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   7, 114,  32,  16,   0, 
      2,   0,   0,   0, 246,  15, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   2,   0, 
      0,   0,  54,   0,   0,   5, 
    130,  32,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   7, 114,   0,  16,   0, 
      2,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     22,   6,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  38,   9,  16,   0, 
      0,   0,   0,   0, 134,   1, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16, 128,  65,   0, 
      0,   0,   2,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   0,   0,   0,   0, 
    246,  15,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   8, 114,   0,  16,   0, 
      1,   0,   0,   0,  86,   5, 
     16,   0,   0,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,  21,   0,   0,   0, 
     50,   0,   0,  10, 178,   0, 
     16,   0,   0,   0,   0,   0, 
      6,   0,  16,   0,   0,   0, 
      0,   0,  70, 136,  32,   0, 
      0,   0,   0,   0,  20,   0, 
      0,   0,  70,   8,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 114,   0,  16,   0, 
      0,   0,   0,   0, 166,  10, 
     16,   0,   0,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,  22,   0,   0,   0, 
     70,   3,  16,   0,   0,   0, 
      0,   0,  16,   0,   0,   7, 
    130,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     68,   0,   0,   5, 130,   0, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   7, 
    114,  32,  16,   0,   3,   0, 
      0,   0, 246,  15,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   5, 130,  32, 
     16,   0,   3,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     50,  32,  16,   0,   4,   0, 
      0,   0,  70,  16,  16,   0, 
      4,   0,   0,   0,  54,   0, 
      0,   8, 194,  32,  16,   0, 
      4,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     62,   0,   0,   1
};
