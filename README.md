# AMD OpenGL ARB Program Issue Reproductions
## (December 2023)

Currently, there are some issues with ARB assembly vertex/fragment programs on AMD GPUs (using recent Windows drivers,
see screenshot below). The issues are described below. (note that this is not a full list of known issues, just a subset
affecting our use)  
It is believed that these issues were introduced in the major driver update released in 2022.

![Screenshot of Radeon software showing driver version(s): 23.19.05.01-231017a...](/img/test_drv_ver.png)

My hardware is a Renoir APU's integrated graphics, however I have heard that the same issues also affect newer GPUs
based on RDNA architecture.

I am not at liberty to discuss the software affected by these issues, but reproduction test cases are included for all
issues discussed.
Correct rendering for all tests is a blue triangle in upper left of viewport and red->green gradient-filled triangle in
lower right (see screenshot).

![Screenshot of correct rendering](/img/good_render.png)

<br>

### Issue 1: Incorrect texture image unit limits

#### Priority
High (no workaround identified)

#### Issue description
According to [ARB_fragment_program documentation](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_fragment_program.txt),
`<texImageUnitNum>` (as in `texture[n]`) is an integer between 0 and `MAX_TEXTURE_IMAGE_UNITS_ARB-1`.
However, observed behaviour is that it is limited to a maximum of 15, regardless of `MAX_TEXTURE_IMAGE_UNITS_ARB`.

On my system, `MAX_TEXTURE_IMAGE_UNITS_ARB` is 32, and it is known that OpenGL drivers prior to 2022 allowed higher unit
indices in fragment programs. Therefore, it is expected that the driver should be fixed to make all 32 reported texture
image units available to use. However, simply decreasing `MAX_TEXTURE_IMAGE_UNITS_ARB` would also satisfy the
specification.

The same issue also affects [NV_vertex_program3](https://registry.khronos.org/OpenGL/extensions/NV/NV_vertex_program3.txt),
however, the incorrect limit is not affecting us in vertex programs. We also understand that NV_vertex_program and
NV_fragment_program extensions are not fully supported by AMD. Therefore, a fix is not required for vertex programs.

#### Reproduction case notes
The reproduction program tries to use the maximum texture unit (according to `MAX_TEXTURE_IMAGE_UNITS_ARB`) in a
fragment program (`TEX` instruction referencing `texture[n]`). It is expected that this program will compile without
errors in a correctly functioning driver.  
If a compilation error does occur, the real maximum texture image unit will be found and logged (for info only).

The test render will always use the original fragment program (with maximum texture unit as per
`MAX_TEXTURE_IMAGE_UNITS_ARB`), so correct output will only be observed if the driver behaves according to docs.

<br>

### Issue 2: RET at top level of program breaks shader compilation

#### Priority
High (workaround identified, but not guaranteed by specification)

#### Issue description
In NV_fragment_program2 and NV_vertex_program2/3, it is valid to use the `RET` instruction with an empty call stack
(outside of a subroutine) and doing so should simply terminate the fp/vp early. The AMD shader compiler does not seem
to be able to handle this use and instead appears to crash.

This instruction is important to allow for complex branching (ideally shouldn't be done in shaders, but sometimes is
necessary).

#### Reproduction case notes
<TODO>

<br>

### Issue 3: Incorrect swizzling of fogcoord (and possibly other attributes?)

<TODO>
(note: observed in frag programs)

<br>

### License

Test programs are under MIT.  
glad dependency is public domain.

GLFW is included as a submodule, refer to GLFW for information if redistributing.
