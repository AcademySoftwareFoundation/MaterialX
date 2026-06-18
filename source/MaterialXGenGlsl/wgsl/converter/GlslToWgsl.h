//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLSLTOWGSL_H
#define MATERIALX_GLSLTOWGSL_H

/// @file
/// GLSL-to-WGSL string rewriting utilities for the WGSL shader generator.
///
/// This header is internal to MaterialXGenGlsl (used only by WgslShaderGenerator) and is
/// not part of the installed public API.
///
/// The MaterialX GLSL code path (HwSurfaceNode, CompoundNode, SourceCodeNode and the
/// genglsl `.glsl` library sources) emits GLSL-flavoured text. These utilities convert the
/// common GLSL patterns to valid WGSL at emit time, so the WGSL generator can reuse the
/// genglsl node implementations and library functions without a parallel set of `.wgsl`
/// sources:
///   - types:               `float`/`vec3`/`mat4`        -> `f32`/`vec3f`/`mat4x4f`
///   - const declarations:  `const float x = 1.0`        -> `const x: f32 = 1.0`
///   - variable decls:      `vec3 v = ...`               -> `var v: vec3f = ...`
///   - ternary operators:   `a ? b : c`                  -> `select(c, b, a)`
///   - function signatures: `void f(float x, out int y)` -> `fn f(x: f32) -> i32`
///   - out parameters:      body refs to `y` become `(*y)` with `y: ptr<function,T>`
///   - arrays:              `vec2 c[3]`                  -> `c: array<vec2f, 3>`
///   - shift counts:        `x << k` (signed `k`)        -> `x << u32(k)`
///   - float vec ctors:     `vec2(intX, intY)`           -> `vec2f(f32(intX), f32(intY))`
///
/// These are a *targeted* converter for the constrained, machine-generated GLSL the genglsl
/// node implementations and libraries produce -- not a general-purpose GLSL parser. They
/// assume single-statement-per-line output and well-formed input. As a safety net the
/// generator runs findResidualGlsl() on its output and warns if any unconverted GLSL
/// remains, so an incomplete rewrite is observable rather than silently invalid.

#include <MaterialXCore/Library.h>

#include <string>
#include <utility>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

namespace GlslToWgsl
{

/// Map a GLSL type keyword to its WGSL equivalent (`float`->`f32`, `vec3`->`vec3f`, ...).
/// Returns the input unchanged if it is already a WGSL type or unknown.
string mapType(const string& glslType);

/// Apply the stateless, single-line GLSL->WGSL syntax conversions (type casts, const/var
/// declarations, ternaries, increment/decrement, function signatures). Out-parameter body
/// dereferencing is handled by LineRewriter, not here.
string rewriteAll(string line);

// --- Whole-shader post-processing passes (run in order over the finished stage) ---

/// Dereference `ptr<function,T>` (out) parameters in compound function bodies, which assign
/// to them by bare name (`out1 = ...`). Library functions already use `(*out1)`.
string derefPointerParams(const string& shader);

/// Drop duplicate module-scope definitions (the same `const`/`alias`/`struct`/`fn` emitted
/// by more than one library file), keeping the first; WGSL forbids redeclaration.
string dedupDefinitions(const string& shader);

/// Wrap integer MaterialX booleans in `bool(...)` at call sites whose callee declares a
/// native WGSL `bool` parameter. Run after resolveOverloads.
string coerceBoolCallSites(const string& shader);

/// Resolve GLSL function overloading (which WGSL forbids): give each same-named definition a
/// unique type-suffixed name and rewrite every call site by argument-type inference.
string resolveOverloads(const string& shader);

/// Convert the HwNumLightsNode / HwLightSamplerNode GLSL function stubs that per-line
/// rewriting leaves behind. Does not touch native WGSL bindings.
string rewriteResidualGlslFunctions(const string& shader);

/// Fix `else { // comment }` empty blocks that orphan the real `{ ... }` body and break
/// brace balance in generated WGSL.
string repairEmptyElseCommentBlocks(const string& shader);

/// Split GLSL chained assignment `a = b = c = expr;` (legal GLSL, rejected by WGSL) into
/// separate right-to-left statements (`c = expr; b = c; a = b;`). Only fires when every
/// target is a simple lvalue, as in the mx_noise.glsl hash seed.
string splitChainedAssignments(const string& shader);

/// Scan finished WGSL for residual GLSL tokens (`#version`, `layout(`, `sampler2D `, ...).
/// Returns one human-readable entry per distinct token found; empty when fully converted.
StringVec findResidualGlsl(const string& wgsl);

/// Stateful, function-scope-aware GLSL-to-WGSL line rewriter.
///
/// Feed it one line at a time in source order. It tracks the current function body (via
/// brace depth) so that:
///   - a signature `void f(..., out T r)` becomes `fn f(..., r: ptr<function, T>)` (no
///     `-> T` return), and
///   - every reference to an `out` parameter `r` inside the body becomes `(*r)`.
/// Value-returning signatures (`vec3 g(...)`) gain a `-> vec3f` return type.
class LineRewriter
{
  public:
    /// Rewrite one line (without trailing newline). Updates internal scope state.
    string rewrite(const string& line);

