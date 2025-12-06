#!/usr/bin/env python
"""
pybind11 documentation insertion tool.

Extracts documentation from Doxygen XML and inserts it into pybind11 bindings
using  string matching via signature lookup table.

Logic:
- Builds a multi-key lookup for all functions (MaterialX::, mx::, Class::method, method)
- Handles free functions without <qualifiedname> by assuming MaterialX namespace
- Adds class context tracking to correctly document lambda-based bindings
- Supports .def(...) and .def_static(...); skips .def_readonly_static(...)
"""

import argparse
import re
import json
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Dict, Optional

# Defaults (can be overridden by CLI)
DOXYGEN_XML_DIR = Path("build/documents/doxygen_xml")
PYBIND_DIR = Path("source/PyMaterialX")


class DocExtractor:
    """Extracts documentation from Doxygen XML files and builds a lookup table."""

    def __init__(self, xml_dir: Path):
        self.xml_dir = xml_dir
        self.class_docs: Dict[str, str] = {}
        self.func_docs: Dict[str, Dict] = {}
        # Multi-key lookup: all name variants point to the same doc
        self.func_lookup: Dict[str, Dict] = {}

    def extract(self):
        if not self.xml_dir.exists():
            raise FileNotFoundError(f"Doxygen XML directory not found: {self.xml_dir}")

        for xml_file in self.xml_dir.glob("*.xml"):
            self._process_xml_file(xml_file)

        self._build_lookup_table()
        print(f"Extracted {len(self.class_docs)} classes and {len(self.func_docs)} functions")
        print(f"Built lookup table with {len(self.func_lookup)} keys")

    def _process_xml_file(self, xml_file: Path):
        tree = ET.parse(xml_file)
        root = tree.getroot()

        # Class / struct documentation
        for compound in root.findall(".//compounddef[@kind='class']") + root.findall(".//compounddef[@kind='struct']"):
            self._extract_class_doc(compound)

        # Function documentation
        for member in root.findall(".//memberdef[@kind='function']"):
            self._extract_func_doc(member)

    def _extract_class_doc(self, compound):
        name = self._get_text(compound.find("compoundname"))
        brief = self._get_text(compound.find("briefdescription/para"))
        detail = self._extract_detail(compound.find("detaileddescription"))
        doc = "\n\n".join(filter(None, [brief, detail]))
        if doc:
            normalized = self._normalize_name(name)
            self.class_docs[normalized] = doc

    def _extract_func_doc(self, member):
        name = self._get_text(member.find("name"))
        qualified = self._get_text(member.find("qualifiedname"))

        # Many free functions have no <qualifiedname>; use the bare name
        # and normalize to MaterialX::name so lookups can resolve.
        if not qualified and name:
            qualified = name

        if not qualified:
            return

        brief = self._get_text(member.find("briefdescription/para"))
        detail = self._extract_detail(member.find("detaileddescription"))
        params = self._extract_params(member)
        returns = self._get_text(member.find(".//simplesect[@kind='return']"))

        normalized = self._normalize_name(qualified)
        self.func_docs[normalized] = {
            "brief": brief,
            "detail": detail,
            "params": params,
            "returns": returns,
        }

    def _build_lookup_table(self):
        for qualified_name, doc in self.func_docs.items():
            for variant in self._generate_name_variants(qualified_name):
                if variant not in self.func_lookup:
                    self.func_lookup[variant] = doc

    def _generate_name_variants(self, qualified_name: str) -> list:
        variants = [qualified_name]
        parts = qualified_name.split("::")
        # Class::method
        if len(parts) >= 2:
            variants.append("::".join(parts[-2:]))
        # method
        if len(parts) >= 1:
            variants.append(parts[-1])
        # mx:: variant if MaterialX::
        if qualified_name.startswith("MaterialX::"):
            mx_variant = qualified_name.replace("MaterialX::", "mx::", 1)
            variants.append(mx_variant)
            if len(parts) >= 3:
                variants.append(f"mx::{parts[-2]}::{parts[-1]}")
        return variants

    def _normalize_name(self, name: str) -> str:
        if not name:
            return name
        return name if name.startswith("MaterialX::") else f"MaterialX::{name}"

    def _get_text(self, elem) -> str:
        if elem is None:
            return ""
        text = "".join(elem.itertext())
        return re.sub(r"\s+", " ", text).strip()

    def _extract_detail(self, elem, exclude_tags={"parameterlist", "simplesect"}) -> str:
        if elem is None:
            return ""
        parts = []
        for para in elem.findall("para"):
            if not any(para.find(tag) is not None for tag in exclude_tags):
                t = self._get_text(para)
                if t:
                    parts.append(t)
        return "\n\n".join(parts)

    def _extract_params(self, member) -> Dict[str, str]:
        params = {}
        for param_item in member.findall(".//parameterlist[@kind='param']/parameteritem"):
            name = self._get_text(param_item.find("parameternamelist/parametername"))
            desc = self._get_text(param_item.find("parameterdescription"))
            if name:
                params[name] = desc
        return params


