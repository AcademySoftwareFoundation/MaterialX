'''Post-process naga-emitted WGSL function bodies for readability.

Uses tree-sitter (the `tree-sitter-language-pack` WGSL grammar) for statement/identifier
analysis, then applies conservative source edits: collapse param-copy shadows, restore GLSL
parameter names, promote readonly/mutable locals, unwrap naga's redundant compound blocks,
flatten else-if chains, and simplify ptr/assignment parens. Parsing only — emission is
byte-range surgery on naga output.
'''

from __future__ import annotations

import re
import sys

_PARSER = None


def fnBase(name):
    '''Strip naga's trailing `_` / `_N` disambiguation suffix.'''
    return re.sub(r"_\d*$", "", name)


def _parser():
    global _PARSER
    if _PARSER is None:
        from tree_sitter_language_pack import get_parser
        _PARSER = get_parser("wgsl")
    return _PARSER


def _nodeText(node, src):
    return src[node.start_byte:node.end_byte].decode("utf-8")


def _firstIdent(node, src):
    if node.type == "identifier":
        return _nodeText(node, src)
    for child in node.children:
        found = _firstIdent(child, src)
        if found:
            return found
    return None


def _functionDecl(root):
    if root.type == "function_declaration":
        return root
    for child in root.children:
        if child.type == "function_declaration":
            return child
    return None


def _compoundBody(fnDecl):
    for child in fnDecl.children:
        if child.type == "compound_statement":
            return child
    return None


def _paramNames(fnDecl, src):
    names = []
    for child in fnDecl.children:
        if child.type != "parameter_list":
            continue
        for param in child.children:
            if param.type == "parameter":
                ident = _firstIdent(param, src)
                if ident:
                    names.append(ident)
    return names


def _stmtList(compound):
    '''Direct statement children of a compound_statement (the language-pack grammar
    has no `statement` wrapper; real statement nodes sit directly under the block).'''
    return [c for c in compound.children if c.is_named and c.type != "comment"]


def _statements(body):
    yield from _stmtList(body)


def _varDeclName(stmt, src):
    '''Declared name for a `var`/`let` variable_statement, else None.'''
    if stmt.type != "variable_statement":
        return None
    return _firstIdent(stmt, src)


def _copyAssignment(stmt, src):
    '''If stmt is `shadow = param` with simple idents, return (shadow, param).'''
    if stmt.type != "assignment_statement":
        return None
    kids = list(stmt.children)
    if len(kids) != 3:
        return None
    lhsNode, opNode, rhsNode = kids
    if lhsNode.type != "lhs_expression" or opNode.type != "=":
        return None
    if rhsNode.type != "identifier":
        return None
    lhs = _firstIdent(lhsNode, src)
    if not lhs or _nodeText(lhsNode, src).strip() != lhs:
        return None  # member/index write (`x.f = ...`), not a whole-variable copy
    rhs = _nodeText(rhsNode, src)
    if lhs and rhs:
        return lhs, rhs
    return None


def _lineRange(stmt, src):
    '''Full source line(s) for a statement, including indent and trailing newline.

    The language-pack grammar leaves the trailing `;` as a sibling token outside the
    statement node, so extend to the end of the physical line (next newline).'''
    lineStart = src.rfind(b"\n", 0, stmt.start_byte) + 1
    nl = src.find(b"\n", stmt.end_byte)
    lineEnd = len(src) if nl == -1 else nl + 1
    return lineStart, lineEnd


def _stmtSpan(stmt, src):
    '''Node byte span extended past an immediately following `;` sibling token.'''
    end = stmt.end_byte
    i = end
    while i < len(src) and src[i:i + 1] in (b" ", b"\t"):
        i += 1
    if i < len(src) and src[i:i + 1] == b";":
        end = i + 1
    return stmt.start_byte, end


