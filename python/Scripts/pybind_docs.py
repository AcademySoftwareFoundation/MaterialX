import argparse
import re
import json
import xml.etree.ElementTree as ET
from pathlib import Path

# Adjust these paths as needed
DOXYGEN_XML_DIR = Path("build/documents/doxygen_xml")
PYBIND_DIR = Path("source/PyMaterialX")

# Output debug dump of extracted docs
DEBUG_JSON = Path("doc_map.json")

class pybind_doc_builder:
    def __init__(self, doxygen_xml_dir: Path, pybind_dir: Path):
        self.doxygen_xml_dir = doxygen_xml_dir
        self.pybind_dir = pybind_dir
        self.class_docs = {}
        self.func_docs = {}

# Extraction maps
class_docs = {}   # key: "MaterialX::Document" -> docstring
func_docs = {}    # key: "MaterialX::Document::addNodeDefFromGraph" -> dict {brief, detail, params:{name:desc}, returns, args_sig}

def _text_of(elem):
    """Concatenate element text and children text, normalized whitespace."""
    if elem is None:
        return ""
    text = "".join(elem.itertext())
    text = re.sub(r'\s+', ' ', text).strip()
    return text

def _param_desc_tuple(param_item):
    """Return (name, description) for a <parameteritem> element."""
    name = _text_of(param_item.find("parameternamelist/parametername"))
    desc_elem = param_item.find("parameterdescription")
    desc = _text_of(desc_elem)
    return name, desc

def extract_docs_from_xml():
    """
    Populate class_docs and func_docs.
    """
    if not DOXYGEN_XML_DIR.exists():
        raise FileNotFoundError(f"Doxygen XML directory not found: {DOXYGEN_XML_DIR}")

    # Iterate over xml files
    for xml_file in DOXYGEN_XML_DIR.glob("*.xml"):
        tree = ET.parse(xml_file)
        root = tree.getroot()

        # Extract class/struct compound docs
        for compound in root.findall(".//compounddef[@kind='class']") + root.findall(".//compounddef[@kind='struct']"):
            compoundname = _text_of(compound.find("compoundname"))
            brief = _text_of(compound.find("briefdescription/para"))
            # collect detailed paragraphs excluding parameterlist/simplesect which are typically for members
            detail_parts = []
            for para in compound.findall("detaileddescription/para"):
                if para.find("parameterlist") is None and para.find("simplesect") is None:
                    detail_parts.append(_text_of(para))
            detail = "\n\n".join(p for p in detail_parts if p)
            doc_parts = []
            if brief:
                doc_parts.append(brief)
            if detail:
                doc_parts.append(detail)
            full = "\n\n".join(doc_parts).strip()
            if full:
                # normalize name to MaterialX::... if not already
                key = compoundname
                if key and not key.startswith("MaterialX::"):
                    key = "MaterialX::" + key
                class_docs[key] = full

        # Extract member functions
        for member in root.findall(".//memberdef[@kind='function']"):
            name = _text_of(member.find("name"))
            qualified = _text_of(member.find("qualifiedname"))

            # try to build qualified if missing using parent compoundname
            if not qualified and name:
                compound = member.find("../../compoundname")
                compound_name = _text_of(compound) if compound is not None else ""
                if compound_name:
                    qualified = compound_name + "::" + name

            # brief + detailed
            brief = _text_of(member.find("briefdescription/para"))
            detail_parts = []
            for para in member.findall("detaileddescription/para"):
                # skip param lists and return sections here
                if para.find("parameterlist") is not None or para.find("simplesect") is not None:
                    continue
                detail_parts.append(_text_of(para))
            detail = "\n\n".join(p for p in detail_parts if p)

            # params
            params = {}
            for param_item in member.findall(".//parameterlist[@kind='param']/parameteritem"):
                pname, pdesc = _param_desc_tuple(param_item)
                if pname:
                    params[pname] = pdesc

            # returns
            ret_text = ""
            retsect = member.find(".//simplesect[@kind='return']")
            if retsect is not None:
                ret_text = _text_of(retsect)

            # argsstring for overload disambiguation
            argsstring = _text_of(member.find("argsstring"))  # e.g. "(NodeGraphPtr nodeGraph, const string &nodeDefName, ...)"
            # canonicalize argsstring: remove repeated spaces and normalize commas
            args_sig = ""
            if argsstring:
                args_sig = re.sub(r'\s*,\s*', ',', argsstring)
                args_sig = re.sub(r'\s+', ' ', args_sig).strip()

            # normalize qualified name to MaterialX:: prefix variants
            q1 = qualified
            if q1 and not q1.startswith("MaterialX::"):
                q_mat = "MaterialX::" + q1
            else:
                q_mat = q1

            # choose a primary key for func_docs as the fully qualified name (prefer MaterialX::... form)
            primary_key = q_mat if q_mat else q1
            if not primary_key:
                continue

            func_docs[primary_key] = {
                "brief": brief,
                "detail": detail,
                "params": params,
                "returns": ret_text,
                "args_sig": args_sig
            }

    print(f"Extracted {len(class_docs)} classes and {len(func_docs)} functions.")

