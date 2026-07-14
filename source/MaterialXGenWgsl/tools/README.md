# genwgsl library transpiler

`glsl_to_wgsl.py` transpiles the MaterialX **genglsl shader-node** fragments
(`libraries/{stdlib,pbrlib,lights}/genglsl/mx_*.glsl`) and **genglsl/lib/** helpers
(`libraries/{stdlib,pbrlib}/genglsl/lib/*.glsl`) into their **genwgsl** WGSL equivalents.
It exists to keep the WGSL node library in sync with the GLSL originals: the genwgsl nodes were
originally hand-ported and repeatedly drifted as genglsl improved, so most of them are now
generated, and re-running the tool (then diffing) surfaces drift.

```
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --only mx_noise3d_float mx_sheen_bsdf
```

Requires [`naga`](https://github.com/gfx-rs/wgpu/tree/trunk/naga) (`naga-cli`, v29+) on `PATH` or at
`%NAGA%`. The generated node and `lib/` files are **not committed** — they are derived artifacts
produced from genglsl (the single source of truth). Passing `--out libraries` writes each generated
file straight into `libraries/<lib>/genwgsl/`, populating the runtime library in place alongside the
hand-written texture/light/`EXPECTED_FALLBACK` nodes. CI runs exactly this to build/ship WGSL; run it locally
to work on the WGSL target. Use `--out <dir>` to emit to a staging dir instead.

## Scope

* **In:** standalone node `.glsl` files referenced by `file=` impls in stdlib/pbrlib/lights, and
  `genglsl/lib/*.glsl` helper files referenced by those nodes.
* **Out:** texture/image and light nodes (naga's GLSL frontend has no sampler / dynamic-LightData
  support — auto-skipped), and `mx_chiang_hair_bsdf` (naga limitation — `EXPECTED_FALLBACK`).

The result is a *reduced* library: generated nodes and `lib/` helpers for everything that resolves
cleanly against genglsl, hand-written nodes for the rest.

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
genwgsl name) and `EXPECTED_FALLBACK` (nodes intentionally not generated). They encode overload
renaming and known naga limitations; everything else is mechanical.

**Execution order in `main()`:**

1. `transpile_libs()` — topological sort of `genglsl/lib/*.glsl` (22 files), per-function naga
   transpile (or sibling-body path for heavy intra-file deps), `LIB_PREAMBLE`
   (`DIRECTIONAL_ALBEDO_METHOD 0`, etc.), inverted `CALL_MAP` for overload output names,
   `LIB_FIELD_RENAMES` (`tf_*` → `thinfilm_*`), texture stubs (`$envPrefilterMip`, etc.), and a
   special handler for `mx_closure_type` (`LIB_STRUCT_PREAMBLE` aggregates closure structs).
2. `build_wgsl_lib_symbols()` — reads generated `lib/*.wgsl` from `--out` for arity validation.
3. `transpile_nodes()` — existing per-node loop.

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

WGSL has no function overloading. Two post-process passes bridge this:

1. **`remap_calls` / `CALL_MAP`** — overloaded GLSL helpers get a distinct type-suffixed name in the
   genwgsl lib (`mx_square_f32`, `mx_perlin_noise_float_3d`, `mx_matrix_mul_mat4_vec4`, ...).
   `CALL_MAP` maps `(glsl_helper, parameter_types) → genwgsl name`. For lib transpilation the map
   is inverted when emitting function definitions. naga numbers overload stubs `_N` by
   **prototype-declaration order** (kept and indexed deterministically even when unused), and
   prototypes are emitted sorted, so each call is rewritten to the right lib function.
   A `None` entry marks an intentionally-unsupported overload.

2. **`check_lib_arity`** — parses the generated `lib/*.wgsl` signatures and fails any node whose
   helper-call arity disagrees, so it falls back to hand-written instead of producing a shader that
   fails to link.

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

## Local workflow

Since the generated files are not committed, there is no baseline to diff against — you simply
regenerate the library in place whenever genglsl changes:

1. Regenerate in place: `python glsl_to_wgsl.py --libraries libraries --out libraries`. This overwrites
   generated node and `lib/` files under `libraries/{stdlib,pbrlib}/genwgsl/` and leaves the
   hand-written texture/light/`EXPECTED_FALLBACK` nodes untouched. A non-zero exit means an
   *unexpected* failure (a regression) — the known `EXPECTED_FALLBACK` nodes failing is normal.
2. Configure with `-DMATERIALX_BUILD_GEN_WGSL=ON`, rebuild + restage test data, run the `[genwgsl]`
   tests, and `naga`-validate the emitted `Default.{vertex,pixel}.wgsl`.

CI performs step 1 on every job that builds or ships WGSL (the web viewer, sdist/wheels, and the
tagged-release archives), so a change that breaks the WGSL target fails CI.