def _assignmentTargets(body, src):
    '''Map assigned identifier -> list of assignment_statement nodes.'''
    targets = {}
    stack = [body]
    while stack:
        node = stack.pop()
        if node.type == "assignment_statement":
            lhs = None
            for part in node.children:
                if part.type == "lhs_expression":
                    lhs = _firstIdent(part, src)
                    break
            if lhs:
                targets.setdefault(lhs, []).append(node)
        stack.extend(node.children)
    return targets


def _isWholeVarAssign(assignNode, src, name):
    '''True if assignNode rebinds the whole variable `name` (`name = ...`), not a member/index
    write (`name.field = ...`, `name[i] = ...`). Promotion may only fold whole-variable assigns.'''
    for child in assignNode.children:
        if child.type == "lhs_expression":
            return _nodeText(child, src).strip() == name
    return False


def _identUsesOutside(body, src, name, skipRanges):
    '''Count ident uses not wholly inside any (start, end) skip range.'''
    count = 0
    stack = [body]
    while stack:
        node = stack.pop()
        if node.type == "identifier" and _nodeText(node, src) == name:
            pos = node.start_byte
            if not any(s <= pos < e for s, e in skipRanges):
                count += 1
        stack.extend(node.children)
    return count


def _applyDeletes(src, deleteRanges):
    '''Delete byte ranges from UTF-8 source (end-exclusive).'''
    data = bytearray(src.encode("utf-8"))
    for start, end in sorted(deleteRanges, key=lambda r: r[0], reverse=True):
        del data[start:end]
    return data.decode("utf-8")


def _applyRenames(text, renames):
    '''Word-boundary identifier renames, longest key first.'''
    for old, new in sorted(renames.items(), key=lambda kv: len(kv[0]), reverse=True):
        if old != new:
            text = re.sub(r"\b" + re.escape(old) + r"\b", new, text)
    return text


def _fixMutateShadowReads(text, shadow, param):
    '''After removing a param copy-in, pre-init reads and the first assign RHS use the param.'''
    assignPat = re.compile(
        r"^[ \t]*" + re.escape(shadow) + r"\s*=\s*(.+?);\s*$",
        re.MULTILINE)
    m = assignPat.search(text)
    if not m:
        return text

    before = text[:m.start()]
    assignLine = m.group(0)
    expr = m.group(1)
    after = text[m.end():]

    varDecl = re.compile(r"^[ \t]*var\s+" + re.escape(shadow) + r"\b")
    fixedBefore = []
    for line in before.splitlines(keepends=True):
        if varDecl.match(line):
            fixedBefore.append(line)
        else:
            fixedBefore.append(
                re.sub(r"\b" + re.escape(shadow) + r"\b", param, line))
    before = "".join(fixedBefore)

    exprFixed = re.sub(r"\b" + re.escape(shadow) + r"\b", param, expr)
    stripped = exprFixed.strip()
    if re.fullmatch(r"-\s*\(\s*" + re.escape(param) + r"\s*\)", stripped):
        exprOut = f"(-{param})"
    else:
        exprOut = exprFixed

    indent = re.match(r"^([ \t]*)", assignLine).group(1)
    newline = "\n" if assignLine.endswith("\n") else ""
    return before + f"{indent}{shadow} = {exprOut};{newline}" + after


