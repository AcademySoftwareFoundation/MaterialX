#!/usr/bin/env python
'''
MaterialX Specification Tool

Compare, normalize, and expand MaterialX specification documents.

Subcommands:
- compare: Compare specification with data library, reporting differences.
- normalize: Normalize table formatting and default value notation.
- expand: Expand polymorphic tables into concrete per-signature tables.
'''

import argparse
import re
import sys
from dataclasses import dataclass, field
from enum import Enum
from itertools import product
from pathlib import Path
from typing import NamedTuple

import MaterialX as mx


# -----------------------------------------------------------------------------
# Specification Model
# -----------------------------------------------------------------------------

@dataclass
class PortInfo:
    '''Information about an input or output port from the specification.'''
    name: str
    types: list = field(default_factory=list)  # Ordered list of types, preserving spec order
    typeRef: str = None  # For "Same as X" references
    default: str = None  # Spec default string (before type-specific expansion)
    description: str = None  # Port description from spec
    acceptedValues: str = None  # Accepted values from spec

@dataclass(frozen=True)
class NodeSignature:
    '''A typed combination of inputs and outputs, corresponding to one nodedef.'''
    inputs: frozenset   # frozenset of (name, type) pairs
    outputs: frozenset  # frozenset of (name, type) pairs

    @classmethod
    def create(cls, inputs, outputs):
        '''Create a NodeSignature from input/output dicts of name -> type.'''
        return cls(frozenset(inputs.items()), frozenset(outputs.items()))

    def __str__(self):
        insStr = ', '.join(f'{n}:{t}' for n, t in sorted(self.inputs))
        outsStr = ', '.join(f'{n}:{t}' for n, t in sorted(self.outputs))
        return f'({insStr}) -> {outsStr}'

@dataclass
class ParsedTable:
    '''A parsed table from the specification with its port information.'''
    headers: list
    inputs: dict = field(default_factory=dict)   # name -> PortInfo
    outputs: dict = field(default_factory=dict)  # name -> PortInfo
    portOrder: list = field(default_factory=list)  # Preserve row order
    startLine: int = 0
    endLine: int = 0

    def allPorts(self):
        '''All input and output ports merged into a single dict.'''
        return self.inputs | self.outputs

class TypeSystemContext:
    '''Type system data derived from the standard MaterialX libraries.'''

    def __init__(self, stdlib):
        self.typeGroups = buildTypeGroups(stdlib)
        self.standardTypes = {td.getName() for td in stdlib.getTypeDefs()}
        geompropNames = {gpd.getName() for gpd in stdlib.getGeomPropDefs()}

        # Spec tables use "colorM" to mean "a colorN member that must differ
        # from the colorN choice."  Map each such alias to its base group.
        self.typeGroupAliases = {
            g[:-1] + 'M': g
            for g in self.typeGroups
            if g.endswith('N') and not g.endswith('NN')
        }

        self.knownTypes = self.standardTypes | self.typeGroups.keys() | self.typeGroupAliases.keys()
        self.converter = DefaultConverter(geompropNames)

class NodeDefInfo(NamedTuple):
    '''Information about a single nodedef from a MaterialX document.'''
    nodeName: str
    nodedefName: str
    signature: object  # NodeSignature
    defaults: dict  # portName -> (value, isGeomprop)

class IssueType(Enum):
    '''Categories of issues found during specification parsing and comparison.'''
    # Specification validation issues
    SPEC_COLUMN_MISMATCH = 'Column Count Mismatches in Specification'
    SPEC_EMPTY_PORT_NAME = 'Empty Port Names in Specification'
    SPEC_UNRECOGNIZED_TYPE = 'Unrecognized Types in Specification'
    # Node-level differences
    NODE_MISSING_IN_LIBRARY = 'Nodes in Specification but not Data Library'
    NODE_MISSING_IN_SPEC = 'Nodes in Data Library but not Specification'
    # Signature-level differences
    SIGNATURE_DIFFERENT_INPUTS = 'Nodes with Different Input Sets'
    SIGNATURE_MISSING_IN_LIBRARY = 'Node Signatures in Specification but not Data Library'
    SIGNATURE_MISSING_IN_SPEC = 'Node Signatures in Data Library but not Specification'
    # Default value differences
    DEFAULT_MISMATCH = 'Default Value Mismatches'

@dataclass
class Issue:
    '''An issue found during specification parsing or comparison.'''
    issueType: IssueType
    nodeName: str
    message: str = ''


# -----------------------------------------------------------------------------
# Type System
# -----------------------------------------------------------------------------

def loadStandardLibraries():
    '''Load and return the standard MaterialX libraries as a document.'''
    stdlib = mx.createDocument()
    mx.loadLibraries(mx.getDefaultDataLibraryFolders(), mx.getDefaultDataSearchPath(), stdlib)
    return stdlib

def buildTypeGroups(stdlib):
    '''
    Build type groups (colorN, vectorN, matrixNN) from standard library TypeDefs.
    Returns a dict of groupName -> sorted list of concrete types.
    '''
    groups = {}
    for td in stdlib.getTypeDefs():
        name = td.getName()
        # Match colorN, vectorN patterns (color3, vector2, etc.)
        match = re.match(r'^(color|vector)(\d)$', name)
        if match:
            groupName = f'{match.group(1)}N'
            groups.setdefault(groupName, []).append(name)
            continue
        # Match matrixNN pattern (matrix33, matrix44)
        match = re.match(r'^matrix(\d)\1$', name)
        if match:
            groups.setdefault('matrixNN', []).append(name)
    # Sort each group by length then name for deterministic ordering
    return {g: sorted(ts, key=lambda t: (len(t), t)) for g, ts in groups.items()}

