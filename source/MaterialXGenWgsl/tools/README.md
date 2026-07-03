# genwgsl library transpiler

`glsl_to_wgsl.py` transpiles the MaterialX **genglsl shader-node** fragments
(`libraries/{stdlib,pbrlib,lights}/genglsl/mx_*.glsl`) into their **genwgsl** WGSL equivalents.
It exists to keep the WGSL node library in sync with the GLSL originals: the genwgsl nodes were
originally hand-ported and repeatedly drifted as genglsl improved, so most of them are now
generated, and re-running the tool (then diffing) surfaces drift.

```
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out build/genwgsl_generated
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --only mx_noise3d_float mx_sheen_bsdf
```

Requires [`naga`](https://github.com/gfx-rs/wgpu/tree/trunk/naga) (`naga-cli`, v29+) on `PATH` or at
`%NAGA%`. Output is written to a staging dir (default `build/genwgsl_generated/...`); diff it against
the committed `libraries/.../genwgsl/*.wgsl` before copying over.

## Scope

* **In:** standalone node `.glsl` files referenced by `file=` impls in stdlib/pbrlib/lights.
* **Out:** core `lib/` math/helper files (hand-maintained), texture/image and light nodes (naga's
  GLSL frontend has no sampler / dynamic-LightData support — auto-skipped), and a handful of nodes
  that depend on **hand-adapted** lib helpers (see below) — those stay hand-written.

The result is a *reduced* library: generated nodes for everything that resolves cleanly against the
genwgsl lib, hand-written nodes for the rest.

## How it works

**Mental model (read this first).** naga does the actual GLSL→WGSL translation — the `naga`
subprocess call is a single line. Everything else in the script is two adapters around it:

* a **pre-processor** that manufactures a complete, compilable shader naga will accept from an
  incomplete node fragment, and
* a **post-processor** that reconciles naga's (correct but verbose) output with the conventions of
  the *hand-written* genwgsl library.

The tool does **not** reimplement transpilation, and it is a *partial* generator by design (see
Scope): most nodes generate, a known few stay hand-written. The regex/bracket-matching is deliberate
string surgery, not a real parser — naga's own run is the correctness gate, and output is
deterministic (no timestamps; overload stubs indexed by declaration order) so re-running is
byte-identical and `git diff` = drift.

**What a reviewer should focus on:** the two hand-maintained tables — `CALL_MAP` (GLSL overload →
genwgsl name) and `EXPECTED_FALLBACK` (nodes intentionally not generated). They encode the genwgsl
library's divergences from genglsl and must be kept in step with it; everything else is mechanical.

The pipeline, per node function:

1. **Pre-process** — a node fragment has no `main`, `#include`s helpers, carries MaterialX
   `$`-tokens, and uses generator-emitted closure structs, so none of it is valid standalone GLSL.
   Build a naga-parseable translation unit: shared `#define`/`const`/`struct` context + function
   *prototypes* (so helper calls resolve) + closure structs (`BSDF`/`surfaceshader`, from
   `CLOSURE_PREAMBLE`) + `$`-token→placeholder swaps + the one function body + a dummy `main`. The
   parsing (`parse_functions`, `build_context`) exists only to synthesize that context.
2. **Transpile** — run `naga --input-kind glsl --shader-stage frag` (which also validates).
3. **Post-process** — clean up naga's SSA-style output into a readable fragment (collapse param-copy
   shadows, inline single-use temps, `vec3<f32>`→`vec3f`, drop float-literal `f` suffixes, ...).

WGSL has no function overloading, and the hand-written genwgsl `lib/` diverges from genglsl in two
further ways a naive transpile can't satisfy. Two post-process passes bridge this:

1. **`remap_calls` / `CALL_MAP`** — overloaded GLSL helpers get a distinct type-suffixed name in the
   genwgsl lib (`mx_square_f32`, `mx_perlin_noise_float_3d`, `mx_matrix_mul_mat4_vec4`, ...).
   `CALL_MAP` maps `(glsl_helper, parameter_types) → genwgsl name`. naga numbers overload stubs
   `_N` by **prototype-declaration order** (kept and indexed deterministically even when unused),
   and prototypes are emitted sorted, so each call is rewritten to the right lib function.
   A `None` entry marks an intentionally-unsupported (adapted) overload.

2. **`check_lib_arity`** — some genwgsl helpers were hand-adapted beyond overloading
   (e.g. `mx_ggx_energy_compensation` takes a precomputed `vec3f`, not `FresnelData`;
   `mx_subsurface_scattering_approx` grew a `curvature` parameter). This pass parses the real
   genwgsl `lib/*.wgsl` signatures and fails any node whose helper-call arity disagrees, so it
   falls back to hand-written instead of producing a shader that fails to link.

A node that hits either case is reported as `FAIL ... (kept hand-written)` and left out of the
generated set — that is expected, not an error in the tool. The set of such nodes is listed in
`EXPECTED_FALLBACK`; the tool's exit code is non-zero only when a node *outside* that set fails
(a regression — e.g. a genglsl change that breaks a previously-generable node, or a new naga
error). If a node in `EXPECTED_FALLBACK` starts transpiling cleanly, the tool prints a `WARN` so it
can be removed from the set and its generated version committed.

## Build-time validation (CMake)

Configure with `-DMATERIALX_GENERATE_WGSL_LIBRARY=ON` (requires `MATERIALX_BUILD_GEN_WGSL`, a Python
interpreter, and the `naga` CLI) to add a `MaterialXGenWgslLibrary` target that runs this tool over
`libraries/` on every build, emitting to `${CMAKE_BINARY_DIR}/genwgsl_generated`. Transpile/naga
errors and regressions then surface as **build errors** (the committed `.wgsl` files stay the source
of truth — the build-tree output is for validation only). The option defaults `OFF` so ordinary
builds need neither Python nor naga.

CMake finds naga from, in order: `-DMATERIALX_NAGA_EXECUTABLE=/path/to/naga`, the `NAGA` environment
variable, `PATH`, and the standard cargo bin directories (`$CARGO_HOME/bin`, `~/.cargo/bin`,
`%USERPROFILE%\.cargo\bin`) — so an existing `cargo install naga-cli` is picked up even when
`~/.cargo/bin` is not on `PATH`.

If naga is still not found, CMake locates **cargo** the same way EMSDK is located
(`-DMATERIALX_CARGO_PATH=<cargo home>`, then `$CARGO_HOME`, then the standard `~/.cargo` /
`%USERPROFILE%\.cargo` locations, then `PATH`) and builds naga into the build tree
(`<build>/naga`) via `cargo install naga-cli`. This install runs at **build** time (not configure,
so configure stays fast) and is cached — it recompiles only if the binary is missing. cargo must
already be installed; MaterialX does not bootstrap the Rust toolchain. If cargo cannot be found,
generation is skipped with a warning.

## Sync workflow

1. Regenerate to staging: `python glsl_to_wgsl.py --libraries libraries --out build/genwgsl_generated`.
2. Diff `build/genwgsl_generated/**/*.wgsl` against `libraries/**/genwgsl/*.wgsl`.
3. Copy the generated files over the committed ones (the hand-written fallbacks and `lib/` are not
   produced by the tool, so they are preserved).
4. Rebuild + restage test data, run the `[genwgsl]` tests, and `naga`-validate the emitted
   `Default.{vertex,pixel}.wgsl`.