# ----------------------------
# Insertion code
# ----------------------------
def escape_docstring_for_cpp(s: str) -> str:
    # escape double quotes and backslashes, then represent newlines as \n in a C++ string literal
    # keep it as a single-line C++ string literal with explicit \n sequences
    if s is None:
        return ""
    s = s.replace("\\", "\\\\").replace('"', '\\"')
    s = s.replace("\r\n", "\n").replace("\r", "\n")
    s = s.replace("\n", "\\n")
    return s

def build_method_docstring(func_entry):
    """Build a readable docstring for a method from func_entry dict."""
    parts = []
    if func_entry.get("brief"):
        parts.append(func_entry["brief"])
    if func_entry.get("detail"):
        parts.append(func_entry["detail"])
    # Args block
    params = func_entry.get("params", {})
    if params:
        param_lines = []
        for pname, pdesc in params.items():
            if pdesc:
                param_lines.append(f"    {pname}: {pdesc}")
            else:
                param_lines.append(f"    {pname}:")
        parts.append("Args:\n" + "\n".join(param_lines))
    # Returns
    if func_entry.get("returns"):
        parts.append("Returns:\n    " + func_entry["returns"])
    return "\n\n".join(parts).strip()

def insert_docs_into_bindings_old():
    """
    Modify C++ pybind files in place:
    - Insert class docstrings into the py::class_<...>(mod, "Name", "doc")
    - Insert method docstrings into .def("name", &mx::Class::name, "doc")
    """
    cpp_files = list(PYBIND_DIR.rglob("*.cpp"))

    # Regex patterns
    # py::class_<...>(mod, "Document")
    class_pattern = re.compile(r'(py::class_<[^>]+>\(\s*([A-Za-z0-9_:]+)\s*,\s*"([^"]+)"\s*)(\))')
    # Then a variant where there are spaces/newline before closing paren: handle simpler by searching for 'py::class_<...' up to the first ')' on the same line.

    # .def("name", &mx::Document::name, ...)
    def_pattern = re.compile(r'\.def\(\s*"([^"]+)"\s*,\s*&([A-Za-z0-9_:]+)\s*([,)]?)')

    for cpp in cpp_files:
        text = cpp.read_text(encoding="utf-8")
        orig_text = text

        lines = text.splitlines()
        new_lines = []
        changed = False

        for i, line in enumerate(lines):
            # First: try class insertion when a py::class_ is declared
            # We look for the pattern: py::class_<...>(mod, "Name")
            class_match = re.search(r'py::class_<[^>]+>\(\s*([A-Za-z0-9_:]+)\s*,\s*"([^"]+)"\s*\)', line)
            if class_match:
                cpp_class_type = class_match.group(1)  # maybe mx::Document, etc
                class_name = class_match.group(2)      # "Document"
                # Normalize to MaterialX::ClassName
                # We try keys: MaterialX::Document, Document, mx::Document
                keys = []
                if cpp_class_type:
                    # if cpp_class_type includes 'mx::', remove and use just the class name
                    short = class_name
                    keys.append(f"MaterialX::{short}")
                    keys.append(short)
                    if cpp_class_type.startswith("mx::"):
                        keys.append(cpp_class_type.replace("mx::", "MaterialX::"))
                    else:
                        keys.append(cpp_class_type)
                else:
                    keys.append(class_name)
                    keys.append(f"MaterialX::{class_name}")

                class_doc = None
                for k in keys:
                    if k in class_docs:
                        class_doc = class_docs[k]
                        break

                if class_doc:
                    # If the line already has a third argument (docstring) skip
                    # Simple check: count commas inside the parentheses - but safer to check presence of a string literal after the class name
                    if re.search(r'py::class_<[^>]+>\(\s*[A-Za-z0-9_:]+\s*,\s*"' + re.escape(class_name) + r'"\s*,', line):
                        # already has doc - do nothing
                        pass
                    else:
                        escaped = escape_docstring_for_cpp(class_doc)
                        # Insert as third argument before the closing paren
                        new_line = re.sub(r'\)\s*$', f', "{escaped}")', line)
                        new_lines.append(new_line)
                        changed = True
                        continue  # skip default append at end
            # Next: method .def insertion
            m = def_pattern.search(line)
            if m:
                py_name = m.group(1)  # "createInput"
                cpp_ref = m.group(2)  # mx::Document::addNodeDefFromGraph or Document::...
                # Normalize cpp_ref to MaterialX::... form to lookup func_docs
                lookup_keys = []
                if cpp_ref.startswith("mx::"):
                    lookup_keys.append(cpp_ref.replace("mx::", "MaterialX::"))
                lookup_keys.append(cpp_ref)
                # Also try without namespace
                if "::" in cpp_ref:
                    parts = cpp_ref.split("::")
                    lookup_keys.append("::".join(parts[-2:]))  # Class::method
                    lookup_keys.append(parts[-1])  # method only

                func_entry = None
                for k in lookup_keys:
                    if k in func_docs:
                        func_entry = func_docs[k]
                        break

                # If not found, try suffix match (sometimes args_sig stored)
                if not func_entry:
                    for k, v in func_docs.items():
                        # match just by 'Class::method' ending part
                        if k.endswith("::" + py_name):
                            func_entry = v
                            break

                if func_entry:
                    # Check if this .def already has a docstring (a string literal after the callable)
                    # We'll consider the remainder of the line after the callable
                    rest = line[m.end():].strip()
                    already_has_doc = False
                    # crude check: does a string literal appear before the closing ')'
                    if '"' in rest or "R\"" in rest:
                        already_has_doc = True

                    if not already_has_doc:
                        docstring = build_method_docstring(func_entry)
                        if docstring:
                            escaped = escape_docstring_for_cpp(docstring)
                            # We must be careful to not break existing trailing arguments (py::arg(...))
                            # We'll try to insert the docstring after the callable reference and before other args.
                            # If line ends with ')' we can simply replace the trailing ')' with , "doc")
                            if line.rstrip().endswith(")"):
                                new_line = line.rstrip()
                                # but avoid adding doc into lines that are multi-line function chains; this is a best-effort inline insertion
                                new_line = new_line.rstrip(")")
                                new_line = new_line + f', "{escaped}")'
                                new_lines.append(new_line)
                                changed = True
                                continue
                            else:
                                # line doesn't end with ')', likely args continue on following lines; just add doc at line end for now
                                new_line = line + f', "{escaped}"'
                                new_lines.append(new_line)
                                changed = True
                                continue

            # default: copy original line
            new_lines.append(line)

        # if changed, write back
        if changed:
            new_text = "\n".join(new_lines)
            cpp.write_text(new_text, encoding="utf-8")
            #print("new text", new_text)
            print(f"Patched: {cpp}")

    print("Insertion complete.")

