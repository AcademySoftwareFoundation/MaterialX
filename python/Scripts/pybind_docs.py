import argparse
import re
import json
import xml.etree.ElementTree as ET
from pathlib import Path

# Adjust these paths as needed
DOXYGEN_XML_DIR = Path("build/documents/doxygen_xml")
PYBIND_DIR = Path("source/PyMaterialX")

# Extraction maps
class_docs = {}   # key: "MaterialX::Document" -> docstring
func_docs = {}    # key: "MaterialX::Document::addNodeDefFromGraph" -> dict {brief, detail, params:{name:desc}, returns, args_sig}

# ----------------------------
# Helper functions
# ----------------------------
def normalize_name(name: str) -> str:
    """Ensure name has MaterialX:: prefix if not already present."""
    if not name:
        return name
    return name if name.startswith("MaterialX::") else f"MaterialX::{name}"

def generate_lookup_keys(name: str, include_variants: bool = True) -> list:
    """Generate all possible lookup key variants for a given name."""
    if not name:
        return []
    
    keys = []
    # Add MaterialX:: variant
    normalized = normalize_name(name)
    if normalized != name:
        keys.append(normalized)
    keys.append(name)
    
    # Add mx:: to MaterialX:: conversion
    if name.startswith("mx::"):
        keys.insert(0, name.replace("mx::", "MaterialX::"))
    
    # Add class::method and method-only variants
    if include_variants and "::" in name:
        parts = name.split("::")
        if len(parts) >= 2:
            keys.append("::".join(parts[-2:]))  # Class::method
        keys.append(parts[-1])  # method only
    
    return keys

def extract_detail_text(parent_elem, exclude_tags={'parameterlist', 'simplesect'}):
    """Extract detail paragraphs from XML element, excluding specified child tags."""
    parts = []
    for para in parent_elem.findall("para"):
        if not any(para.find(tag) is not None for tag in exclude_tags):
            parts.append(_text_of(para))
    return "\n\n".join(p for p in parts if p)

def find_doc_by_suffix(func_docs: dict, *suffixes) -> dict:
    """Find function doc by suffix match. Returns first match or None."""
    for suffix in suffixes:
        if suffix:
            for key, value in func_docs.items():
                if key.endswith(f"::{suffix}"):
                    return value
    return None

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
            detail = extract_detail_text(compound.find("detaileddescription"))
            
            doc_parts = [p for p in [brief, detail] if p]
            full = "\n\n".join(doc_parts).strip()
            if full:
                class_docs[normalize_name(compoundname)] = full

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
            detail = extract_detail_text(member.find("detaileddescription"))

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
            argsstring = _text_of(member.find("argsstring"))
            args_sig = ""
            if argsstring:
                # Normalize argsstring
                args_sig = re.sub(r'\s*,\s*', ',', argsstring)
                args_sig = re.sub(r'\s+', ' ', args_sig).strip()

            # Store with normalized qualified name
            if qualified:
                primary_key = normalize_name(qualified)
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

def _has_docstring(args):
    """Check if any arg (after first) is a string literal, excluding py::arg calls."""
    for arg_text, _, _ in args[1:]:
        a = arg_text.strip()
        if a.startswith("py::arg"):
            continue
        if re.match(r'^".*"$', a):
            return True
    return False

def _find_matching_paren(s: str, start_idx: int) -> int:
    """Find the index of the matching ')' for '(' at start_idx.
    Handles nested parentheses and string literals."""
    depth = 0
    in_string = False
    escape_next = False
    i = start_idx
    
    while i < len(s):
        c = s[i]
        
        if escape_next:
            escape_next = False
            i += 1
            continue
        
        if c == '\\':
            escape_next = True
            i += 1
            continue
        
        if c == '"':
            in_string = not in_string
            i += 1
            continue
        
        if not in_string:
            if c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
                if depth == 0:
                    return i
        
        i += 1
    
    return -1