def parseSpecTypes(typeStr):
    '''
    Parse a specification type string into (types, typeRef).

    Supported patterns:
      - Simple types: "float", "color3"
      - Comma-separated: "float, color3"
      - Union with "or": "BSDF or VDF", "BSDF, EDF, or VDF"
      - Type references: "Same as bg", "Same as in1 or float"
    '''
    if typeStr is None or not typeStr.strip():
        return [], None

    typeStr = typeStr.strip()

    # Handle "Same as X" and "Same as X or Y" references
    sameAsMatch = re.match(r'^Same as\s+`?(\w+)`?(?:\s+or\s+(.+))?$', typeStr, re.IGNORECASE)
    if sameAsMatch:
        refPort = sameAsMatch.group(1)
        extraTypes = sameAsMatch.group(2)
        extraList = []
        if extraTypes:
            extraList, _ = parseSpecTypes(extraTypes)
        return extraList, refPort

    # Normalize "or" to comma: "X or Y" -> "X, Y", "X, Y, or Z" -> "X, Y, Z"
    normalized = re.sub(r',?\s+or\s+', ', ', typeStr)

    result = []
    for t in normalized.split(','):
        t = t.strip()
        if t and t not in result:  # Preserve order, avoid duplicates
            result.append(t)

    return result, None

def expandTypeSet(types, typeCtx):
    '''Expand type groups to concrete types. Returns list of (concreteType, groupName) tuples.'''
    result = []
    for t in types:
        if t in typeCtx.typeGroups:
            for concrete in typeCtx.typeGroups[t]:
                result.append((concrete, t))
        elif t in typeCtx.typeGroupAliases:
            baseGroup = typeCtx.typeGroupAliases[t]
            for concrete in typeCtx.typeGroups[baseGroup]:
                result.append((concrete, t))
        else:
            result.append((t, None))
    return result


# -----------------------------------------------------------------------------
# Default Value Conversion
# -----------------------------------------------------------------------------

def getComponentCount(typeName):
    '''Get the number of components for a MaterialX type, or None if unknown.'''
    if typeName in ('float', 'integer', 'boolean'):
        return 1
    match = re.match(r'^(color|vector)(\d)$', typeName)
    if match:
        return int(match.group(2))
    match = re.match(r'^matrix(\d)(\d)$', typeName)
    if match:
        return int(match.group(1)) * int(match.group(2))
    return None

def isClosureType(typeName):
    '''Check if a type is a closure type (distribution function, shader, or material).'''
    return typeName in {
        'BSDF', 'EDF', 'VDF',
        'surfaceshader', 'volumeshader', 'lightshader', 'displacementshader',
        'material',
    }

class DefaultConverter:
    '''Convert default values between Markdown notation, normalized placeholders, and typed MaterialX values.'''

    # Placeholder expansion overrides for specific types.
    # Unlisted types fall through to the generic N-component expansion.
    _OVERRIDES = {
        '0':   {'boolean': 'false'},
        '1':   {'boolean': 'true',
                'matrix33': '1, 0, 0, 0, 1, 0, 0, 0, 1',
                'matrix44': '1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1'},
        '0.5': {},
    }

    # Placeholders that have no meaningful expansion for these types.
    _EXCLUDED = {
        '0.5': {'integer', 'boolean'},
    }

    # Single source of truth for spec notation ←→ normalized placeholder.
    _NOTATION_TO_PLACEHOLDER = {
        '__zero__': '0',
        '__one__':  '1',
        '__half__': '0.5',
        '__empty__': '',
    }

    def __init__(self, geompropNames):
        self._geompropNames = geompropNames

        # Build both directions of the notation mapping from the single source above.
        self.notationToPlaceholder = dict(self._NOTATION_TO_PLACEHOLDER)
        self.placeholderToNotation = {v: k for k, v in self._NOTATION_TO_PLACEHOLDER.items()}
        for name in geompropNames:
            self.notationToPlaceholder[f'_{name}_'] = name
            self.placeholderToNotation[name] = f'_{name}_'

    def _expandPlaceholder(self, placeholder, typeName):
        '''
        Expand a placeholder ("0", "1", "0.5") to a type-appropriate value string.
        Returns None if the placeholder cannot be expanded for the given type.
        '''

        # Only expand recognized placeholders; pass through literal defaults.
        if placeholder not in self._OVERRIDES:
            return None

        # Closure types (BSDF, shaders, material) only have a meaningful zero.
        if isClosureType(typeName):
            return '' if placeholder == '0' else None

        # Check whether this placeholder is excluded for the type.
        if typeName in self._EXCLUDED.get(placeholder, ()):
            return None

        # Check for a type-specific override.
        override = self._OVERRIDES.get(placeholder, {}).get(typeName)
        if override is not None:
            return override

        # Generic expansion: repeat the placeholder for each component.
        count = getComponentCount(typeName)
        if count is None:
            return None
        return ', '.join([placeholder] * count)

    def toTableCell(self, specDefault, valueType):
        '''Convert a normalized placeholder to a Markdown table cell string.'''
        if specDefault is None:
            return None

        # Geomprops and empty strings map directly to their notation.
        if specDefault in self._geompropNames:
            return f'_{specDefault}_'
        if specDefault == '':
            return '__empty__'

        # Closure types keep __zero__ notation rather than expanding.
        if isClosureType(valueType) and specDefault == '0':
            return '__zero__'

        expansion = self._expandPlaceholder(specDefault, valueType)
        return expansion if expansion is not None else specDefault

    def toTypedValue(self, specDefault, valueType):
        '''
        Convert a normalized placeholder to a typed MaterialX value.
        Returns (value, isGeomprop).  Returns (None, False) on failure.
        '''
        if specDefault is None or specDefault == '':
            return None, False

        # Geomprops are returned as plain strings.
        if specDefault in self._geompropNames:
            return specDefault, True

        expansion = self._expandPlaceholder(specDefault, valueType)
        valueStr = expansion if expansion is not None else specDefault
        try:
            return mx.createValueFromStrings(valueStr, valueType), False
        except Exception:
            return None, False

    def fromTypedValue(self, value, valueType):
        '''Convert a typed MaterialX value back to spec notation for display.'''
        if value is None:
            return 'None'

        # String values: geomprops, empty strings, or pass-through.
        if isinstance(value, str):
            if value in self._geompropNames:
                return f'_{value}_'
            if value == '':
                return '__zero__' if isClosureType(valueType) else '__empty__'
            return value

        # Try each placeholder to see if the value matches its expansion.
        for placeholder, notation in self.placeholderToNotation.items():
            if not placeholder:  # skip the empty-string entry
                continue
            expansion = self._expandPlaceholder(placeholder, valueType)
            if expansion is None:
                continue
            try:
                if value == mx.createValueFromStrings(expansion, valueType):
                    return notation
            except Exception:
                pass

        return str(value)