# Replace the previous insert_docs_into_bindings() with this more robust version.

def _find_matching_paren(s: str, start_idx: int) -> int:
    """Find the index of the matching ')' for s[start_idx] == '('.
    Handles nested parentheses and string literals roughly (so quotes inside strings are ignored)."""
    i = start_idx
    depth = 0
    in_single = False
    in_double = False
    in_raw = False
    raw_delim = None
    L = len(s)
    while i < L:
        c = s[i]
        # handle entering/exiting raw string literal R"delim(... )delim"
        if not in_single and not in_double:
            if s.startswith('R"', i):
                # find raw delim start: R"delim(
                # grab delim between R" and (
                m = re.match(r'R"([^\(\s]*)\(', s[i:])
                if m:
                    in_raw = True
                    raw_delim = m.group(1)
                    i += 2 + len(raw_delim)  # position at '(' after delim
                    depth += 1
                    i += 1
                    continue

        if in_raw:
            # raw literal ends with )delim"
            # check for )raw_delim"
            end_token = ')' + (raw_delim or '') + '"'
            if s.startswith(end_token, i):
                in_raw = False
                i += len(end_token)
                continue
            else:
                i += 1
                continue

        if c == '"' and not in_single:
            # toggle double quotes (but ignore escaped quotes)
            # ensure not escaped
            prev = s[i-1] if i > 0 else ''
            if prev != '\\':
                in_double = not in_double
            i += 1
            continue
        if c == "'" and not in_double:
            prev = s[i-1] if i > 0 else ''
            if prev != '\\':
                in_single = not in_single
            i += 1
            continue

        if in_single or in_double:
            i += 1
            continue

        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def _split_top_level_args(arglist: str):
    """Split a string of arguments (content inside parentheses) into a list of top-level args.
    Returns list of (arg_text, start_index, end_index) relative to the arglist string.
    Handles nested parentheses and string literals so commas inside those are ignored.
    """
    args = []
    start = 0
    i = 0
    depth = 0
    in_single = False
    in_double = False
    in_raw = False
    raw_delim = None
    L = len(arglist)
    while i < L:
        c = arglist[i]
        # raw string handling
        if not in_single and not in_double:
            if arglist.startswith('R"', i):
                m = re.match(r'R"([^\(\s]*)\(', arglist[i:])
                if m:
                    in_raw = True
                    raw_delim = m.group(1)
                    i += 2 + len(raw_delim)  # move to '('
                    depth += 1
                    i += 1
                    continue

        if in_raw:
            end_token = ')' + (raw_delim or '') + '"'
            if arglist.startswith(end_token, i):
                in_raw = False
                i += len(end_token)
                continue
            else:
                i += 1
                continue

        if c == '"' and not in_single:
            prev = arglist[i-1] if i > 0 else ''
            if prev != '\\':
                in_double = not in_double
            i += 1
            continue
        if c == "'" and not in_double:
            prev = arglist[i-1] if i > 0 else ''
            if prev != '\\':
                in_single = not in_single
            i += 1
            continue

        if in_single or in_double:
            i += 1
            continue

        if c == '(':
            depth += 1
        elif c == ')':
            if depth > 0:
                depth -= 1
        elif c == ',' and depth == 0:
            # top-level comma
            args.append((arglist[start:i].strip(), start, i))
            start = i + 1
        i += 1

    # append last arg
    if start < L:
        args.append((arglist[start:].strip(), start, L))
    return args