class DocInserter:
    """Inserts documentation into pybind11 binding files."""

    def __init__(self, extractor: DocExtractor, pybind_dir: Path, force_replace: bool = False):
        self.extractor = extractor
        self.pybind_dir = pybind_dir
        self.force_replace = force_replace

        self.class_pattern = re.compile(r"py::class_<")
        self.def_pattern = re.compile(r"\.def(?:_static)?\s*\(")
        # Match .def and .def_static; skip .def_readonly_static (constants)
        self.def_pattern = re.compile(r"\.def(?:_static)?\s*\(")
        self.skip_pattern = re.compile(r"\.def_readonly_static\s*\(")

    def process_all_files(self):
        cpp_files = list(self.pybind_dir.rglob("*.cpp"))
        patched = 0
        for cpp_file in cpp_files:
            if self._process_file(cpp_file):
                patched += 1
        print(f"\nProcessed {len(cpp_files)} files, patched {patched}")

    def _process_file(self, cpp_file: Path) -> bool:
        content = cpp_file.read_text(encoding="utf-8")
        original = content

        content = self._insert_class_docs(content)
        content = self._insert_method_docs(content)

        if content != original:
            cpp_file.write_text(content, encoding="utf-8")
            print(f"  - {cpp_file.relative_to(self.pybind_dir.parent)}")
            return True
        else:
            print(f"  - {cpp_file.relative_to(self.pybind_dir.parent)}")
            return False

    def _insert_class_docs(self, content: str) -> str:
        result = []
        pos = 0

        for match in self.class_pattern.finditer(content):
            result.append(content[pos:match.start()])

            start = match.start()
            template_end = self._find_template_end(content, start)
            if template_end == -1:
                result.append(content[start:match.end()])
                pos = match.end()
                continue

            paren_start = content.find('(', template_end)
            if paren_start == -1:
                result.append(content[start:match.end()])
                pos = match.end()
                continue

            paren_end = self._find_matching_paren(content, paren_start)
            if paren_end == -1:
                result.append(content[start:match.end()])
                pos = match.end()
                continue

            args_text = content[paren_start + 1:paren_end]
            class_name = self._extract_class_name(args_text)

            if class_name:
                doc = self.extractor.class_docs.get(self.extractor._normalize_name(class_name))
                if doc:
                    args = self._split_args(args_text)
                    if len(args) >= 3 and not self.force_replace:
                        result.append(content[start:paren_end + 1])
                        pos = paren_end + 1
                        continue

                    escaped = self._escape_for_cpp(doc)
                    if len(args) >= 3 and self.force_replace:
                        new_args = args[:2] + [f'"{escaped}"'] + args[3:]
                        result.append(content[start:paren_start + 1])
                        result.append(", ".join(new_args))
                        result.append(")")
                    else:
                        result.append(content[start:paren_end])
                        result.append(f', "{escaped}")')
                    pos = paren_end + 1
                    continue

            result.append(content[start:paren_end + 1])
            pos = paren_end + 1

        result.append(content[pos:])
        return "".join(result)

    def _insert_method_docs(self, content: str) -> str:
        # Build a map of line numbers to class contexts
        class_contexts = self._extract_class_contexts(content)

        result = []
        pos = 0

        for match in self.def_pattern.finditer(content):
            if self.skip_pattern.match(content, match.start()):
                continue

            result.append(content[pos:match.start()])

            start = match.start()
            paren_start = content.find('(', start)
            if paren_start == -1:
                result.append(content[start:match.end()])
                pos = match.end()
                continue

            paren_end = self._find_matching_paren(content, paren_start)
            if paren_end == -1:
                result.append(content[start:match.end()])
                pos = match.end()
                continue

            args_text = content[paren_start + 1:paren_end]
            args = self._split_args(args_text)

            if len(args) < 2:
                result.append(content[start:paren_end + 1])
                pos = paren_end + 1
                continue

            has_doc = self._has_docstring(args)
            if has_doc and not self.force_replace:
                result.append(content[start:paren_end + 1])
                pos = paren_end + 1
                continue

            callable_ref = args[1].strip()

            current_line = content[:start].count('\n')
            class_context = class_contexts.get(current_line)

            doc_entry = self._find_doc_for_callable(callable_ref, class_context)

            if doc_entry:
                docstring = self._build_docstring(doc_entry)
                escaped = self._escape_for_cpp(docstring)

                if has_doc and self.force_replace:
                    doc_idx = self._find_docstring_arg_index(args)
                    if doc_idx is not None:
                        new_args = args[:doc_idx] + [f'"{escaped}"'] + args[doc_idx + 1:]
                        result.append(content[start:paren_start + 1])
                        result.append(", ".join(new_args))
                        result.append(")")
                        pos = paren_end + 1
                        continue

                result.append(content[start:paren_end])
                result.append(f', "{escaped}")')
                pos = paren_end + 1
                continue

            result.append(content[start:paren_end + 1])
            pos = paren_end + 1

        result.append(content[pos:])
        return "".join(result)

    def _extract_class_contexts(self, content: str) -> Dict[int, str]:
        contexts = {}
        for match in self.class_pattern.finditer(content):
            start = match.start()
            template_end = self._find_template_end(content, start)
            if template_end == -1:
                continue
            template_start = content.find('<', start) + 1
            template_content = content[template_start:template_end - 1]
            class_type = template_content.split(',')[0].strip()
            class_name = class_type.split('::')[-1] if '::' in class_type else class_type

            start_line = content[:start].count('\n')
            end_pos = content.find(';', start)
            if end_pos != -1:
                end_line = content[:end_pos].count('\n')
                for line in range(start_line, end_line + 1):
                    contexts[line] = class_name
        return contexts

    def _find_doc_for_callable(self, callable_ref: str, class_context: Optional[str] = None) -> Optional[Dict]:
        callable_ref = callable_ref.strip()

        # Function pointers like &mx::Class::method or &MaterialX::name
        if callable_ref.startswith('&'):
            name = callable_ref[1:].strip()
            name = re.sub(r'[,\s]+$', '', name)
            return self.extractor.func_lookup.get(name)

        # Lambdas: look for elem.method( or obj->method(
        method_match = re.search(r'[\.\->](\w+)\s*\(', callable_ref)
        if method_match:
            method_name = method_match.group(1)
            if class_context:
                for prefix in ("", "mx::", "MaterialX::"):
                    qualified = f"{prefix}{class_context}::{method_name}" if prefix else f"{class_context}::{method_name}"
                    doc = self.extractor.func_lookup.get(qualified)
                    if doc:
                        return doc
            return self.extractor.func_lookup.get(method_name)

        return None

    def _build_docstring(self, doc_entry: Dict) -> str:
        parts = []
        if doc_entry.get("brief"):
            parts.append(doc_entry["brief"])
        if doc_entry.get("detail"):
            parts.append(doc_entry["detail"])
        params = doc_entry.get("params", {})
        if params:
            param_lines = ["Args:"]
            for name, desc in params.items():
                param_lines.append(f"    {name}: {desc}" if desc else f"    {name}:")
            parts.append("\n".join(param_lines))
        if doc_entry.get("returns"):
            parts.append(f"Returns:\n    {doc_entry['returns']}")
        return "\n\n".join(parts)

    def _escape_for_cpp(self, s: str) -> str:
        if not s:
            return ""
        s = s.replace("\\", "\\\\").replace('"', '\\"')
        s = s.replace("\n", "\\n")
        return s

    def _find_template_end(self, content: str, start: int) -> int:
        pos = content.find('<', start)
        if pos == -1:
            return -1
        depth = 1
        i = pos + 1
        in_string = False
        while i < len(content) and depth > 0:
            c = content[i]
            if c == '"' and content[i - 1] != '\\':
                in_string = not in_string
            elif not in_string:
                if c == '<':
                    depth += 1
                elif c == '>':
                    depth -= 1
            i += 1
        return i if depth == 0 else -1

    def _find_matching_paren(self, content: str, start: int) -> int:
        depth = 0
        in_string = False
        escape = False
        for i in range(start, len(content)):
            c = content[i]
            if escape:
                escape = False
                continue
            if c == '\\':
                escape = True
                continue
            if c == '"':
                in_string = not in_string
                continue
            if not in_string:
                if c == '(':
                    depth += 1
                elif c == ')':
                    depth -= 1
                    if depth == 0:
                        return i
        return -1

    def _split_args(self, args_text: str) -> list:
        args = []
        current = []
        depth = 0
        in_string = False
        escape = False
        for c in args_text:
            if escape:
                current.append(c)
                escape = False
                continue
            if c == '\\':
                current.append(c)
                escape = True
                continue
            if c == '"':
                in_string = not in_string
                current.append(c)
                continue
            if not in_string:
                if c in '(<':
                    depth += 1
                elif c in ')>':
                    depth -= 1
                elif c == ',' and depth == 0:
                    args.append("".join(current).strip())
                    current = []
                    continue
            current.append(c)
        if current:
            args.append("".join(current).strip())
        return args

    def _extract_class_name(self, args_text: str) -> Optional[str]:
        args = self._split_args(args_text)
        if len(args) >= 2:
            return args[1].strip().strip('"')
        return None

    def _has_docstring(self, args: list) -> bool:
        for arg in args[2:]:
            a = arg.strip()
            if not a.startswith("py::arg") and a.startswith('"'):
                return True
        return False

    def _find_docstring_arg_index(self, args: list) -> Optional[int]:
        for i, arg in enumerate(args[2:], start=2):
            a = arg.strip()
            if not a.startswith("py::arg") and a.startswith('"'):
                return i
        return None