def _collapseParamShadows(fnText, glslParamNames=None):
    src = fnText.encode("utf-8")
    tree = _parser().parse(src)
    root = tree.root_node
    # The language-pack grammar cannot parse bare multi-arg function-call statements
    # (`foo(a, b, c);`) and marks the tree as having errors, but those errors are local
    # to the call statements; the declarations/copy-ins we act on parse correctly. So do
    # not bail on has_error -- operate on the well-formed statements best-effort.
    fnDecl = _functionDecl(root)
    if fnDecl is None:
        return fnText

    body = _compoundBody(fnDecl)
    if body is None:
        return fnText

    wgslParams = _paramNames(fnDecl, src)
    assignTargets = _assignmentTargets(body, src)
    stmts = list(_statements(body))
    topLevel = {(s.start_byte, s.end_byte) for s in stmts}

    varDeclStmts = {}
    copyStmts = {}
    shadowToParam = {}
    for stmt in stmts:
        shadow = _varDeclName(stmt, src)
        if shadow:
            varDeclStmts[shadow] = stmt
        pair = _copyAssignment(stmt, src)
        if pair:
            lhs, rhs = pair
            if rhs in wgslParams and fnBase(lhs) == fnBase(rhs):
                shadowToParam[lhs] = rhs
                copyStmts[lhs] = stmt

    # Pair naga param names with GLSL source names (same position).
    paramRename = {}
    if glslParamNames and len(glslParamNames) == len(wgslParams):
        for wgslName, glslName in zip(wgslParams, glslParamNames):
            if wgslName != glslName:
                paramRename[wgslName] = glslName

    deleteRanges = []
    identRenames = {}
    mutateFold = []  # (shadow, param) pairs needing self-negate fold after copy removal

    for shadow, param in shadowToParam.items():
        varStmt = varDeclStmts.get(shadow)
        copyStmt = copyStmts.get(shadow)
        if varStmt is None or copyStmt is None:
            continue

        assigns = assignTargets.get(shadow, [])
        skip = [_stmtSpan(varStmt, src), _stmtSpan(copyStmt, src)]
        usesOutside = _identUsesOutside(body, src, shadow, skip)

        if len(assigns) == 1 and usesOutside == 0:
            deleteRanges.extend(skip)
            continue

        if len(assigns) == 1:
            deleteRanges.extend(skip)
            identRenames[shadow] = param
            continue

        # The shadow is reassigned (beyond the copy-in). Dropping the `shadow = param` copy and
        # folding is only safe when every reassignment is at the SAME (top-level) scope as the copy:
        # otherwise a sibling branch or loop iteration would read the shadow uninitialized (e.g. a
        # tangent reassigned only in the reflection branch but read in the environment branch). If
        # any reassignment is nested, keep naga's copy-in intact.
        keepCopy = False
        for a in assigns:
            if a.start_byte >= copyStmt.start_byte and a.end_byte <= copyStmt.end_byte:
                continue  # this is the copy-in itself
            # A member/index write (`shadow.field = …`, `shadow[i] = …`) mutates the copy in
            # place and has no whole-variable reassignment for `_fixMutateShadowReads` to fold,
            # so the `shadow = param` copy-in must stay to seed the other fields — otherwise the
            # shadow is read uninitialized (e.g. `fd.refraction = true` on a FresnelData copy).
            if not _isWholeVarAssign(a, src, shadow):
                keepCopy = True
                break
            st = _stmtContaining(body, a)
            if st is None or (st.start_byte, st.end_byte) not in topLevel:
                keepCopy = True
                break
        if keepCopy:
            continue

        deleteRanges.append(_stmtSpan(copyStmt, src))
        mutateFold.append((shadow, param))

    for wgslName, glslName in paramRename.items():
        identRenames[wgslName] = glslName
        for shadow, target in list(identRenames.items()):
            if target == wgslName:
                identRenames[shadow] = glslName

    text = _applyDeletes(fnText, deleteRanges)
    text = _applyRenames(text, identRenames)
    for shadow, param in mutateFold:
        text = _fixMutateShadowReads(
            text, shadow, identRenames.get(param, param))
    while True:
        cleaned = re.sub(r"\n[ \t]*\n[ \t]*\n", "\n\n", text)
        if cleaned == text:
            break
        text = cleaned
    text = re.sub(r"(\{\n)\s*\n+", r"\1", text)
    return text


def _assignmentRhsText(assignNode, src):
    '''RHS expression text of `lhs = <rhs>` (the rhs is the node's last child;
    it is not wrapped in an `expression` node in the language-pack grammar).'''
    kids = list(assignNode.children)
    if len(kids) >= 3 and kids[0].type == "lhs_expression":
        return _nodeText(kids[-1], src)
    return None