def _has_docstring(args):
    """
    True if there is a string literal among top-level args that is not a py::arg(...)
    """
    for a, _, _ in args[1:]:  # skip first arg (Python name)
        # strip whitespace
        a_stripped = a.strip()
        # skip py::arg(...) calls
        if a_stripped.startswith("py::arg"):
            continue
        # crude string literal check
        if re.match(r'^R?".*"$', a_stripped):
            return True
    return False


def insert_docs_into_bindings():
    """
    Robust insertion: 
    1. Insert class docstrings into py::class_<...>(mod, "Name") -> py::class_<...>(mod, "Name", "doc")
    2. Parse each .def(...) call, split top-level args, and insert docstring
       after the second argument (callable) if no docstring is present.
    """
    cpp_files = list(PYBIND_DIR.rglob("*.cpp"))
    def_start_re = re.compile(r'\.def\s*\(')
    class_start_re = re.compile(r'py::class_<')

    for cpp in cpp_files:
        text = cpp.read_text(encoding="utf-8")
        
        # First pass: insert class documentation
        # Look for py::class_<...>(mod, "Name") and add docstring as third parameter
        class_changed = False
        class_matches = []
        for m in class_start_re.finditer(text):
            start = m.start()
            # Find the opening paren after py::class_<...>
            # First find the closing > of the template
            template_start = start + len('py::class_<')
            depth = 1
            i = template_start
            while i < len(text) and depth > 0:
                if text[i] == '<':
                    depth += 1
                elif text[i] == '>':
                    depth -= 1
                i += 1
            # Now find the opening paren
            paren_open = text.find('(', i)
            if paren_open == -1:
                continue
            paren_close = _find_matching_paren(text, paren_open)
            if paren_close == -1:
                continue
            
            # Extract arguments
            arglist = text[paren_open+1:paren_close]
            args = _split_top_level_args(arglist)
            
            # We expect at least 2 args: (mod, "ClassName")
            if len(args) < 2:
                continue
            
            # Check if there's already a third argument (docstring)
            if len(args) >= 3:
                # Already has docstring
                continue
            
            # Extract class name from second argument
            class_name = args[1][0].strip().strip('"')
            
            # Try to find documentation
            # Build lookup keys
            lookup_keys = [
                f"MaterialX::{class_name}",
                class_name
            ]
            
            class_doc = None
            for k in lookup_keys:
                if k in class_docs:
                    class_doc = class_docs[k]
                    break
            
            if class_doc:
                escaped = escape_docstring_for_cpp(class_doc)
                class_matches.append((paren_close, f', "{escaped}"'))
                class_changed = True
        
        # Apply class documentation insertions in reverse order to preserve positions
        if class_changed:
            for pos, insertion in reversed(class_matches):
                text = text[:pos] + insertion + text[pos:]
        
        # Second pass: insert method documentation
        new_text_parts = []
        idx = 0
        changed = False
        while True:
            m = def_start_re.search(text, idx)
            if not m:
                # append rest and break
                new_text_parts.append(text[idx:])
                break

            start = m.start()
            new_text_parts.append(text[idx:start])  # content up to .def(
            paren_open = text.find('(', start)
            if paren_open == -1:
                # shouldn't happen; append rest and break
                new_text_parts.append(text[start:])
                break

            paren_close = _find_matching_paren(text, paren_open)
            if paren_close == -1:
                # unmatched, append rest
                new_text_parts.append(text[start:])
                break

            # argument list content (without outer parentheses)
            arglist = text[paren_open+1:paren_close]
            args = _split_top_level_args(arglist)

            # if we have less than 2 args, we can't determine callable; just copy as-is
            if len(args) < 2:
                new_text_parts.append(text[start:paren_close+1])
                idx = paren_close + 1
                continue

            # first arg is typically the python name string literal (e.g. "addNode")
            # second arg is the callable, e.g. &mx::Document::addNodeGraph
            first_arg_text = args[0][0]
            second_arg_text = args[1][0]

            # determine if any existing argument is a string literal (treat raw string R"..." too)
            has_docstring = _has_docstring(args)
            # A simpler check: presence of an unqualified string literal among top-level args other than the first (name) arg
            if not has_docstring:
                # build lookup keys using the second arg (callable)
                cpp_ref = second_arg_text
                # strip & and possible std::function wrappers, templates, whitespace
                cpp_ref_clean = cpp_ref.strip()
                
                # Check if this is a lambda function (starts with '[')
                is_lambda = cpp_ref_clean.startswith('[')
                
                if is_lambda:
                    # For lambda functions, try to infer the method name from the Python method name
                    # and look for it being called within the lambda
                    py_method_name = first_arg_text.strip().strip('"')
                    lookup_keys = []
                    
                    # Try to find method calls within the lambda body
                    # Look for patterns like elem.method( or obj->method(
                    method_call_pattern = re.search(r'[\.\->](\w+)\s*\(', cpp_ref_clean)
                    if method_call_pattern:
                        called_method = method_call_pattern.group(1)
                        # Try to match with various namespace prefixes
                        lookup_keys.append(f"MaterialX::Element::{called_method}")
                        lookup_keys.append(f"MaterialX::{called_method}")
                        lookup_keys.append(called_method)
                    
                    # Also try using the Python method name directly
                    lookup_keys.append(f"MaterialX::Element::{py_method_name}")
                    lookup_keys.append(f"MaterialX::{py_method_name}")
                    lookup_keys.append(py_method_name)
                    
                    possible = py_method_name
                else:
                    # remove address-of operator and potential casts like (py::cpp_function) &foo  — try to extract last token with ::method
                    # crude heuristic: find last token containing :: and use everything from that token to end (remove trailing spaces)
                    tokens = re.split(r'\s+', cpp_ref_clean)
                    possible = None
                    for t in reversed(tokens):
                        if '::' in t:
                            possible = t
                            break
                    if not possible:
                        possible = tokens[-1]

                    # strip trailing commas/parens etc
                    possible = possible.rstrip(',').strip()

                    # normalize to lookups used earlier
                    lookup_keys = []
                    if possible.startswith("mx::"):
                        lookup_keys.append(possible.replace("mx::", "MaterialX::"))
                    lookup_keys.append(possible)
                    if "::" in possible:
                        parts = possible.split("::")
                        lookup_keys.append("::".join(parts[-2:]))  # Class::method
                        lookup_keys.append(parts[-1])  # method only

                func_entry = None
                for k in lookup_keys:
                    if k in func_docs:
                        func_entry = func_docs[k]
                        break
                if not func_entry:
                    # fallback: suffix match
                    for k, v in func_docs.items():
                        if k.endswith("::" + args[0][0].strip().strip('"')) or k.endswith("::" + possible.split("::")[-1]):
                            func_entry = v
                            break

                if func_entry:
                    docstring = build_method_docstring(func_entry)
                    if docstring:
                        escaped = escape_docstring_for_cpp(docstring)
                        # Simple approach: insert ", "docstring"" right before the closing )
                        # This preserves all the original formatting and structure
                        new_def_text = text[start:paren_close] + f', "{escaped}")' 
                        new_text_parts.append(new_def_text)
                        idx = paren_close + 1
                        changed = True
                        continue

            # no insertion performed — copy original .def(...) exactly
            new_text_parts.append(text[start:paren_close+1])
            idx = paren_close + 1

        if changed or class_changed:
            new_text = "".join(new_text_parts)
            cpp.write_text(new_text, encoding="utf-8")
            print(f"- Patched: {cpp}")
        else:
            # no changes; nothing to write
            print('- No changes needed for:', cpp)
            pass

    print("Code insertion complete.")


