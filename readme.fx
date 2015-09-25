     .,o'       `o,.
   ,o8'           `8o.
  o8'               `8o
 o8:                 ;8o
.88                   88.
:88.                 ,88:
`888                 888'
 888o   `888 888'   o888
 `888o,. `88 88' .,o888'
  `8888888888888888888'
    `888888888888888'
       `::88;88;:'
          88 88
          88 88
          `8 8'
           ` '

+----------------------------------+
|        Table of Contents         |
|                                  |
|  1 - Setup                       |
|  2 - Recommend System Specs      |
|  3 - Mesa vs. Sage               |
|  4 - Interesting Paramaters      |
|  5 - Interesting Mesa Parameters |
|  6 - Interesting Sage Parameters |
|  7 - Known General Issues        |
|  8 - Known Mesa Issues           |
|  9 - Known Sage Issues           |
| 10 - Other tidbits               |
+----------------------------------+

1 - Setup
---------------------------------------------------------
* Pick the correct glide driver (glide3x.dxe) for your 3dfx card from
  one of the directories listed below and put it in the same place as
  q2.exe:
  - voodoo/sst1  -> Voodoo Graphics
  - voodoo/sst96 -> Voodoo Rush
  - voodoo/cvg   -> Voodoo2
  - voodoo/h5    -> Banshee and Voodoo 3/4/5
* Pick a renderer (gl.dxe) from one of the directories listed below
  and put it in the same place as q2.exe:
  - opengl/mesa -> Mesa v6.4.3
  - opengl/sage -> Sage (be sure to copy sage.ini as well)
* It is recommended to use RayeR's MTRRLFBE.EXE utility
  (http://rayer.g6.cz/programm/mtrrlfbe.exe) to setup a
  write-combining MTRR for the LFB.  This will dramatically increase
  performance on Voodoo Rush and higher cards.

2 - Recommended System Specs
---------------------------------------------------------
* Pentium 2 400mhz.  Pentium 3 550mhz or higher for the best experience.
* Voodoo 3.  Older cards will work, but unknown how well they perform in
  faster computers.

3 - Mesa vs. Sage
---------------------------------------------------------
* Sage is much faster than Mesa, very close to or in some cases exceeding
  Windows performance compared to the OpenGL ICD by AmigaMerlin v2.9.
  It is not totally perfect, it has it's own share of issues read below
  for more information.
* Mesa has been more thoroughly tested but also has it's own share of issues.
  Read below for more information.

4 - Interesting parameters
---------------------------------------------------------
* -bpp for setting 15, 16, or 32 bpp on Voodoo 4/5 cards.
  "set vid_glbpp" can also control the bpp in-game.  Requires a vid_restart.
* FX_GLIDE_2PPC controls the 2 Pixels Per Clock mode.  Valid only for
  Voodoo 4/5 cards.  Disabled in multi-texturing mode.  Valid values are
  -1 to disable, 0 driver decides, 1 forced on.  Default is 1.
* FX_GLIDE_LOD_BIAS sets the level detail.  Valid only for Voodoo 3/4/5 cards.
  This one was kind of tricky to figure out, but basically if you want say
  +0.5 you would set the value to 5, for +1.2 you'd set it to 12 and so on.
  Valid ranges are -32 to 32.
* FX_GLIDE_NO_SPLASH=1 to disable the splash logo on Voodoo 1 and 2 cards
  on startup.
* FX_GLIDE_SWAPPENDINGCOUNT to set the number of frames to buffer.  Default
  value is 2, but can go as high as 6.  Traditionally, in Windows, higher
  values can be a slight speed advantage but can introduce input lag.  On
  the hardware I tested it on I was able to gain a few extra frames in
  timedemos (about 3-4) with 6 with no input lag and I am even able to
  use m_filter in combination with this.
* SSTH3_ALPHADITHERMODE set to 3 for the "Smoother" option that is equivalent
  in the Windows control panel for Alpha Blending Quality.  Values 0-2 all
  use the same code path.  Defaults to 1.
* SSTH3_SLI_AA_CONFIGURATION to control SLI on Voodoo 5 5500 as well as
  anti-aliasing on these cards.
  - For Voodoo 4 and 5:
    0 - SLI Disabled, AA Disabled.
    1 - SLI Disabled, AA 2x Enabled.
  - For Voodoo 5 5500:
    2 - SLI Enabled, AA Disabled (default setting).
    3 - SLI Enabled, AA 2x Enabled.
    4 - SLI Disabled, AA 4x Enabled.
  Options 5-8 are for Voodoo 5 6000 users: if you're out there, send me
  an email!

5 - Interesting Mesa parameters
---------------------------------------------------------
* MESA_FX_IGNORE_CMBEXT to allow Voodoo 4/5 to perform single-pass
  trilinear.  This also provides a small speed boost of 4-5 fps on average
  in my timedemo tests with bilinear.  Mesa warns some advanced (multi)texturing
  modes won't work (GL_EXT_texture_env_combine), but multitexturing is slower
  in Mesa and is recommended to be disabled (see below).
* MESA_FX_IGNORE_TEXFMT set to any value (including 0) to disable the
  32bpp-like quality on 16bpp modes.  This causes a slightly performance hit.
  This is enabled by default, and only affects Voodoo 4 and 5.
* MESA_CODEGEN enables code generation for TnL code.  May help performance,
  but I have personally seen no benefits.

6 - Interesting Sage parameters
---------------------------------------------------------
* See sage.ini for more information.

7 - Known General Issues
---------------------------------------------------------
* gl_ext_multitexture is supported by the driver but appears to have an
  approximately 10fps loss on average (more in complex scenes) on everything
  I tested it on EXCEPT in modes 1280x1024 and 1600x1200 where it really
  helps.  It defaults to 0 by default.  If you would like to try it anyways
  set it to 1 and do a vid_restart.

8 - Known Mesa Issues
---------------------------------------------------------
* Mesa is slower than MiniGL drivers such as the WickedGL driver or the
  OpenGL ICD from 3dfx.  These DLLs rely on special Quake engine hacks
  to speed up rendering.  They are closed source so we do not know what
  tricks were used.  However, Mesa in DOS is comparable in speeds to
  Mesa DLLs for Windows.
* Outdoor scenes are slower compared to Windows OpenGL ICD.  This issue
  also exists in Mesa.  You can verify by downloading the Windows Mesa DLLs
  from http://falconfly.de/ .
* gl_ext_pointparameters can lock some machines when particles are drawn,
  i.g. if you start a map and try to fire your gun and the game locks up.
  This is why it is disabled by default in Q2DOS.  But if you are using
  a custom config file imported from another OS it may secretly have this
  enabled.
* Trilinear filtering is disabled by default on Voodoo 4/5 unless you use
  MESA_FX_IGNORE_CMBEXT.
* Warehouse maps with power cubes.  Grabbing/adding a power cube to/from a
  generator uses a dynamic lighting effect to dim or turn on the lights.
  This effect drastically slows the renderer down briefly for a few seconds
  until the effect is finished.  The issue persists on Mesa v6.1 and higher
  due to the tnl code being completely overhauled.  The offending gl functions
  are in DrawGLPolyChain in gl_rsurf.c.  You can use Mesa v6.0.1 or Sage
  if it truly bothers you. DMesaGetProcAddress will have to be re-added to the
  Mesa v6.0.1 code (trivial to do so, do a grep for it in current Mesa code).

9 - Known Sage Issues
---------------------------------------------------------
* Trilinear filtering does not work on Voodoo 4 and 5.
* Static builds have issues with multiple vid_restarts and mode changes.
  You seem to be allowed about 2 or 3 per session until a malloc error
  bombs it out.  This is only going to affect users building custom builds
  from source.  Distribution packages are unaffacted.
* Black screens with segfaults on SSE processors (i.e. P3 and higher).
  Due to an issue with the SSE specific code in Sage you may or may not
  be able to start the game.  As a consequence, it must be explicitly enabled
  via the sage.ini as 3dfx.enable.sse = y.  If you build from source you can
  force GCC SSE optimizations with -march=pentium3 and 3dfx.enable.sse = n and
  get similar performance without the crashes.  Distributing SSE-only builds
  of DXEs and binaries is impractical; you must build it yourself.
* Requires initialization twice for proper performance.  There is a hack in
  place until this issue can be resolved.

10 - Other tidbits
---------------------------------------------------------
* 3DFX's reference hardware for benchmarking with the VSA-100 chipset
  (Voodoo 4 and 5) was a Pentium 3 866mhz.  A setup in this range provides
  the smoothest framerates and is recommended to play the game comfortably.
  See http://www.falconfly.de/downloads/3dfx-v5-PC-Reviewers-Guide.zip for
  more details.