    /// Seed the set of functions known to take a pointer (out/inout) parameter. Used when a
    /// block is rewritten in isolation (see rewriteBlock) and its callees are defined elsewhere.
    void registerPointerFunctions(const StringSet& fns) { _ptrFuncs.insert(fns.begin(), fns.end()); }

  private:
    // Per-line handlers tried in order by rewrite(). Each returns true and fills `out` when
    // it consumes the line; false means "not my case, try the next".
    bool handlePreprocessor(const string& line, string& out);
    bool handleStruct(const string& line, string& out);
    bool handleSignature(const string& line, string& out);
    string rewriteFunctionBodyLine(const string& line);

    // Enter a function body: record out-parameter names and the mutable `var` copies to
    // inject for value parameters; return the rewritten signature line.
    string beginFunction(const string& sig, StringVec&& outNames,
                         const std::vector<std::pair<string, string>>& valueParams);
    // Leave the current function body and clear its per-function state.
    void leaveFunctionScope();

    // The function body currently being rewritten.
    struct FunctionScope
    {
        bool active = false;        // inside a function body (or its signature)
        bool seenOpenBrace = false; // the body's opening brace has been seen
        int braceDepth = 0;         // net brace nesting within the function
        bool injected = false;      // value-parameter `var` copies already emitted
        StringVec outParams;       // out-parameter names to dereference
        StringVec paramInjections; // `var name: T = name_arg;` lines
    };

    // A function signature whose parameter list spans several lines.
    struct SignatureAccum
    {
        bool active = false;
        string buffer;
        int parenDepth = 0;
    };

    // A ternary expression `cond ? a : b` spanning several lines.
    struct TernaryAccum
    {
        bool active = false;
        string buffer;
    };

    // A braceless control header (`if(...)`/`for(...)`/`else`) awaiting its body.
    struct PendingHeader
    {
        bool active = false;
        string line;
    };

    // Minimal `#if/#elif/#else/#endif` evaluator (WGSL has no preprocessor).
    struct Preprocessor
    {
        struct Frame
        {
            bool active;
            bool taken;
            bool parentActive;
        };
        std::vector<Frame> stack;
        bool active() const;             // all enclosing conditionals select the line
        void update(const string& directive);
    };

    // Names of functions seen so far that take a pointer (out/inout) parameter. Used to decide
    // whether an out-parameter forwarded as a call argument must stay a bare pointer.
    StringSet _ptrFuncs;

    FunctionScope _func;
    SignatureAccum _sig;
    TernaryAccum _ternary;
    PendingHeader _pendingHeader;
    Preprocessor _pp;
    bool _inStruct = false; // inside a `struct { ... }` definition body
};

} // namespace GlslToWgsl

MATERIALX_NAMESPACE_END

#endif // MATERIALX_GLSLTOWGSL_H