def main():
    parser = argparse.ArgumentParser(description="Extract Doxygen XML docs and insert into pybind11 bindings.")
    parser.add_argument("-d", "--doxygen_xml_dir", type=Path, default=Path("build/documents/doxygen_xml"),
                        help="Path to Doxygen XML output directory.")
    parser.add_argument("-p", "--pybind_dir", type=Path, default=Path("source/PyMaterialX"),
                        help="Path to pybind11 C++ bindings directory.")
    parser.add_argument("-j", "--write_json", action='store_true', help="Write extracted docs to JSON file.")
    
    args = parser.parse_args()
    doxygen_xml_dir = args.doxygen_xml_dir
    pybind_dir = args.pybind_dir
    if not doxygen_xml_dir.exists():
        print(f"Error: Doxygen XML directory does not exist: {doxygen-xml-dir}")
        return
    if not pybind_dir.exists():
        print(f"Error: Pybind directory does not exist: {pybind-dir}")
        return
    
    DOXYGEN_XML_DIR = doxygen_xml_dir
    PYBIND_DIR = pybind_dir

    # Build documentation maps
    extract_docs_from_xml()

    # Update CPP files
    insert_docs_into_bindings()

    # Write extracted documentation to JSON files.    
    write_json = args.write_json if args.write_json else False
    if write_json:
        json_path = Path("class_docs.json")
        with json_path.open("w", encoding="utf-8") as f:
            print("Writing class docs to", json_path)
            json.dump(class_docs, f, indent=2)
        json_path = Path("func_docs.json")
        with json_path.open("w", encoding="utf-8") as f:
            print("Writing function docs to", json_path)
            json.dump(func_docs, f, indent=2)

    print("Done.")

if __name__ == "__main__":
    main()