# -----------------------------------------------------------------------------
# Markdown Table I/O
# -----------------------------------------------------------------------------

def stripBackticks(s):
    '''Strip wrapping backticks from a string.'''
    s = s.strip()
    if s.startswith('`') and s.endswith('`'):
        return s[1:-1]
    return s

def parseMarkdownTable(lines, startLine):
    '''Parse a markdown table. Returns (rows, headers, columnMismatchCount, endLine).'''
    table = []
    headers = []
    columnMismatchCount = 0
    idx = startLine

    # Parse header row
    if idx < len(lines) and '|' in lines[idx]:
        headerLine = lines[idx].strip()
        headers = [stripBackticks(h) for h in headerLine.split('|')[1:-1]]
        idx += 1
    else:
        return [], [], 0, startLine

    # Skip separator row
    if idx < len(lines) and '|' in lines[idx] and '-' in lines[idx]:
        idx += 1
    else:
        return [], [], 0, startLine

    # Parse data rows
    while idx < len(lines):
        line = lines[idx].strip()
        if not line or not line.startswith('|'):
            break

        cells = [stripBackticks(c) for c in line.split('|')[1:-1]]
        if len(cells) == len(headers):
            row = {headers[i].lower(): cells[i] for i in range(len(headers))}
            table.append(row)
        else:
            columnMismatchCount += 1
        idx += 1

    return table, headers, columnMismatchCount, idx

def generateMarkdownTable(rows, headers):
    '''Generate a markdown table from a list of row dicts keyed by lowercase header name.'''

    # Collect all cell values for all rows
    allRows = [[row.get(h.lower(), '') for h in headers] for row in rows]

    # Calculate column widths (max of header and all cell values)
    colWidths = []
    for colIdx, header in enumerate(headers):
        maxWidth = len(header)
        for row in allRows:
            if colIdx < len(row):
                maxWidth = max(maxWidth, len(row[colIdx]))
        colWidths.append(maxWidth)

    # Build header row with padding
    headerCells = [h.ljust(colWidths[i]) for i, h in enumerate(headers)]
    headerRow = '|' + '|'.join(headerCells) + '|'

    # Build separator row with matching widths
    separatorRow = '|' + '|'.join(['-' * w for w in colWidths]) + '|'

    # Build data rows with padding
    lines = [headerRow, separatorRow]
    for row in allRows:
        paddedCells = [row[i].ljust(colWidths[i]) for i in range(len(headers))]
        lines.append('|' + '|'.join(paddedCells) + '|')

    return lines

def applyReplacements(lines, replacements):
    '''Apply line replacements in reverse order to preserve line numbers. Returns the updated text.'''
    resultLines = lines.copy()
    for startLine, endLine, replacementLines in sorted(replacements, reverse=True):
        resultLines[startLine:endLine] = replacementLines
    return '\n'.join(resultLines)


# -----------------------------------------------------------------------------
# Signature Expansion
# -----------------------------------------------------------------------------