def _stmtContaining(body, innerNode):
    '''Smallest statement node (a direct child of some compound_statement) that
    contains innerNode. With no `statement` wrapper, a statement is any named node
    whose parent is a compound_statement.'''
    node = innerNode
    while node is not None and node != body:
        parent = node.parent
        if parent is not None and parent.type == "compound_statement":
            return node
        node = parent
    return None


def _lineIndent(stmt, src):
    lineStart = src.rfind(b"\n", 0, stmt.start_byte) + 1
    prefix = src[lineStart:stmt.start_byte]
    m = re.match(rb"[ \t]*", prefix)
    return prefix[: m.end()].decode("utf-8") if m else ""


def _pickLocalName(name, wgslParams, taken):
    '''Map naga locals to readable names without shadowing params.'''
    base = fnBase(name)
    if any(fnBase(p) == base for p in wgslParams):
        candidate = f"{base}_f"
    else:
        candidate = base
    if candidate in taken:
        return name
    return candidate


def _applyEdits(text, deleteRanges, replacements):
    '''Apply byte-range deletes and (start, end, newText) replacements.'''
    edits = [(start, end, "") for start, end in deleteRanges]
    edits.extend(replacements)
    edits.sort(key=lambda item: item[0], reverse=True)
    data = bytearray(text.encode("utf-8"))
    for start, end, new in edits:
        replacement = new.encode("utf-8") if isinstance(new, str) else new
        data[start:end] = replacement
    return data.decode("utf-8")


def _braceTokens(compound):
    openBrace = closeBrace = None
    for child in compound.children:
        if child.type == "{":
            openBrace = child
        elif child.type == "}":
            closeBrace = child
    return openBrace, closeBrace


def _compoundInnerWrapper(outer):
    '''If outer's sole statement is a nested compound, return that inner compound.'''
    stmts = _stmtList(outer)
    if len(stmts) != 1:
        return None
    return stmts[0] if stmts[0].type == "compound_statement" else None


def _cleanupMutableLocals(fnText, glslParamNames=None):
    """Promote hoisted `var` + first assign to `var clean = …` when later reassigned."""
    src = fnText.encode("utf-8")
    tree = _parser().parse(src)
    root = tree.root_node
    fnDecl = _functionDecl(root)
    if fnDecl is None:
        return fnText
    body = _compoundBody(fnDecl)
    if body is None:
        return fnText

    wgslParams = _paramNames(fnDecl, src)
    reserved = set(wgslParams)
    if glslParamNames:
        reserved.update(glslParamNames)

    varDecls = {}
    for stmt in _statements(body):
        name = _varDeclName(stmt, src)
        # Only naga's UNINITIALIZED hoist (`var name: T;`) is a promotion candidate. An initialized
        # declaration (`var i = 0i;`, `var I = vec3(0.0);`) is a real accumulator/counter whose
        # value must survive; merging a later (often self-referencing) assignment into it is wrong.
        if name and b"=" not in src[stmt.start_byte:stmt.end_byte]:
            varDecls[name] = stmt

    assignTargets = _assignmentTargets(body, src)
    # Byte ranges of the function body's direct (top-level) statements. naga hoists a `var name: T;`
    # to this scope when the value is assigned in multiple branches; the promotion below may only
    # move that declaration to the first assignment if that assignment is ALSO at this scope.
    topLevel = {(s.start_byte, s.end_byte) for s in _statements(body)}
    deleteRanges = []
    replacements = []
    renames = {}
    taken = set(reserved)

    for name in sorted(varDecls):
        allAssigns = assignTargets.get(name, [])
        wholeAssigns = [a for a in allAssigns if _isWholeVarAssign(a, src, name)]
        # Fold only when EVERY assignment rebinds the whole variable (and there are >=2 of them).
        # If any is a field/index write (`fd.model = ...`, `x[i] = ...`) the value is built in place,
        # so keep naga's hoisted `var` declaration and leave the writes untouched.
        if len(wholeAssigns) != len(allAssigns) or len(wholeAssigns) < 2:
            continue
        firstAssign = min(wholeAssigns, key=lambda node: node.start_byte)
        firstStmt = _stmtContaining(body, firstAssign)
        varStmt = varDecls[name]
        if firstStmt is None:
            continue
        if (firstStmt.start_byte, firstStmt.end_byte) not in topLevel:
            continue  # first assignment is inside a nested block (e.g. an if branch); relocating
                      # the `var` decl there would narrow its scope and break references in sibling
                      # branches or after the block. Keep naga's hoisted declaration as-is.
        rhs = _assignmentRhsText(firstAssign, src)
        if not rhs:
            continue

        clean = _pickLocalName(name, wgslParams, taken)
        taken.add(clean)
        deleteRanges.append(_lineRange(varStmt, src))
        indent = _lineIndent(firstStmt, src)
        replacements.append((
            *_lineRange(firstStmt, src),
            f"{indent}var {clean} = {rhs};\n"))
        if name != clean:
            renames[name] = clean

    if not deleteRanges and not replacements:
        return fnText

    text = _applyEdits(fnText, deleteRanges, replacements)
    text = _applyRenames(text, renames)
    return text


