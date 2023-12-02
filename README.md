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
lower right (see screenshot). The blue triangle is actually just the viewport background (`glClear`), while the
gradient-filled one is a real textured tri.

![Screenshot of correct rendering](/img/good_render.png)

<br>

### Issue 1: Not all texture image units are usable

#### Reproduction case name
`texunit_limit`

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
If a compilation error does occur, the real maximum texture image unit will be found and logged to stderr (for info/
reference only, doesn't affect the visual result).

The test render will always use the original fragment program (with maximum texture unit as per
`MAX_TEXTURE_IMAGE_UNITS_ARB`), so correct output will only be observed if the driver behaves according to the
specification.

<br>

### Issue 2: RET outside of subroutine crashes/hangs shader compiler

#### Reproduction case name
`ret_crash`

#### Priority
High (workaround identified, but suboptimal)

#### Issue description
In NV_fragment_program2 and NV_vertex_program2/3, it is valid to use the `RET` instruction with an empty call stack
(outside of a subroutine) and doing so should simply terminate the fp/vp early. The AMD shader compiler does not seem
to be able to handle this use and instead appears to crash/hang.

Programs can generally be rewritten using other control flow statements to not require the use of RET like this, but
a driver-level fix would be appreciated (if at all possible) to maintain current code structure. In programs utilising
subroutines, it may be necessary to use additional branching to prevent execution falling into unwanted paths.

#### Reproduction case notes
The reproduction program simply uses `RET` instructions outside of subroutines. The vertex program contains one at the
top-level of execution flow (outside of any control flow structures), while the fragment program places one within an
`IF` block.  
Note that compilation of these programs does not fail with an error (which would be logged to stderr), but compilation
does not return at all!

The test render will only work if RET behaves correctly (programs compile and RET terminates execution).

<br>

### Issue 3: Branch conditions are ignored on CAL and RET

#### Reproduction case name
`branchcond_ignored`

#### Priority
Low (easy workaround known)

#### Issue description
NV_fragment_program2 and NV_vertex_program2/3 allow the use of a branch condition (e.g. `(GT.x)`) on `CAL` and `RET`
instructions. However, this seems to be ignored (the instructions execute regardless of specified contition).

This is easy to work around by placing these instructions within `IF` blocks with the appropriate condition, so is not
considered a priority.

#### Reproduction case notes
The vertex program used to reproduce the issue uses a `CAL` instruction with `(FL)` (false) branch condition, yet the
referenced subroutine is still executed. Similarly, the fragment program uses a `RET` instruction with `(FL)` branch
condition, yet this instruction still causes its subroutine to return.

Vertex program failure causes the triangle to not render (vertex positions are zeroed). Fragment program failure causes
the triangle to appear black.

<br>

### Issue 4: No swizzling of fogcoord attribute

#### Reproduction case name
`fogcoord_swizzle`

#### Priority
Low (easy workaround known)

#### Issue description
When accessing the fogcoord attribute in a fragment program (possible in vertex programs too, untested), directly
accessing components via swizzling (e.g. `fragment.fogcoord.x`) does not work as expected. Rather than the requested
components, it seems that the unswizzled `((f, 0, 0, 1)` vector is returned instead.

Loading the value into a temporary variable first, then swizzling said variable, works as expected.

This may also affect other attributes (not sure).

#### Reproduction case notes
The left side of the gradient-filled triangle is correctly rendered by swizzling a temporary variable. The right side
uses the same logic but applied to incorrectly unswizzled attribute access instead, resulting in incorrect output.

If comfortable with editing the fragment program code, it's trivial to observe that `fc`, `fc_x`, `fc_y`, ... are all
interchangeable without affecting the output (hence the observation that the unswizzled vector is returned in all
cases).

<br>

### License

Test programs are under MIT.  
glad dependency is public domain.

GLFW is included as a submodule, refer to GLFW for information if redistributing.