def isValidTypeGroupAssignment(driverNames, combo, typeGroupAliases):
    '''
    Check if type assignments satisfy group constraints (e.g., colorN ports must
    match, colorM must differ from colorN). Returns a dict or None.
    '''
    typeAssignment = {}
    groupAssignments = {}  # groupName -> concreteType assigned to that group

    for name, (concreteType, groupName) in zip(driverNames, combo):
        typeAssignment[name] = concreteType

        # Skip constraint checking for None types (these will be resolved via typeRef)
        if concreteType is None:
            continue

        if not groupName:
            continue

        # For group aliases (colorM), get the base group (colorN)
        baseGroup = typeGroupAliases.get(groupName, groupName)
        isAlias = groupName in typeGroupAliases

        # Check consistency: all uses of the same group must have same concrete type
        if groupName in groupAssignments:
            if groupAssignments[groupName] != concreteType:
                return None
        else:
            groupAssignments[groupName] = concreteType

        # Aliases and their base groups must differ in concrete type
        if isAlias and baseGroup in groupAssignments:
            if groupAssignments[baseGroup] == concreteType:
                return None
        if not isAlias:
            for alias, base in typeGroupAliases.items():
                if base == groupName and alias in groupAssignments:
                    if groupAssignments[alias] == concreteType:
                        return None

    return typeAssignment

def resolveTypeAssignment(baseAssignment, allPorts):
    '''Resolve "Same as X" references to complete port type assignments.'''
    assignment = baseAssignment.copy()

    # Iteratively resolve references (limit iterations to handle circular refs)
    for _ in range(10):
        changed = False
        for name, port in allPorts.items():
            if name in assignment:
                continue
            if port.typeRef and port.typeRef in assignment:
                assignment[name] = assignment[port.typeRef]
                changed = True
        if not changed:
            break

    # Check all ports resolved
    if set(assignment.keys()) != set(allPorts.keys()):
        return None

    return assignment

def expandSpecSignatures(inputs, outputs, typeCtx):
    '''
    Expand spec port definitions into concrete NodeSignatures.
    Handles type groups, type group aliases, and "Same as X or Y" patterns.
    '''
    allPorts = inputs | outputs

    # Identify driver ports and their type options
    # - Ports with explicit types (no typeRef): use those types
    # - Ports with both types AND typeRef ("Same as X or Y"): explicit types OR inherit from typeRef
    drivers = {}
    for name, port in allPorts.items():
        if port.types and not port.typeRef:
            # Normal driver: explicit types only
            drivers[name] = expandTypeSet(port.types, typeCtx)
        elif port.types and port.typeRef:
            # "Same as X or Y" pattern: explicit types OR inherit from typeRef
            expanded = expandTypeSet(port.types, typeCtx)
            expanded.append((None, None))  # None means "inherit from typeRef"
            drivers[name] = expanded

    if not drivers:
        return []

    # Generate all combinations of driver types
    driverNames = sorted(drivers.keys())
    driverTypeLists = [drivers[n] for n in driverNames]

    signatures = []
    seen = set()
    for combo in product(*driverTypeLists):
        # Validate type group constraints (skip None values which will be resolved via typeRef)
        typeAssignment = isValidTypeGroupAssignment(driverNames, combo, typeCtx.typeGroupAliases)
        if typeAssignment is None:
            continue

        # Remove None assignments - these ports will be resolved via typeRef
        typeAssignment = {k: v for k, v in typeAssignment.items() if v is not None}

        # Resolve typeRefs for this combination
        resolved = resolveTypeAssignment(typeAssignment, allPorts)
        if resolved is None:
            continue

        # Build signature
        sigInputs = {name: resolved[name] for name in inputs if name in resolved}
        sigOutputs = {name: resolved[name] for name in outputs if name in resolved}
        sig = NodeSignature.create(sigInputs, sigOutputs)

        # Only add if not already seen (preserve first occurrence order)
        if sig not in seen:
            seen.add(sig)
            signatures.append(sig)

    return signatures

def expandTable(table, typeCtx):
    '''Expand a parsed table into a list of (signature, [row dict]) pairs.'''
    allPorts = table.allPorts()
    signatures = expandSpecSignatures(table.inputs, table.outputs, typeCtx)

    expanded = []
    for sig in signatures:
        concreteTypes = dict(sig.inputs | sig.outputs)
        rows = []
        for portName in table.portOrder:
            port = allPorts.get(portName)
            if not port:
                continue
            valueType = concreteTypes.get(portName, '')
            default = typeCtx.converter.toTableCell(port.default, valueType)
            if default is None:
                default = port.default or ''
            rows.append({
                'port': f'`{portName}`',
                'type': valueType,
                'default': default,
                'description': port.description or '',
                'accepted values': port.acceptedValues or '',
            })
        expanded.append((sig, rows))

    return expanded

def extractNodeDefs(doc):
    '''Extract per-nodedef information from a MaterialX document.'''
    result = []
    for nodedef in doc.getNodeDefs():
        nodeName = nodedef.getNodeString()
        nodedefName = nodedef.getName()
        sigInputs = {inp.getName(): inp.getType() for inp in nodedef.getInputs()}
        sigOutputs = {out.getName(): out.getType() for out in nodedef.getOutputs()}
        sig = NodeSignature.create(sigInputs, sigOutputs)

        defaults = {}
        for inp in nodedef.getInputs():
            if inp.hasDefaultGeomPropString():
                defaults[inp.getName()] = (inp.getDefaultGeomPropString(), True)
            elif inp.getValue() is not None:
                defaults[inp.getName()] = (inp.getValue(), False)

        result.append(NodeDefInfo(nodeName, nodedefName, sig, defaults))
    return result


# -----------------------------------------------------------------------------
# Specification Parsing
# -----------------------------------------------------------------------------