def _dedentBlock(text, spaces=4):
    '''Remove one indent level from non-empty lines.'''
    prefix = " " * spaces
    lines = []
    for line in text.splitlines(keepends=True):
        if line.strip() and line.startswith(prefix):
            lines.append(line[spaces:])
        else:
            lines.append(line)
    return "".join(lines)


def _unwrapRedundantCompounds(fnText):
    '''Remove naga's `if (c) { { stmts } }` wrapper compounds.'''
    while True:
        src = fnText.encode("utf-8")
        tree = _parser().parse(src)
        fnDecl = _functionDecl(tree.root_node)
        if fnDecl is None:
            break
        body = _compoundBody(fnDecl)
        if body is None:
            break

        best = None
        stack = [body]
        while stack:
            node = stack.pop()
            if node.type == "compound_statement":
                inner = _compoundInnerWrapper(node)
                if inner is not None:
                    wrapperStmt = inner
                    size = wrapperStmt.end_byte - wrapperStmt.start_byte
                    if best is None or size > best[0]:
                        best = (size, wrapperStmt, inner)
            stack.extend(node.children)

        if best is None:
            break

        _, wrapperStmt, inner = best
        innerOpen, innerClose = _braceTokens(inner)
        if not innerOpen or not innerClose:
            break

        raw = src[innerOpen.end_byte:innerClose.start_byte].decode("utf-8")
        body = _dedentBlock(raw.strip("\n")).rstrip()
        content = (body + "\n") if body else ""
        start, end = _lineRange(wrapperStmt, src)
        fnText = _applyEdits(fnText, [], [(start, end, content)])
    return fnText


def _simplifyPtrParens(text):
    '''`((*bsdf)).field` -> `(*bsdf).field`.'''
    return re.sub(r"\(\(\*([^)]+)\)\)", r"(*\1)", text)


def _balancedParens(expr):
    depth = 0
    for ch in expr:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth < 0:
                return False
    return depth == 0


def _unwrapAssignmentParens(text):
    '''Drop a single outer paren pair around `let`/`var` rhs when balanced.'''
    def repl(match):
        indent, kind, name, expr = (match.group(1), match.group(2),
                                    match.group(3), match.group(4))
        if _balancedParens(expr):
            return f"{indent}{kind} {name} = {expr};"
        return match.group(0)

    changed = True
    while changed:
        changed = False
        new = re.sub(
            r"^([ \t]*)\b(let|var)\s+(\w+)\s*=\s*\((.+)\);\s*$",
            repl,
            text,
            flags=re.MULTILINE)
        if new != text:
            text = new
            changed = True
    return text


