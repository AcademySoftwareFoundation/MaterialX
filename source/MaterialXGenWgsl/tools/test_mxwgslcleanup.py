"""Tests for mxwgslcleanup.py.

These guard against the class of bug that is otherwise painful to find: a readability
pass that emits VALID WGSL but silently changes behaviour (e.g. reading an uninitialized
variable). Run with `pytest`, or standalone: `python test_mxwgslcleanup.py`.

Only requires `tree-sitter-language-pack` (the transpile requirement) — no naga needed,
since the raw naga output for the tricky functions is embedded as fixtures below.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mxwgslcleanup as m


# --- differential semantic invariant ----------------------------------------

def uninitialized_whole_reads(fn_text):
    """Count locals that are read as a WHOLE value yet never assigned as a whole.

    A variable declared `var x: T;` (no initializer) that is passed to a call, returned,
    or used on an assignment RHS, but only ever written via member/index stores
    (`x.f = ...`) — or not written at all — reads its zero default. `collapse` removing a
    `x = param` copy-in is exactly what turns a safe shadow into one of these.

    This is a per-function count. It is deliberately compared RAW-vs-CLEANED (not asserted
    to be zero) so that legitimate struct builders (`var fd: FresnelData; fd.a=..; return fd;`)
    — which have the same shape before and after cleanup — never trip it; only a NEW
    occurrence introduced by cleanup is a regression.
    """
    src = fn_text.encode("utf-8")
    tree = m._parser().parse(src)
    fn = m._functionDecl(tree.root_node)
    if fn is None:
        return 0
    body = m._compoundBody(fn)
    if body is None:
        return 0

    uninit = {}
    for stmt in m._statements(body):
        name = m._varDeclName(stmt, src)
        if name and b"=" not in src[stmt.start_byte:stmt.end_byte]:
            uninit[name] = stmt

    assigns = m._assignmentTargets(body, src)
    count = 0
    for name, decl in uninit.items():
        name_assigns = assigns.get(name, [])
        has_whole = any(m._isWholeVarAssign(a, src, name) for a in name_assigns)
        # Skip the decl and every assignment's lhs target; whatever `name` occurrences
        # remain are genuine reads of the whole value.
        skip = [(decl.start_byte, decl.end_byte)]
        for a in name_assigns:
            for child in a.children:
                if child.type == "lhs_expression":
                    skip.append((child.start_byte, child.end_byte))
        reads = m._identUsesOutside(body, src, name, skip)
        if reads > 0 and not has_whole:
            count += 1
    return count


# --- fixtures: raw naga output (input to cleanup) ----------------------------

# The regression that caused this test to exist: `fd_8` is a copy of the `fd` param that is
# mutated by a FIELD write (`fd_8.refraction = true`) then read as a whole struct. collapse
# must keep the `fd_8 = fd` copy-in; deleting it left fd_8 zero-initialized -> broken glass.
FD_REFRACT_SHADOW = (
    "fn mx_surface_transmission(N_13: vec3f, V_7: vec3f, X_1: vec3f, alpha_15: vec2f, "
    "distribution_1: i32, fd_7: FresnelData, tint: vec3f) -> vec3f {\n"
    "    var N_14: vec3f;\n"
    "    var V_8: vec3f;\n"
    "    var X_2: vec3f;\n"
    "    var alpha_16: vec2f;\n"
    "    var distribution_2: i32;\n"
    "    var fd_8: FresnelData;\n"
    "    var tint_1: vec3f;\n"
    "\n"
    "    N_14 = N_13;\n"
    "    V_8 = V_7;\n"
    "    X_2 = X_1;\n"
    "    alpha_16 = alpha_15;\n"
    "    distribution_2 = distribution_1;\n"
    "    fd_8 = fd_7;\n"
    "    tint_1 = tint;\n"
    "    fd_8.refraction = true;\n"
    "    if false {\n"
    "        {\n"
    "            let _e33 = tint_1;\n"
    "            let _e34 = mx_square_vec3(_e33);\n"
    "            tint_1 = _e34;\n"
    "        }\n"
    "    }\n"
    "    let _e35 = N_14;\n"
    "    let _e36 = V_8;\n"
    "    let _e37 = X_2;\n"
    "    let _e38 = alpha_16;\n"
    "    let _e39 = distribution_2;\n"
    "    let _e40 = fd_8;\n"
    "    let _e41 = mx_environment_radiance(_e35, _e36, _e37, _e38, _e39, _e40);\n"
    "    let _e42 = tint_1;\n"
    "    return (_e41 * _e42);\n"
    "}\n"
)
FD_REFRACT_PARAMS = ["N", "V", "X", "alpha", "distribution", "fd", "tint"]

# A plain read-only param shadow: should collapse away entirely and restore the GLSL name.
READONLY_SHADOW = (
    "fn f(a_1: f32) -> f32 {\n"
    "    var a_2: f32;\n"
    "    a_2 = a_1;\n"
    "    let _e5 = a_2;\n"
    "    return (_e5 * 2.0);\n"
    "}\n"
)
READONLY_PARAMS = ["a"]

# A legitimate struct builder (NOT a param shadow): zero-init then fill field-by-field.
# cleanup must leave it working; the invariant must NOT flag it (same shape raw and cleaned).
STRUCT_BUILDER = (
    "fn mk() -> FresnelData {\n"
    "    var fd: FresnelData;\n"
    "    fd.ior = 1.5;\n"
    "    fd.model = 0i;\n"
    "    return fd;\n"
    "}\n"
)

# A param shadow reassigned in a NESTED scope only: copy-in must be kept (else read
# uninitialized in a sibling branch). Mirrors the tangent-X dielectric case.
NESTED_REASSIGN_SHADOW = (
    "fn g(X_1: vec3f, flag: bool) -> vec3f {\n"
    "    var X_2: vec3f;\n"
    "    X_2 = X_1;\n"
    "    if flag {\n"
    "        X_2 = (X_2 * 2.0);\n"
    "    }\n"
    "    let _e9 = X_2;\n"
    "    return _e9;\n"
    "}\n"
)
NESTED_PARAMS = ["X", "flag"]

FIXTURES = [
    ("fd_refract_field_shadow", FD_REFRACT_SHADOW, FD_REFRACT_PARAMS),
    ("readonly_shadow", READONLY_SHADOW, READONLY_PARAMS),
    ("struct_builder", STRUCT_BUILDER, None),
    ("nested_reassign_shadow", NESTED_REASSIGN_SHADOW, NESTED_PARAMS),
]


# --- tests -------------------------------------------------------------------

def test_cleanup_introduces_no_uninitialized_reads():
    """Core guard: cleanup must never turn a safely-initialized variable into one that
    is read while uninitialized. Would have caught the fd_8 glass-transmission bug."""
    for name, raw, params in FIXTURES:
        cleaned = m.cleanupFunction(raw, params)
        before = uninitialized_whole_reads(raw)
        after = uninitialized_whole_reads(cleaned)
        assert after <= before, (
            f"[{name}] cleanup introduced {after - before} uninitialized whole-value "
            f"read(s) (raw={before}, cleaned={after}).\n--- cleaned ---\n{cleaned}")


def test_field_mutated_shadow_keeps_copy_in():
    """Regression: the FresnelData copy-in must survive so the struct is fully seeded
    before `fd.refraction = true` and the whole-struct read."""
    cleaned = m.cleanupFunction(FD_REFRACT_SHADOW, FD_REFRACT_PARAMS)
    assert uninitialized_whole_reads(cleaned) == 0, cleaned
    # The struct is read whole; whatever name it keeps, it must be whole-assigned first.
    src = cleaned.encode("utf-8")
    tree = m._parser().parse(src)
    body = m._compoundBody(m._functionDecl(tree.root_node))
    assigns = m._assignmentTargets(body, src)
    # find the FresnelData local decl name
    fd_name = None
    for stmt in m._statements(body):
        n = m._varDeclName(stmt, src)
        if n and b"FresnelData" in src[stmt.start_byte:stmt.end_byte]:
            fd_name = n
    assert fd_name is not None, cleaned
    assert any(m._isWholeVarAssign(a, src, fd_name) for a in assigns.get(fd_name, [])), (
        f"FresnelData local '{fd_name}' has no whole-variable initializer:\n{cleaned}")


def test_readonly_shadow_collapses_and_renames():
    cleaned = m.cleanupFunction(READONLY_SHADOW, READONLY_PARAMS)
    assert "a_2" not in cleaned, cleaned          # shadow removed
    assert "a_1" not in cleaned, cleaned          # naga name restored to GLSL 'a'
    assert "(a * 2.0)" in cleaned, cleaned


def test_struct_builder_unchanged_shape():
    """A field-filled local struct is a legitimate zero-init pattern; it should still
    read whole (count 1) in both raw and cleaned — proving no false positive."""
    assert uninitialized_whole_reads(STRUCT_BUILDER) == 1
    cleaned = m.cleanupFunction(STRUCT_BUILDER, None)
    assert uninitialized_whole_reads(cleaned) == 1, cleaned


def test_nested_reassign_keeps_copy_in():
    cleaned = m.cleanupFunction(NESTED_REASSIGN_SHADOW, NESTED_PARAMS)
    assert uninitialized_whole_reads(cleaned) == 0, cleaned


def test_cleanup_is_idempotent():
    for _name, raw, params in FIXTURES:
        once = m.cleanupFunction(raw, params)
        twice = m.cleanupFunction(once, params)
        assert once == twice, f"not idempotent for {_name}:\n{once!r}\n!=\n{twice!r}"


if __name__ == "__main__":
    failed = 0
    for fn in sorted(k for k in dict(globals()) if k.startswith("test_")):
        try:
            globals()[fn]()
            print(f"PASS {fn}")
        except AssertionError as exc:
            failed += 1
            print(f"FAIL {fn}: {exc}")
    print(f"\n{'OK' if not failed else str(failed) + ' FAILED'}")
    sys.exit(1 if failed else 0)