def parseSpec(text, typeCtx):
    '''Parse specification markdown text. Returns (nodeSections, warnings, lines).'''
    lines = text.split('\n')
    nodeSections = {}
    warnings = []
    currentNode = None
    idx = 0

    while idx < len(lines):
        line = lines[idx]

        # Check for node header (### `nodename`)
        nodeMatch = re.match(r'^###\s+`([^`]+)`', line)
        if nodeMatch:
            currentNode = nodeMatch.group(1)
            nodeSections.setdefault(currentNode, [])
            idx += 1
            continue

        # Any other section header ends the current node's scope, so that
        # tables in non-node sections are not parsed.
        if currentNode and re.match(r'^#{1,3}\s', line):
            currentNode = None
            idx += 1
            continue

        # Check for table start
        if currentNode and '|' in line and 'Port' in line:
            table, tableWarnings, idx = parseNodeTable(lines, idx, typeCtx, nodeName=currentNode)
            warnings.extend(tableWarnings)
            if table:
                nodeSections[currentNode].append(table)
            continue

        idx += 1

    return nodeSections, warnings, lines

def parseNodeTable(lines, startIdx, typeCtx, nodeName):
    '''
    Parse a specification table starting at startIdx.
    Returns (ParsedTable or None, warnings, endIdx).
    '''
    warnings = []
    rows, headers, columnMismatchCount, endIdx = parseMarkdownTable(lines, startIdx)

    for _ in range(columnMismatchCount):
        warnings.append(Issue(IssueType.SPEC_COLUMN_MISMATCH, nodeName))

    if not rows:
        return None, warnings, endIdx

    # Process all rows into local port dicts
    inputs = {}
    outputs = {}
    for row in rows:
        portName = row.get('port', '').strip('`*')

        if not portName:
            warnings.append(Issue(IssueType.SPEC_EMPTY_PORT_NAME, nodeName))
            continue

        portType = row.get('type', '')
        portDefault = row.get('default', '')
        portDesc = row.get('description', '')
        portAccepted = row.get('accepted values', '')

        types, typeRef = parseSpecTypes(portType)

        # Track unrecognized types
        unrecognized = [t for t in types if t not in typeCtx.knownTypes]
        if unrecognized:
            warnings.append(Issue(
                IssueType.SPEC_UNRECOGNIZED_TYPE, nodeName,
                f'{portName}: {", ".join(sorted(unrecognized))}'))

        # Determine if this is an output port
        isOutput = (portName == 'out') or portDesc.lower().startswith('output')
        target = outputs if isOutput else inputs

        # Create or merge port info
        portInfo = target.setdefault(portName, PortInfo(
            name=portName,
            default=typeCtx.converter.notationToPlaceholder.get(portDefault, portDefault) if portDefault.strip() else None,
            description=portDesc,
            acceptedValues=portAccepted,
        ))
        for t in types:
            if t not in portInfo.types:
                portInfo.types.append(t)
        if typeRef and not portInfo.typeRef:
            portInfo.typeRef = typeRef

    if not inputs and not outputs:
        return None, warnings, endIdx

    table = ParsedTable(
        headers=headers,
        inputs=inputs,
        outputs=outputs,
        portOrder=list(inputs.keys()) + list(outputs.keys()),
        startLine=startIdx,
        endLine=endIdx,
    )
    return table, warnings, endIdx


# -----------------------------------------------------------------------------
# Subcommand Utilities
# -----------------------------------------------------------------------------

def resolveSpecPath(specFile):
    '''Resolve the specification file path from a command-line argument.'''
    if specFile:
        path = Path(specFile)
    else:
        path = Path(__file__).resolve().parent.parent.parent / 'documents' / 'Specification' / 'MaterialX.StandardNodes.md'
    if not path.exists():
        raise FileNotFoundError(f'Specification document not found: {path}')
    return path

def printWarnings(warnings):
    '''Print specification parsing warnings to stderr, grouped by type.'''
    if not warnings:
        return

    byType = {}
    for warning in warnings:
        byType.setdefault(warning.issueType, []).append(warning)

    print(f'\nSpecification warnings ({len(warnings)}):', file=sys.stderr)
    for issueType in IssueType:
        typeWarnings = byType.get(issueType)
        if not typeWarnings:
            continue
        print(f'  {issueType.value} ({len(typeWarnings)}):', file=sys.stderr)
        for warning in typeWarnings:
            detail = f': {warning.message}' if warning.message else ''
            print(f'    {warning.nodeName}{detail}', file=sys.stderr)

def writeOutput(text, specPath, opts):
    '''Write output text to the appropriate destination.'''
    if opts.inplace:
        with open(specPath, 'w', encoding='utf-8', newline='\n') as f:
            f.write(text)
        print(f'Output written to: {specPath}', file=sys.stderr)
    elif opts.outputFile:
        outputPath = Path(opts.outputFile)
        with open(outputPath, 'w', encoding='utf-8', newline='\n') as f:
            f.write(text)
        print(f'Output written to: {outputPath}', file=sys.stderr)
    else:
        sys.stdout.reconfigure(encoding='utf-8')
        print(text)


# -----------------------------------------------------------------------------
# Compare Subcommand
# -----------------------------------------------------------------------------

def loadDataLibrary(mtlxPath):
    '''Load a data library MTLX document. Returns (signatures, defaults).'''
    doc = mx.createDocument()
    mx.readFromXmlFile(doc, str(mtlxPath))

    signatures = {}
    defaults = {}

    for ndi in extractNodeDefs(doc):
        signatures.setdefault(ndi.nodeName, set()).add(ndi.signature)
        if ndi.defaults:
            defaults[(ndi.nodeName, ndi.signature)] = ndi.defaults

    return signatures, defaults