def _elseIfWrapper(elseClause):
    '''If else { if (...) { ... } } return (elseClause, innerIf, expr).

    `expr` is the if-condition parenthesized_expression (parens included).'''
    compound = None
    for child in elseClause.children:
        if child.type == "compound_statement":
            compound = child
    if compound is None:
        return None
    stmts = _stmtList(compound)
    if len(stmts) != 1 or stmts[0].type != "if_statement":
        return None
    innerIf = stmts[0]
    expr = next((c for c in innerIf.children
                 if c.type == "parenthesized_expression"), None)
    if expr is None:
        return None
    return elseClause, innerIf, expr


def _flattenElseIfChains(fnText):
    '''`} else { if (c) {` -> `} else if (c) {`.'''
    while True:
        src = fnText.encode("utf-8")
        tree = _parser().parse(src)
        fnDecl = _functionDecl(tree.root_node)
        if fnDecl is None:
            break
        body = _compoundBody(fnDecl)
        if body is None:
            break

        candidate = None
        stack = [body]
        while stack:
            node = stack.pop()
            if node.type == "else_statement":
                wrapped = _elseIfWrapper(node)
                if wrapped is not None:
                    candidate = wrapped
                    break
            stack.extend(node.children)

        if candidate is None:
            break

        elseClause, innerIf, expr = candidate
        # The `else` keyword is a sibling token BEFORE else_statement in this grammar (not part
        # of the else node), so the rewrite must start at the keyword to avoid a duplicate `else`.
        elseKw = elseClause.prev_sibling
        cutStart = (elseKw.start_byte if elseKw is not None and elseKw.type == "else"
                    else elseClause.start_byte)
        lineStart = src.rfind(b"\n", 0, elseClause.start_byte) + 1
        prefix = src[lineStart:elseClause.start_byte].decode("utf-8")
        indent = re.match(r"^[ \t]*", prefix).group(0)
        innerPart = src[expr.start_byte:innerIf.end_byte].decode("utf-8")
        newElse = indent + "else if " + _dedentBlock(innerPart, 4)
        fnText = (fnText[:cutStart] + newElse
                   + fnText[elseClause.end_byte:])
    return fnText