def main():
    parser = argparse.ArgumentParser(description="Extract Doxygen docs and insert into pybind11 bindings (simplified)")
    parser.add_argument("-d", "--doxygen_xml_dir", type=Path, default=Path("build/documents/doxygen_xml"), help="Path to Doxygen XML output directory")
    parser.add_argument("-p", "--pybind_dir", type=Path, default=Path("source/PyMaterialX"), help="Path to pybind11 bindings directory")
    parser.add_argument("-f", "--force", action="store_true", help="Force replace existing docstrings")
    parser.add_argument("-j", "--write_json", action="store_true", help="Write extracted docs to JSON files")

    args = parser.parse_args()

    if not args.doxygen_xml_dir.exists():
        print(f"Error: Doxygen XML directory not found: {args.doxygen_xml_dir}")
        return 1
    if not args.pybind_dir.exists():
        print(f"Error: Pybind directory not found: {args.pybind_dir}")
        return 1

    print("Extracting documentation from Doxygen XML...")
    extractor = DocExtractor(args.doxygen_xml_dir)
    extractor.extract()

    if args.write_json:
        print("\nWriting JSON files...")
        Path("class_docs.json").write_text(json.dumps(extractor.class_docs, indent=2), encoding="utf-8")
        Path("func_docs.json").write_text(json.dumps(extractor.func_docs, indent=2), encoding="utf-8")
        print("  - class_docs.json")
        print("  - func_docs.json")

    print(f"\n{'Replacing' if args.force else 'Inserting'} documentation in pybind11 files...")
    inserter = DocInserter(extractor, args.pybind_dir, args.force)
    inserter.process_all_files()

    print("\nDone!")
    return 0


if __name__ == "__main__":
    exit(main())
 