def expandSpecToNodes(nodeSections, typeCtx):
    '''Expand a parsed spec into signatures and defaults dicts.'''
    signatures = {}
    specDefaults = {}
    for nodeName, tables in nodeSections.items():
        sigs = signatures.setdefault(nodeName, set())
        for table in tables:
            tableSigs = expandSpecSignatures(table.inputs, table.outputs, typeCtx)
            sigs.update(tableSigs)

            # Associate this table's input defaults with each signature
            # expanded from the table, keyed by (nodeName, signature) to
            # mirror the data library defaults.
            tableDefaults = {name: port.default for name, port in table.inputs.items() if port.default}
            for sig in tableSigs:
                specDefaults.setdefault((nodeName, sig), tableDefaults)
    return signatures, specDefaults

def compareSignatureDefaults(nodeName, signature, specDefaults, libDefaults, typeCtx):
    '''Compare default values for a matching signature. Returns list of Issues.'''
    converter = typeCtx.converter
    issues = []

    for portName, valueType in signature.inputs:
        specDefault = specDefaults.get(portName)
        if not specDefault:
            continue

        specValue, specIsGeomprop = converter.toTypedValue(specDefault, valueType)
        libValue, libIsGeomprop = libDefaults.get(portName, (None, False))

        # Skip if either value is unavailable
        if specValue is None or libValue is None:
            continue

        # Compare values (geomprops compare as strings, typed values use equality)
        valuesMatch = (specValue == libValue) if (specIsGeomprop == libIsGeomprop) else False

        if not valuesMatch:
            issues.append(Issue(
                IssueType.DEFAULT_MISMATCH, nodeName,
                f'{portName} ({valueType}):\n'
                f'Signature:            {signature}\n'
                f'Spec default:         {converter.fromTypedValue(specValue, valueType)}\n'
                f'Data library default: {converter.fromTypedValue(libValue, valueType)}'))

    return issues

def findLibraryMatch(specSig, libSigs):
    '''
    Find a matching library signature. Returns (libSig, extraInLib, extraInSpec),
    or (None, None, None) if no match. Extra tuples are empty for exact matches.
    '''
    specInputs = specSig.inputs
    specOutputs = specSig.outputs

    # First pass: look for an exact match.
    for libSig in libSigs:
        if specInputs == libSig.inputs and specOutputs == libSig.outputs:
            return libSig, (), ()

    # Second pass: look for a fuzzy match with the same outputs.
    for libSig in sorted(libSigs, key=str):
        libInputs = libSig.inputs
        libOutputs = libSig.outputs

        if specOutputs != libOutputs:
            continue

        # Different input sets: verify common inputs have the same types
        commonInputNames = {name for name, _ in specInputs} & {name for name, _ in libInputs}
        if commonInputNames:
            specInputDict = dict(specSig.inputs)
            libInputDict = dict(libSig.inputs)
            if not all(specInputDict[n] == libInputDict[n] for n in commonInputNames):
                continue

        extraInLib = tuple(sorted(libInputs - specInputs))
        extraInSpec = tuple(sorted(specInputs - libInputs))
        return libSig, extraInLib, extraInSpec

    return None, None, None

def compareNodes(specSigs, libSigs, specDefaults, libDefaults, typeCtx, compareDefaults=False):
    '''Compare nodes between spec and library. Returns list of Issues.'''
    issues = []

    specNames = set(specSigs.keys())
    libNames = set(libSigs.keys())

    # Nodes in spec but not in library
    for nodeName in sorted(specNames - libNames):
        issues.append(Issue(IssueType.NODE_MISSING_IN_LIBRARY, nodeName))

    # Nodes in library but not in spec
    for nodeName in sorted(libNames - specNames):
        issues.append(Issue(IssueType.NODE_MISSING_IN_SPEC, nodeName))

    # Compare signatures for common nodes
    for nodeName in sorted(specNames & libNames):
        nodeSpecSigs = specSigs[nodeName]
        nodeLibSigs = libSigs[nodeName]

        # Track which signatures have been matched
        matchedLibSigs = set()
        matchedSpecSigs = set()
        inputDiffMatches = []

        # For each spec signature, find matching library signature
        for specSig in nodeSpecSigs:
            libSig, extraInLib, extraInSpec = findLibraryMatch(specSig, nodeLibSigs)
            if libSig is None:
                continue

            matchedLibSigs.add(libSig)
            matchedSpecSigs.add(specSig)

            if extraInLib or extraInSpec:
                inputDiffMatches.append((specSig, libSig, extraInLib, extraInSpec))

            if compareDefaults:
                sigDefaults = libDefaults.get((nodeName, libSig), {})
                issues.extend(compareSignatureDefaults(
                    nodeName, specSig, specDefaults.get((nodeName, specSig), {}), sigDefaults, typeCtx))

        # Report different input set matches
        for specSig, libSig, extraInLib, extraInSpec in sorted(inputDiffMatches, key=lambda x: str(x[0])):
            lines = [f'{specSig}']
            if extraInLib:
                lines.append(f'Extra in library: {", ".join(f"{n}:{t}" for n, t in extraInLib)}')
            if extraInSpec:
                lines.append(f'Extra in spec:    {", ".join(f"{n}:{t}" for n, t in extraInSpec)}')
            issues.append(Issue(
                IssueType.SIGNATURE_DIFFERENT_INPUTS, nodeName, '\n'.join(lines)))

        # Spec signatures not matched by any library signature
        for specSig in sorted(nodeSpecSigs - matchedSpecSigs, key=str):
            issues.append(Issue(
                IssueType.SIGNATURE_MISSING_IN_LIBRARY, nodeName, str(specSig)))

        # Library signatures not matched by any spec signature
        for libSig in sorted(nodeLibSigs - matchedLibSigs, key=str):
            issues.append(Issue(
                IssueType.SIGNATURE_MISSING_IN_SPEC, nodeName, str(libSig)))

    return issues