def _cleanupReadonlyLocals(fnText, glslParamNames=None):
    '''Promote single-assignment `var` locals to `let` with GLSL-style names.'''
    src = fnText.encode("utf-8")
    tree = _parser().parse(src)
    root = tree.root_node
    # See _collapseParamShadows: errors from bare call statements are local; do not bail.
    fnDecl = _functionDecl(root)
    if fnDecl is None:
        return fnText
    body = _compoundBody(fnDecl)
    if body is None:
        return fnText

    wgslParams = _paramNames(fnDecl, src)
    reserved = set(wgslParams)
    if glslParamNames:
        reserved.update(glslParamNames)

    varDecls = {}
    for stmt in _statements(body):
        name = _varDeclName(stmt, src)
        # Only naga's UNINITIALIZED hoist (`var name: T;`) is a promotion candidate; an initialized
        # declaration (`var i = 0i;`) is a real accumulator/counter and must be left intact.
        if name and b"=" not in src[stmt.start_byte:stmt.end_byte]:
            varDecls[name] = stmt

    assignTargets = _assignmentTargets(body, src)
    # Byte ranges of the function body's direct (top-level) statements. The `var` declaration is
    # always at this scope; converting it to a `let` at the assignment site is only valid when the
    # assignment is ALSO at this scope (see the guard below).
    topLevel = {(s.start_byte, s.end_byte) for s in _statements(body)}
    deleteRanges = []
    replacements = []
    renames = {}
    taken = set(reserved)

    for name in sorted(varDecls):
        allAssigns = assignTargets.get(name, [])
        # Promote to `let` only when the variable is assigned exactly once, as a whole (never a
        # field/index write like `fd.model = ...`, which requires it to remain a mutable `var`).
        if len(allAssigns) != 1 or not _isWholeVarAssign(allAssigns[0], src, name):
            continue
        assignNode = allAssigns[0]
        assignStmt = _stmtContaining(body, assignNode)
        varStmt = varDecls[name]
        if assignStmt is None:
            continue
        rhs = _assignmentRhsText(assignNode, src)
        if not rhs:
            continue
        skip = [(varStmt.start_byte, varStmt.end_byte),
                (assignNode.start_byte, assignNode.end_byte)]
        uses = _identUsesOutside(body, src, name, skip)
        if uses == 0:
            deleteRanges.append(_lineRange(varStmt, src))
            deleteRanges.append(_lineRange(assignStmt, src))
            continue

        if (assignStmt.start_byte, assignStmt.end_byte) not in topLevel:
            continue  # the single assignment is in a nested block (e.g. a loop `continuing` block or
                      # an if branch); relocating the declaration there as a `let` would move it out
                      # of the scope where the variable is read. Keep naga's hoisted declaration.

        clean = _pickLocalName(name, wgslParams, taken)
        taken.add(clean)
        deleteRanges.append(_lineRange(varStmt, src))
        indent = _lineIndent(assignStmt, src)
        replacements.append((
            *_lineRange(assignStmt, src),
            f"{indent}let {clean} = {rhs};\n"))
        if name != clean:
            renames[name] = clean

    if not deleteRanges and not replacements:
        return fnText

    text = _applyEdits(fnText, deleteRanges, replacements)
    text = _applyRenames(text, renames)
    while True:
        cleaned = re.sub(r"\n[ \t]*\n[ \t]*\n", "\n\n", text)
        if cleaned == text:
            break
        text = cleaned
    return text


def inlineSingleUseTemps(body):
    '''Inline naga's single-use `let _eN = expr;` temporaries.'''
    changed = True
    while changed:
        changed = False
        for m in re.finditer(r"[ \t]*let\s+(_e\d+)\s*=\s*(.+?);\n", body):
            name, expr = m.group(1), m.group(2)
            if len(re.findall(r"\b" + re.escape(name) + r"\b", body)) - 1 == 1:
                repl = "(" + expr + ")" if re.search(r"[-+*/ ]", expr.strip()) else expr
                body = body[:m.start()] + body[m.end():]
                body = re.sub(r"\b" + re.escape(name) + r"\b", lambda _m: repl, body, count=1)
                changed = True
                break
    return body


def cleanupFunction(fnText, glslParamNames=None):
    '''Run readability cleanup on one transpiled WGSL function.'''
    try:
        # Inline naga temps first so param-mutate patterns like `N_25 = -(_e39)` become
        # `N_25 = -(N_25)` before tree-sitter param-shadow analysis runs.
        text = inlineSingleUseTemps(fnText)
        text = _collapseParamShadows(text, glslParamNames)
        while True:
            newText = _cleanupReadonlyLocals(text, glslParamNames)
            if newText == text:
                break
            text = newText
        text = _cleanupMutableLocals(text, glslParamNames)
        text = _unwrapRedundantCompounds(text)
        text = _flattenElseIfChains(text)
        text = _simplifyPtrParens(text)
        text = _unwrapAssignmentParens(text)
        text = re.sub(r"\}\s+else\b", "} else", text)
        text = re.sub(r"\n[ \t]+\n", "\n", text)
        while True:
            cleaned = re.sub(r"\n[ \t]*\n[ \t]*\n", "\n\n", text)
            if cleaned == text:
                break
            text = cleaned
    except Exception as exc:
        print(f"  WARN mxwgslcleanup: {exc}; keeping verbose naga output", file=sys.stderr)
        text = fnText
    return text