def _split_top_level_args(arglist: str):
    """Split arguments at top-level commas.
    Returns list of (arg_text, start_index, end_index) tuples.
    Handles nested parentheses and string literals."""
    args = []
    start = 0
    i = 0
    depth = 0
    in_string = False
    escape_next = False
    
    while i < len(arglist):
        c = arglist[i]
        
        if escape_next:
            escape_next = False
            i += 1
            continue
        
        if c == '\\':
            escape_next = True
            i += 1
            continue
        
        if c == '"':
            in_string = not in_string
            i += 1
            continue
        
        if not in_string:
            if c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
            elif c == ',' and depth == 0:
                # Top-level comma found
                args.append((arglist[start:i].strip(), start, i))
                start = i + 1
        
        i += 1
    
    # Append last argument
    if start < len(arglist):
        args.append((arglist[start:].strip(), start, len(arglist)))
    
    return args


def insert_docs_into_bindings(force_replace=False):
    """
    Robust insertion: 
    1. Insert class docstrings into py::class_<...>(mod, "Name") -> py::class_<...>(mod, "Name", "doc")
    2. Parse each .def(...) call, split top-level args, and insert docstring
       after the second argument (callable) if no docstring is present.
    
    Args:
        force_replace: If True, replace existing docstrings. If False, skip entries that already have docs.
    """
    if force_replace:
        print("Force replace mode: Will update existing docstrings")
    else:
        print("Normal mode: Will skip entries with existing docstrings")
    
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
                if not force_replace:
                    # Already has docstring, skip unless force_replace
                    continue
                else:
                    # Force replace: we'll need to replace the third arg
                    # Store as (start, end, new_doc) for replacement
                    pass  # Handle below
            
            # Extract class name from second argument
            class_name = args[1][0].strip().strip('"')
            
            # Find documentation using helper
            class_doc = None
            for k in generate_lookup_keys(class_name, include_variants=False):
                if k in class_docs:
                    class_doc = class_docs[k]
                    break
            
            if class_doc:
                escaped = escape_docstring_for_cpp(class_doc)
                if len(args) >= 3 and force_replace:
                    # Replace existing docstring (third argument) - just replace the quoted content
                    # Find the position of the third argument in the original text
                    arg3_start = paren_open + 1 + args[2][1]
                    arg3_end = paren_open + 1 + args[2][2]
                    original_arg = text[arg3_start:arg3_end]
                    
                    # Find the opening and closing quotes
                    first_quote = original_arg.find('"')
                    last_quote = original_arg.rfind('"')
                    
                    if first_quote != -1 and last_quote != -1 and first_quote < last_quote:
                        # Replace only the content between quotes, preserving everything else
                        new_arg = original_arg[:first_quote+1] + escaped + original_arg[last_quote:]
                        class_matches.append((arg3_start, arg3_end, new_arg, True))
                    else:
                        # Fallback: replace entire argument
                        class_matches.append((arg3_start, arg3_end, f'"{escaped}"', True))
                else:
                    # Insert new docstring (with space after comma to match original style)
                    class_matches.append((paren_close, f', "{escaped}"', False))  # False = insert
                class_changed = True
        
        # Apply class documentation changes in reverse order to preserve positions
        if class_changed:
            for match in reversed(class_matches):
                if len(match) == 4:  # Replace mode: (start, end, new_text, is_replace)
                    start, end, new_text, _ = match
                    text = text[:start] + new_text + text[end:]
                else:  # Insert mode: (pos, insertion, is_replace)
                    pos, insertion, _ = match
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
            
            if not has_docstring or force_replace:
                # Extract Python method name and C++ callable reference
                py_method_name = first_arg_text.strip().strip('"')
                cpp_ref_clean = second_arg_text.strip()
                
                # Extract the callable name (works for both regular functions and lambdas)
                # For lambdas: try to find method calls like elem.method( or obj->method(
                # For regular callables: extract the qualified name like &mx::Class::method
                callable_name = None
                
                # Check for method call pattern (handles lambdas and some edge cases)
                method_call = re.search(r'[\.\->](\w+)\s*\(', cpp_ref_clean)
                if method_call:
                    callable_name = method_call.group(1)
                else:
                    # Extract qualified name from regular callable reference
                    # Find last token containing ::
                    tokens = re.split(r'\s+', cpp_ref_clean)
                    for token in reversed(tokens):
                        if '::' in token:
                            callable_name = token.rstrip(',').strip()
                            break
                    if not callable_name and tokens:
                        callable_name = tokens[-1].rstrip(',').strip()
                
                # Generate lookup keys
                lookup_keys = generate_lookup_keys(callable_name if callable_name else py_method_name)
                # Also try with Python method name if different
                if callable_name != py_method_name:
                    lookup_keys.extend(generate_lookup_keys(py_method_name))

                # Find documentation
                func_entry = None
                for k in lookup_keys:
                    if k in func_docs:
                        func_entry = func_docs[k]
                        break
                
                # Fallback: suffix match
                if not func_entry:
                    func_entry = find_doc_by_suffix(func_docs, callable_name, py_method_name)

                if func_entry:
                    docstring = build_method_docstring(func_entry)
                    if docstring:
                        escaped = escape_docstring_for_cpp(docstring)
                        
                        if has_docstring and force_replace:
                            # Find and replace the existing docstring argument
                            # Find which argument is the docstring (first string literal after callable)
                            doc_arg_idx = None
                            for i, (arg_text, _, _) in enumerate(args[2:], start=2):  # Start from 3rd arg
                                a = arg_text.strip()
                                if not a.startswith("py::arg") and re.match(r'^".*"$', a):
                                    doc_arg_idx = i
                                    break
                            
                            if doc_arg_idx is not None:
                                # Replace only the quoted content, preserving all formatting
                                arg_start = paren_open + 1 + args[doc_arg_idx][1]
                                arg_end = paren_open + 1 + args[doc_arg_idx][2]
                                original_arg = text[arg_start:arg_end]
                                
                                # Find the opening and closing quotes
                                first_quote = original_arg.find('"')
                                last_quote = original_arg.rfind('"')
                                
                                if first_quote != -1 and last_quote != -1 and first_quote < last_quote:
                                    # Replace only content between quotes, preserving everything else
                                    new_arg = original_arg[:first_quote+1] + escaped + original_arg[last_quote:]
                                    # Copy everything before the docstring, insert new docstring, copy everything after
                                    new_def_text = text[start:arg_start] + new_arg + text[arg_end:paren_close+1]
                                    new_text_parts.append(new_def_text)
                                    idx = paren_close + 1
                                    changed = True
                                    continue
                        
                        # Insert new docstring at the end
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
    parser.add_argument("-j", "--write_json", action='store_true', 
                        help="Write extracted docs to JSON file.")
    parser.add_argument("-f", "--force", action='store_true',
                        help="Force replace existing docstrings. By default, existing docstrings are preserved.")
    
    args = parser.parse_args()
    
    # Validate paths
    if not args.doxygen_xml_dir.exists():
        print(f"Error: Doxygen XML directory does not exist: {args.doxygen_xml_dir}")
        return
    if not args.pybind_dir.exists():
        print(f"Error: Pybind directory does not exist: {args.pybind_dir}")
        return
    
    # Set global paths (needed by extraction/insertion functions)
    global DOXYGEN_XML_DIR, PYBIND_DIR
    DOXYGEN_XML_DIR = args.doxygen_xml_dir
    PYBIND_DIR = args.pybind_dir

    # Build documentation maps
    extract_docs_from_xml()

    # Update CPP files
    insert_docs_into_bindings(force_replace=args.force)

    # Write extracted documentation to JSON files
    if args.write_json:
        class_json = Path("class_docs.json")
        with class_json.open("w", encoding="utf-8") as f:
            print(f"Writing class docs to {class_json}")
            json.dump(class_docs, f, indent=2)
        
        func_json = Path("func_docs.json")
        with func_json.open("w", encoding="utf-8") as f:
            print(f"Writing function docs to {func_json}")
            json.dump(func_docs, f, indent=2)

    print("Done.")

if __name__ == "__main__":
    main()