def printIssues(issues):
    '''Print the issues in a formatted way.'''
    if not issues:
        print("No differences found between specification and data library.")
        return

    # Group issues by type
    byType = {}
    for issue in issues:
        byType.setdefault(issue.issueType, []).append(issue)

    print(f"\n{'=' * 70}")
    print(f"COMPARISON RESULTS: {len(issues)} difference(s) found")
    print(f"{'=' * 70}")

    for issueType in IssueType:
        if issueType not in byType:
            continue

        typeIssues = byType[issueType]
        print(f"\n{issueType.value} ({len(typeIssues)}):")
        print("-" * 50)

        for issue in typeIssues:
            if not issue.message:
                print(f'  {issue.nodeName}')
            else:
                lines = issue.message.split('\n')
                print(f'  {issue.nodeName}: {lines[0]}')
                for line in lines[1:]:
                    print(f'    {line}')

def compareMain(opts):
    '''Run the compare subcommand.'''
    specPath = resolveSpecPath(opts.specFile)
    if opts.mtlxFile:
        mtlxPath = Path(opts.mtlxFile)
    else:
        found = mx.getDefaultDataSearchPath().find('libraries/stdlib/stdlib_defs.mtlx')
        mtlxPath = Path(found.asString())

    if not mtlxPath.exists():
        raise FileNotFoundError(f"Data library document not found: {mtlxPath}")

    print(f"Comparing:")
    print(f"  Specification: {specPath}")
    print(f"  Data Library: {mtlxPath}")

    # Build type system data
    stdlib = loadStandardLibraries()
    typeCtx = TypeSystemContext(stdlib)

    # Parse specification
    print("\nParsing specification...")
    nodeSections, warnings, _ = parseSpec(specPath.read_text(encoding='utf-8'), typeCtx)
    specSigs, specDefaults = expandSpecToNodes(nodeSections, typeCtx)
    specSigCount = sum(len(sigs) for sigs in specSigs.values())
    print(f"  Found {len(specSigs)} nodes with {specSigCount} node signatures")
    if warnings:
        print(f"  Found {len(warnings)} invalid specification entries")

    # Load data library
    print("Loading data library...")
    libSigs, libDefaults = loadDataLibrary(mtlxPath)
    libSigCount = sum(len(sigs) for sigs in libSigs.values())
    print(f"  Found {len(libSigs)} nodes with {libSigCount} node signatures")

    # List nodes if requested
    if opts.listNodes:
        print("\nNodes in Specification:")
        for name in sorted(specSigs.keys()):
            print(f"  {name}: {len(specSigs[name])} signature(s)")

        print("\nNodes in Data Library:")
        for name in sorted(libSigs.keys()):
            print(f"  {name}: {len(libSigs[name])} signature(s)")

    # Compare nodes
    print("\nComparing node signatures...")
    issues = compareNodes(specSigs, libSigs, specDefaults, libDefaults, typeCtx, opts.compareDefaults)

    # Include spec validation issues
    issues = warnings + issues

    # Print issues
    printIssues(issues)
    print()


# -----------------------------------------------------------------------------
# Normalize Subcommand
# -----------------------------------------------------------------------------

def normalizeSpec(text, typeCtx):
    '''Normalize table column alignment and default value notation. Returns (updatedText, warnings).'''
    nodeSections, warnings, lines = parseSpec(text, typeCtx)

    replacements = []
    for nodeName, tables in nodeSections.items():
        for table in tables:
            signatures = expandSpecSignatures(table.inputs, table.outputs, typeCtx)

            # Single signature: resolve concrete types for default expansion
            concreteTypes = {}
            if len(signatures) == 1:
                concreteTypes = dict(signatures[0].inputs | signatures[0].outputs)

            normalizedRows = []
            for portName in table.portOrder:
                port = table.allPorts().get(portName)
                if not port:
                    continue

                # Reconstruct the polymorphic type string from parsed port info
                types = ', '.join(port.types) if port.types else ''
                if port.typeRef:
                    typeStr = f'Same as `{port.typeRef}` or {types}' if types else f'Same as `{port.typeRef}`'
                else:
                    typeStr = types

                if port.default is None:
                    default = ''
                elif concreteTypes:
                    default = typeCtx.converter.toTableCell(port.default, concreteTypes.get(portName, ''))
                    if default is None:
                        default = port.default
                else:
                    default = typeCtx.converter.placeholderToNotation.get(port.default, port.default)

                normalizedRows.append({
                    'port': f'`{portName}`',
                    'type': typeStr,
                    'default': default or '',
                    'description': port.description or '',
                    'accepted values': port.acceptedValues or '',
                })

            if normalizedRows:
                replacementLines = generateMarkdownTable(normalizedRows, table.headers)
                replacements.append((table.startLine, table.endLine, replacementLines))

    return applyReplacements(lines, replacements), warnings

def normalizeMain(opts):
    '''Run the normalize subcommand.'''
    specPath = resolveSpecPath(opts.specFile)
    stdlib = loadStandardLibraries()
    typeCtx = TypeSystemContext(stdlib)
    text = specPath.read_text(encoding='utf-8')
    text, warnings = normalizeSpec(text, typeCtx)
    printWarnings(warnings)
    writeOutput(text, specPath, opts)


# -----------------------------------------------------------------------------
# Expand Subcommand
# -----------------------------------------------------------------------------

def expandSpec(text, typeCtx, nodedefNames=None):
    '''Expand polymorphic tables into concrete per-signature tables. Returns (updatedText, missingSignatures, warnings).'''
    nodeSections, warnings, lines = parseSpec(text, typeCtx)

    replacements = []
    missingSignatures = []

    for nodeName, tables in nodeSections.items():
        for table in tables:
            expanded = expandTable(table, typeCtx)
            if not expanded:
                continue

            if len(expanded) <= 1 and nodedefNames is None:
                continue

            replacementLines = []
            for i, (sig, ports) in enumerate(expanded):
                if i > 0:
                    replacementLines.append('')

                if nodedefNames is not None:
                    nodedefName = nodedefNames.get((nodeName, sig))
                    if nodedefName is None:
                        missingSignatures.append((nodeName, sig))
                        inTypes = ', '.join(t for _, t in sorted(sig.inputs))
                        outTypes = ', '.join(t for _, t in sorted(sig.outputs))
                        sigStr = f'{inTypes} -> {outTypes}' if inTypes else outTypes
                        subheader = f'#### No NodeDef: {sigStr}'
                    else:
                        subheader = f'#### `{nodedefName}`'
                    replacementLines.append(subheader)
                    replacementLines.append('')

                tableLines = generateMarkdownTable(ports, table.headers)
                replacementLines.extend(tableLines)

            replacements.append((table.startLine, table.endLine, replacementLines))

    return applyReplacements(lines, replacements), missingSignatures, warnings

def expandMain(opts):
    '''Run the expand subcommand.'''
    specPath = resolveSpecPath(opts.specFile)
    stdlib = loadStandardLibraries()
    typeCtx = TypeSystemContext(stdlib)
    text = specPath.read_text(encoding='utf-8')

    nodedefNames = None
    if opts.nodedefs:
        print('Loading nodedef names from standard libraries...', file=sys.stderr)
        nodedefNames = {(ndi.nodeName, ndi.signature): ndi.nodedefName
                        for ndi in extractNodeDefs(stdlib)}
        print(f'  Found {len(nodedefNames)} nodedefs', file=sys.stderr)

    text, missingSignatures, warnings = expandSpec(text, typeCtx, nodedefNames)
    printWarnings(warnings)

    if missingSignatures:
        print(f'\nSignatures in specification but not in data library ({len(missingSignatures)}):', file=sys.stderr)
        for nodeName, sig in missingSignatures:
            print(f'  {nodeName}: {sig}', file=sys.stderr)

    writeOutput(text, specPath, opts)


# -----------------------------------------------------------------------------
# Main Entry Point
# -----------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description='MaterialX Specification Tool: compare, normalize, and expand specification documents.')
    subparsers = parser.add_subparsers(dest='command')

    # Compare subcommand
    compareParser = subparsers.add_parser('compare',
        help='Compare spec vs data library, reporting differences')
    compareParser.add_argument('--spec', dest='specFile',
        help='Specification Markdown document')
    compareParser.add_argument('--mtlx', dest='mtlxFile',
        help='Data library MaterialX document')
    compareParser.add_argument('--defaults', dest='compareDefaults', action='store_true',
        help='Compare default values using typed value comparison')
    compareParser.add_argument('--listNodes', dest='listNodes', action='store_true',
        help='List all nodes and their signature counts')

    # Normalize subcommand
    normalizeParser = subparsers.add_parser('normalize',
        help='Normalize table formatting and default value notation')
    normalizeParser.add_argument('--spec', dest='specFile',
        help='Specification Markdown document')
    normalizeParser.add_argument('--output', dest='outputFile',
        help='Path to write the output document. Defaults to stdout.')
    normalizeParser.add_argument('--inplace', dest='inplace', action='store_true',
        help='Modify the specification file in place')

    # Expand subcommand
    expandParser = subparsers.add_parser('expand',
        help='Expand polymorphic tables into concrete per-signature tables')
    expandParser.add_argument('--spec', dest='specFile',
        help='Specification Markdown document')
    expandParser.add_argument('--nodedefs', dest='nodedefs', action='store_true',
        help='Add nodedef name subheaders to expanded tables')
    expandParser.add_argument('--output', dest='outputFile',
        help='Path to write the output document. Defaults to stdout.')
    expandParser.add_argument('--inplace', dest='inplace', action='store_true',
        help='Modify the specification file in place')

    opts = parser.parse_args()

    if not opts.command:
        parser.print_help()
        for sub in subparsers.choices.values():
            print()
            sub.print_help()
        sys.exit(0)
    elif opts.command == 'compare':
        compareMain(opts)
    elif opts.command == 'normalize':
        normalizeMain(opts)
    elif opts.command == 'expand':
        expandMain(opts)


if __name__ == '__main__':
    main()
