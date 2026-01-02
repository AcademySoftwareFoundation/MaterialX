#!/usr/bin/env python
'''
Compare node definitions between a specification Markdown document and a
data library MaterialX document.

Report any differences between the two in their supported node sets, typed
node signatures, and default values.
'''

import argparse
import re
from dataclasses import dataclass, field
from enum import Enum
from itertools import product
from pathlib import Path
import MaterialX as mx


# -----------------------------------------------------------------------------
# Type System
# -----------------------------------------------------------------------------

def loadStandardLibraries():
    '''Load and return the standard MaterialX libraries as a document.'''
    stdlib = mx.createDocument()
    mx.loadLibraries(mx.getDefaultDataLibraryFolders(), mx.getDefaultDataSearchPath(), stdlib)
    return stdlib

def getStandardTypes(stdlib):
    '''Extract the set of standard type names from library TypeDefs.'''
    return {td.getName() for td in stdlib.getTypeDefs()}

def buildTypeGroups(stdlib):
    '''
    Build type groups from standard library TypeDefs.
    Derives colorN, vectorN, matrixNN groups from type naming patterns.
    '''
    groups = {}
    for td in stdlib.getTypeDefs():
        name = td.getName()
        # Match colorN, vectorN patterns (color3, vector2, etc.)
        match = re.match(r'^(color|vector)(\d)$', name)
        if match:
            groupName = f'{match.group(1)}N'
            groups.setdefault(groupName, set()).add(name)
            continue
        # Match matrixNN pattern (matrix33, matrix44)
        match = re.match(r'^matrix(\d)\1$', name)
        if match:
            groups.setdefault('matrixNN', set()).add(name)
    return groups

def buildTypeGroupVariables(typeGroups):
    '''Build type group variables (e.g., colorM from colorN) for "must differ" constraints.'''
    variables = {}
    for groupName in typeGroups:
        if groupName.endswith('N') and not groupName.endswith('NN'):
            variantName = groupName[:-1] + 'M'
            variables[variantName] = groupName
    return variables

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
        return set(), None

    typeStr = typeStr.strip()

    # Handle "Same as X" and "Same as X or Y" references
    sameAsMatch = re.match(r'^Same as\s+`?(\w+)`?(?:\s+or\s+(.+))?$', typeStr, re.IGNORECASE)
    if sameAsMatch:
        refPort = sameAsMatch.group(1)
        extraTypes = sameAsMatch.group(2)
        extraSet = set()
        if extraTypes:
            extraSet, _ = parseSpecTypes(extraTypes)
        return extraSet, refPort

    # Normalize "or" to comma: "X or Y" -> "X, Y", "X, Y, or Z" -> "X, Y, Z"
    normalized = re.sub(r',?\s+or\s+', ', ', typeStr)

    result = set()
    for t in normalized.split(','):
        t = t.strip()
        if t:
            result.add(t)

    return result, None

def expandTypeSet(types, typeGroups, typeGroupVariables):
    '''Expand type groups to concrete types. Returns list of (concreteType, groupName) tuples.'''
    result = []
    for t in types:
        if t in typeGroups:
            for concrete in typeGroups[t]:
                result.append((concrete, t))
        elif t in typeGroupVariables:
            baseGroup = typeGroupVariables[t]
            for concrete in typeGroups[baseGroup]:
                result.append((concrete, t))
        else:
            result.append((t, None))
    return result


# -----------------------------------------------------------------------------
# Data Classes
# -----------------------------------------------------------------------------

class MatchType(Enum):
    '''Types of signature matches between spec and library.'''
    EXACT = 'exact'  # Identical inputs and outputs
    DIFFERENT_INPUTS = 'different_inputs'  # Same outputs but different inputs

class DiffType(Enum):
    '''Categories of differences between spec and library, with display labels.'''

    # Invalid specification entries
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
class PortInfo:
    '''Information about an input or output port from the specification.'''
    name: str
    types: set = field(default_factory=set)
    typeRef: str = None  # For "Same as X" references
    default: str = None  # Spec default string (before type-specific expansion)

@dataclass(frozen=True)
class NodeSignature:
    '''A typed combination of inputs and outputs, corresponding to one nodedef.'''
    inputs: tuple   # ((name, type), ...) sorted for hashing
    outputs: tuple  # ((name, type), ...) sorted for hashing
    _displayInputs: tuple = None
    _displayOutputs: tuple = None

    @classmethod
    def create(cls, inputs, outputs):
        '''Create a NodeSignature from input/output dicts of name -> type.'''
        return cls(
            inputs=tuple(sorted(inputs.items())),
            outputs=tuple(sorted(outputs.items())),
            _displayInputs=tuple(inputs.items()),
            _displayOutputs=tuple(outputs.items()),
        )

    def __hash__(self):
        return hash((self.inputs, self.outputs))

    def __eq__(self, other):
        if not isinstance(other, NodeSignature):
            return False
        return self.inputs == other.inputs and self.outputs == other.outputs

    def __str__(self):
        insStr = ', '.join(f'{n}:{t}' for n, t in self._displayInputs)
        outsStr = ', '.join(f'{n}:{t}' for n, t in self._displayOutputs)
        return f'({insStr}) -> {outsStr}'

@dataclass
class NodeInfo:
    '''A node and its supported signatures.'''
    name: str
    signatures: set = field(default_factory=set)
    _specInputs: dict = field(default_factory=dict)  # For default value comparison

@dataclass
class Difference:
    '''A difference found between spec and data library.'''
    diffType: DiffType
    node: str
    port: str = None
    signature: NodeSignature = None
    extraInLib: tuple = None
    extraInSpec: tuple = None
    valueType: str = None
    specDefault: str = None
    libDefault: str = None

def formatDifference(diff):
    '''Format a Difference for display, returning a list of lines.'''
    # Default mismatch
    if diff.diffType == DiffType.DEFAULT_MISMATCH:
        return [
            f'  {diff.node}.{diff.port} ({diff.valueType}):',
            f'    Signature:            {diff.signature}',
            f'    Spec default:         {diff.specDefault}',
            f'    Data library default: {diff.libDefault}',
        ]

    # Different input sets
    if diff.diffType == DiffType.SIGNATURE_DIFFERENT_INPUTS:
        lines = [f'  {diff.node}: {diff.signature}']
        if diff.extraInLib:
            extraStr = ', '.join(f'{n}:{t}' for n, t in diff.extraInLib)
            lines.append(f'    Extra in library: {extraStr}')
        if diff.extraInSpec:
            extraStr = ', '.join(f'{n}:{t}' for n, t in diff.extraInSpec)
            lines.append(f'    Extra in spec:    {extraStr}')
        return lines

    # Signature mismatch (missing in spec or library)
    if diff.signature:
        return [f'  {diff.node}: {diff.signature}']

    # Spec validation error with port
    if diff.port:
        return [f'  {diff.node}.{diff.port}']

    # Node-level difference or simple spec validation error
    return [f'  {diff.node}']


# -----------------------------------------------------------------------------
# Default Value Utilities
# -----------------------------------------------------------------------------

def buildGeompropNames(stdlib):
    '''Extract geomprop names from standard library GeomPropDefs.'''
    return {gpd.getName() for gpd in stdlib.getGeomPropDefs()}

def getComponentCount(typeName):
    '''Get the number of components for a MaterialX type, or None if unknown.'''
    if typeName in ('float', 'integer', 'boolean'):
        return 1
    # Match colorN, vectorN patterns
    match = re.match(r'^(color|vector)(\d)$', typeName)
    if match:
        return int(match.group(2))
    # Match matrixNN pattern
    match = re.match(r'^matrix(\d)(\d)$', typeName)
    if match:
        return int(match.group(1)) * int(match.group(2))
    return None

def expandDefaultPlaceholder(placeholder, typeName):
    '''Expand a placeholder (0, 1, 0.5) to a type-appropriate value string.'''
    count = getComponentCount(typeName)
    if count is None:
        return None

    if placeholder == '0':
        if typeName == 'boolean':
            return 'false'
        return ', '.join(['0'] * count)

    if placeholder == '1':
        if typeName == 'boolean':
            return 'true'
        # Identity matrices, not all-ones
        if typeName == 'matrix33':
            return '1, 0, 0, 0, 1, 0, 0, 0, 1'
        if typeName == 'matrix44':
            return '1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1'
        return ', '.join(['1'] * count)

    if placeholder == '0.5':
        if typeName in ('integer', 'boolean'):
            return None  # 0.5 doesn't apply to these types
        return ', '.join(['0.5'] * count)

    return None

def parseSpecDefault(value, specDefaultNotation):
    '''Parse specification default value notation into normalized form.'''
    if value is None:
        return None
    value = value.strip()
    return specDefaultNotation.get(value, value)

def expandSpecDefaultToValue(specDefault, valueType, geompropNames):
    '''Parse a spec default to a typed MaterialX value. Returns (value, isGeomprop).'''
    if specDefault is None or specDefault == '':
        return None, False

    # Handle geomprop references - these are compared as strings, not typed values
    if specDefault in geompropNames:
        return specDefault, True

    # Expand placeholder values to type-appropriate strings
    expansion = expandDefaultPlaceholder(specDefault, valueType)
    if expansion is not None:
        specDefault = expansion

    # Parse to typed value using MaterialX
    try:
        return mx.createValueFromStrings(specDefault, valueType), False
    except Exception:
        return None, False

def formatDefaultValue(value, valueType, geompropNames):
    '''Format a default value for display using spec notation (__zero__, etc.).'''
    if value is None:
        return 'None'

    # Handle string values (geomprops, empty strings, etc.)
    if isinstance(value, str):
        if value in geompropNames:
            return f'_{value}_'
        return '__empty__' if value == '' else value

    # Check if value matches a standard constant (__zero__, __one__, __half__)
    for placeholder, display in [('0', '__zero__'), ('1', '__one__'), ('0.5', '__half__')]:
        expansion = expandDefaultPlaceholder(placeholder, valueType)
        if expansion is None:
            continue
        try:
            if value == mx.createValueFromStrings(expansion, valueType):
                return display
        except Exception:
            pass

    # Fall back to string representation
    return str(value)


# -----------------------------------------------------------------------------
# Markdown Table Parsing
# -----------------------------------------------------------------------------

def parseMarkdownTable(lines, startIdx):
    '''Parse a markdown table. Returns (rows, columnMismatchCount, endIndex).'''
    table = []
    headers = []
    columnMismatchCount = 0
    idx = startIdx

    # Parse header row
    if idx < len(lines) and '|' in lines[idx]:
        headerLine = lines[idx].strip()
        headers = [h.strip().strip('`') for h in headerLine.split('|')[1:-1]]
        idx += 1
    else:
        return [], 0, startIdx

    # Skip separator row
    if idx < len(lines) and '|' in lines[idx] and '-' in lines[idx]:
        idx += 1
    else:
        return [], 0, startIdx

    # Parse data rows
    while idx < len(lines):
        line = lines[idx].strip()
        if not line or not line.startswith('|'):
            break

        cells = [c.strip().strip('`') for c in line.split('|')[1:-1]]
        if len(cells) == len(headers):
            row = {headers[i].lower(): cells[i] for i in range(len(headers))}
            table.append(row)
        else:
            columnMismatchCount += 1
        idx += 1

    return table, columnMismatchCount, idx


# -----------------------------------------------------------------------------
# Specification Document Parsing
# -----------------------------------------------------------------------------

def isValidTypeGroupAssignment(driverNames, combo, typeGroupVariables):
    '''
    Check if type assignments satisfy group constraints (e.g., colorN ports must
    match, colorM must differ from colorN). Returns (isValid, typeAssignment).
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

        # For group variables (colorM), get the base group (colorN)
        baseGroup = typeGroupVariables.get(groupName, groupName)
        isVariable = groupName in typeGroupVariables

        # Check consistency: all uses of the same group must have same concrete type
        if groupName in groupAssignments:
            if groupAssignments[groupName] != concreteType:
                return False, None
        else:
            groupAssignments[groupName] = concreteType

        # For variables: must differ from base group if base is already assigned
        if isVariable and baseGroup in groupAssignments:
            if groupAssignments[baseGroup] == concreteType:
                return False, None

    return True, typeAssignment

def expandSpecSignatures(inputs, outputs, typeGroups, typeGroupVariables):
    '''
    Expand spec port definitions into concrete NodeSignatures.
    Handles type groups, type group variables, and "Same as X or Y" patterns.
    '''
    allPorts = {**inputs, **outputs}

    # Identify driver ports and their type options
    # - Ports with explicit types (no typeRef): use those types
    # - Ports with both types AND typeRef ("Same as X or Y"): explicit types OR inherit from typeRef
    drivers = {}
    for name, port in allPorts.items():
        if port.types and not port.typeRef:
            # Normal driver: explicit types only
            drivers[name] = expandTypeSet(port.types, typeGroups, typeGroupVariables)
        elif port.types and port.typeRef:
            # "Same as X or Y" pattern: explicit types OR inherit from typeRef
            expanded = expandTypeSet(port.types, typeGroups, typeGroupVariables)
            expanded.append((None, None))  # None means "inherit from typeRef"
            drivers[name] = expanded

    if not drivers:
        return set()

    # Generate all combinations of driver types
    driverNames = sorted(drivers.keys())
    driverTypeLists = [drivers[n] for n in driverNames]

    signatures = set()
    for combo in product(*driverTypeLists):
        # Validate type group constraints (skip None values which will be resolved via typeRef)
        valid, typeAssignment = isValidTypeGroupAssignment(driverNames, combo, typeGroupVariables)
        if not valid:
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
        signatures.add(NodeSignature.create(sigInputs, sigOutputs))

    return signatures

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

def resolvePortTypeRefs(ports):
    '''Resolve type references between ports by copying types. Modifies ports in place.'''
    # Limit iterations to handle circular refs
    for _ in range(10):
        changed = False
        for port in ports.values():
            if port.typeRef:
                refPort = ports.get(port.typeRef)
                if refPort and refPort.types:
                    port.types.update(refPort.types)
                    port.typeRef = None
                    changed = True
        if not changed:
            break

def parseSpecDocument(specPath, stdlib, geompropNames):
    '''Parse a specification markdown document. Returns (nodes, invalidEntries).'''
    # Build type system data from stdlib
    standardTypes = getStandardTypes(stdlib)
    typeGroups = buildTypeGroups(stdlib)
    typeGroupVariables = buildTypeGroupVariables(typeGroups)

    # Build derived values for validation and parsing
    knownTypes = standardTypes | set(typeGroups.keys()) | set(typeGroupVariables.keys())
    specDefaultNotation = {
        '__zero__': '0',
        '__one__': '1',
        '__half__': '0.5',
        '__empty__': '',
    }
    for name in geompropNames:
        specDefaultNotation[f'_{name}_'] = name

    nodes = {}
    invalidEntries = []

    with open(specPath, 'r', encoding='utf-8') as f:
        content = f.read()

    lines = content.split('\n')
    currentNode = None
    currentTableInputs = {}
    currentTableOutputs = {}
    idx = 0

    def finalizeCurrentTable():
        '''Expand current table to signatures and add to node.'''
        nonlocal currentTableInputs, currentTableOutputs
        if currentNode and (currentTableInputs or currentTableOutputs):
            node = nodes[currentNode]
            # Expand to signatures (do NOT pre-resolve typeRefs - expansion handles them)
            tableSigs = expandSpecSignatures(currentTableInputs, currentTableOutputs, typeGroups, typeGroupVariables)
            node.signatures.update(tableSigs)
            # Merge input port info for default comparison (resolve types for defaults)
            allPorts = {**currentTableInputs, **currentTableOutputs}
            resolvePortTypeRefs(allPorts)
            for name, port in currentTableInputs.items():
                if name not in node._specInputs:
                    node._specInputs[name] = port
                else:
                    node._specInputs[name].types.update(port.types)
        currentTableInputs = {}
        currentTableOutputs = {}

    while idx < len(lines):
        line = lines[idx]

        # Look for node headers (### `nodename`)
        nodeMatch = re.match(r'^###\s+`([^`]+)`', line)
        if nodeMatch:
            # Finalize previous table before switching nodes
            finalizeCurrentTable()
            currentNode = nodeMatch.group(1)
            if currentNode not in nodes:
                nodes[currentNode] = NodeInfo(name=currentNode)
            idx += 1
            continue

        # Look for tables when we have a current node
        if currentNode and '|' in line and 'Port' in line:
            # Finalize previous table before starting new one
            finalizeCurrentTable()

            rows, columnMismatchCount, idx = parseMarkdownTable(lines, idx)

            # Track column count mismatches
            for _ in range(columnMismatchCount):
                invalidEntries.append(Difference(
                    diffType=DiffType.SPEC_COLUMN_MISMATCH,
                    node=currentNode,
                ))

            if rows:
                for row in rows:
                    portName = row.get('port', '').strip('`*')

                    # Track empty port names
                    if not portName:
                        invalidEntries.append(Difference(
                            diffType=DiffType.SPEC_EMPTY_PORT_NAME,
                            node=currentNode,
                        ))
                        continue

                    portType = row.get('type', '')
                    portDefault = row.get('default', '')
                    portDesc = row.get('description', '')

                    types, typeRef = parseSpecTypes(portType)

                    # Track unrecognized types
                    if types - knownTypes:
                        invalidEntries.append(Difference(
                            diffType=DiffType.SPEC_UNRECOGNIZED_TYPE,
                            node=currentNode,
                            port=portName,
                        ))

                    # Determine if this is an output port
                    isOutput = portName == 'out' or portDesc.lower().startswith('output')
                    target = currentTableOutputs if isOutput else currentTableInputs

                    # Create port info for this table
                    portInfo = target.setdefault(portName, PortInfo(
                        name=portName,
                        default=parseSpecDefault(portDefault, specDefaultNotation),
                    ))
                    portInfo.types.update(types)
                    if typeRef and not portInfo.typeRef:
                        portInfo.typeRef = typeRef
            continue

        idx += 1

    # Finalize the last table
    finalizeCurrentTable()

    return nodes, invalidEntries


# -----------------------------------------------------------------------------
# Data Library Loading
# -----------------------------------------------------------------------------

def loadDataLibrary(mtlxPath):
    '''Load a data library MTLX document. Returns (nodes, defaults).'''
    doc = mx.createDocument()
    mx.readFromXmlFile(doc, str(mtlxPath))

    nodes = {}
    defaults = {}  # (nodeName, signature) -> {portName -> (value, isGeomprop)}

    for nodedef in doc.getNodeDefs():
        nodeName = nodedef.getNodeString()
        node = nodes.setdefault(nodeName, NodeInfo(name=nodeName))

        # Build signature from this nodedef
        sigInputs = {inp.getName(): inp.getType() for inp in nodedef.getInputs()}
        sigOutputs = {out.getName(): out.getType() for out in nodedef.getOutputs()}
        sig = NodeSignature.create(sigInputs, sigOutputs)
        node.signatures.add(sig)

        # Store defaults keyed by signature
        sigDefaults = {}
        for inp in nodedef.getInputs():
            if inp.hasDefaultGeomPropString():
                sigDefaults[inp.getName()] = (inp.getDefaultGeomPropString(), True)
            elif inp.getValue() is not None:
                sigDefaults[inp.getName()] = (inp.getValue(), False)
        if sigDefaults:
            defaults[(nodeName, sig)] = sigDefaults

    return nodes, defaults


# -----------------------------------------------------------------------------
# Comparison Logic
# -----------------------------------------------------------------------------

def compareSignatureDefaults(nodeName, signature, specNode, libDefaults, geompropNames):
    '''Compare default values for a matching signature. Returns list of Differences.'''
    differences = []

    for portName, valueType in signature.inputs:
        specPort = specNode._specInputs.get(portName)
        if not specPort or not specPort.default:
            continue

        specValue, specIsGeomprop = expandSpecDefaultToValue(specPort.default, valueType, geompropNames)
        libValue, libIsGeomprop = libDefaults.get(portName, (None, False))

        # Skip if either value is unavailable
        if specValue is None or libValue is None:
            continue

        # Compare values (geomprops compare as strings, typed values use equality)
        valuesMatch = (specValue == libValue) if (specIsGeomprop == libIsGeomprop) else False

        if not valuesMatch:
            differences.append(Difference(
                diffType=DiffType.DEFAULT_MISMATCH,
                node=nodeName,
                port=portName,
                signature=signature,
                valueType=valueType,
                specDefault=formatDefaultValue(specValue, valueType, geompropNames),
                libDefault=formatDefaultValue(libValue, valueType, geompropNames),
            ))

    return differences

def findLibraryMatch(specSig, libSigs):
    '''Find a matching library signature. Returns (matchType, libSig, extraInLib, extraInSpec).'''
    specInputs = set(specSig.inputs)
    specOutputs = set(specSig.outputs)

    for libSig in libSigs:
        libInputs = set(libSig.inputs)
        libOutputs = set(libSig.outputs)

        # Check for exact match
        if specInputs == libInputs and specOutputs == libOutputs:
            return MatchType.EXACT, libSig, None, None

        # Check for different input sets (same outputs, different inputs)
        if specOutputs == libOutputs and specInputs != libInputs:
            # If there are common inputs, verify they have the same types
            commonInputNames = {name for name, _ in specInputs} & {name for name, _ in libInputs}
            if commonInputNames:
                specInputDict = dict(specSig.inputs)
                libInputDict = dict(libSig.inputs)
                typesMatch = all(specInputDict[n] == libInputDict[n] for n in commonInputNames)
                if not typesMatch:
                    continue  # Common inputs have different types - not a match

            extraInLib = tuple(sorted(libInputs - specInputs))
            extraInSpec = tuple(sorted(specInputs - libInputs))
            return MatchType.DIFFERENT_INPUTS, libSig, extraInLib, extraInSpec

    return None, None, None, None

def compareNodes(specNodes, libNodes, libDefaults, geompropNames, compareDefaults=False):
    '''Compare nodes between spec and library. Returns list of Differences.'''
    differences = []

    specNames = set(specNodes.keys())
    libNames = set(libNodes.keys())

    # Nodes in spec but not in library
    for nodeName in sorted(specNames - libNames):
        differences.append(Difference(
            diffType=DiffType.NODE_MISSING_IN_LIBRARY,
            node=nodeName))

    # Nodes in library but not in spec
    for nodeName in sorted(libNames - specNames):
        differences.append(Difference(
            diffType=DiffType.NODE_MISSING_IN_SPEC,
            node=nodeName))

    # Compare signatures for common nodes
    for nodeName in sorted(specNames & libNames):
        specNode = specNodes[nodeName]
        libNode = libNodes[nodeName]

        specSigs = specNode.signatures
        libSigs = libNode.signatures

        # Track which signatures have been matched
        matchedLibSigs = set()
        matchedSpecSigs = set()
        inputDiffMatches = []  # (specSig, libSig, extraInLib, extraInSpec)

        # For each spec signature, find matching library signature
        for specSig in specSigs:
            matchType, libSig, extraInLib, extraInSpec = findLibraryMatch(specSig, libSigs)

            if matchType == MatchType.EXACT:
                matchedLibSigs.add(libSig)
                matchedSpecSigs.add(specSig)
                # Compare defaults for exact matches
                if compareDefaults:
                    sigDefaults = libDefaults.get((nodeName, libSig), {})
                    differences.extend(compareSignatureDefaults(
                        nodeName, specSig, specNode, sigDefaults, geompropNames))

            elif matchType == MatchType.DIFFERENT_INPUTS:
                matchedLibSigs.add(libSig)
                matchedSpecSigs.add(specSig)
                inputDiffMatches.append((specSig, libSig, extraInLib, extraInSpec))
                # Compare defaults for different input matches too (for common ports)
                if compareDefaults:
                    sigDefaults = libDefaults.get((nodeName, libSig), {})
                    differences.extend(compareSignatureDefaults(
                        nodeName, specSig, specNode, sigDefaults, geompropNames))

        # Report different input set matches
        for specSig, libSig, extraInLib, extraInSpec in sorted(inputDiffMatches, key=lambda x: str(x[0])):
            differences.append(Difference(
                diffType=DiffType.SIGNATURE_DIFFERENT_INPUTS,
                node=nodeName,
                signature=specSig,
                extraInLib=extraInLib,
                extraInSpec=extraInSpec,
            ))

        # Spec signatures not matched by any library signature
        for specSig in sorted(specSigs - matchedSpecSigs, key=str):
            differences.append(Difference(
                diffType=DiffType.SIGNATURE_MISSING_IN_LIBRARY,
                node=nodeName,
                signature=specSig,
            ))

        # Library signatures not matched by any spec signature
        for libSig in sorted(libSigs - matchedLibSigs, key=str):
            differences.append(Difference(
                diffType=DiffType.SIGNATURE_MISSING_IN_SPEC,
                node=nodeName,
                signature=libSig,
            ))

    return differences


# -----------------------------------------------------------------------------
# Output Formatting
# -----------------------------------------------------------------------------

def printDifferences(differences):
    '''Print the differences in a formatted way.'''
    if not differences:
        print("No differences found between specification and data library.")
        return

    # Group differences by type
    byType = {}
    for diff in differences:
        byType.setdefault(diff.diffType, []).append(diff)

    print(f"\n{'=' * 70}")
    print(f"COMPARISON RESULTS: {len(differences)} difference(s) found")
    print(f"{'=' * 70}")

    for diffType in DiffType:
        if diffType not in byType:
            continue

        diffs = byType[diffType]
        print(f"\n{diffType.value} ({len(diffs)}):")
        print("-" * 50)

        for diff in diffs:
            for line in formatDifference(diff):
                print(line)


# -----------------------------------------------------------------------------
# Main Entry Point
# -----------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Compare node definitions between a specification Markdown document and a data library MaterialX document.")
    parser.add_argument('--spec', dest='specFile',
        help='Path to the specification Markdown document. Defaults to documents/Specification/MaterialX.StandardNodes.md')
    parser.add_argument('--mtlx', dest='mtlxFile',
        help='Path to the data library MaterialX document. Defaults to libraries/stdlib/stdlib_defs.mtlx')
    parser.add_argument('--defaults', dest='compareDefaults', action='store_true',
        help='Compare default values for inputs using MaterialX typed value comparison')
    parser.add_argument('--listNodes', dest='listNodes', action='store_true',
        help='List all nodes and their node signature counts')
    opts = parser.parse_args()

    # Determine file paths
    repoRoot = Path(__file__).resolve().parent.parent.parent

    specPath = Path(opts.specFile) if opts.specFile else repoRoot / 'documents' / 'Specification' / 'MaterialX.StandardNodes.md'
    mtlxPath = Path(opts.mtlxFile) if opts.mtlxFile else repoRoot / 'libraries' / 'stdlib' / 'stdlib_defs.mtlx'

    # Verify files exist
    if not specPath.exists():
        raise FileNotFoundError(f"Specification document not found: {specPath}")

    if not mtlxPath.exists():
        raise FileNotFoundError(f"MTLX document not found: {mtlxPath}")

    print(f"Comparing:")
    print(f"  Specification: {specPath}")
    print(f"  Data Library: {mtlxPath}")

    # Load standard libraries
    stdlib = loadStandardLibraries()
    geompropNames = buildGeompropNames(stdlib)

    # Parse specification
    print("\nParsing specification...")
    specNodes, invalidEntries = parseSpecDocument(specPath, stdlib, geompropNames)
    specSigCount = sum(len(n.signatures) for n in specNodes.values())
    print(f"  Found {len(specNodes)} nodes with {specSigCount} node signatures")
    if invalidEntries:
        print(f"  Found {len(invalidEntries)} invalid specification entries")

    # Load data library
    print("Loading data library...")
    libNodes, libDefaults = loadDataLibrary(mtlxPath)
    libSigCount = sum(len(n.signatures) for n in libNodes.values())
    print(f"  Found {len(libNodes)} nodes with {libSigCount} node signatures")

    # List nodes if requested
    if opts.listNodes:
        print("\nNodes in Specification:")
        for name in sorted(specNodes.keys()):
            node = specNodes[name]
            print(f"  {name}: {len(node.signatures)} signature(s)")

        print("\nNodes in Data Library:")
        for name in sorted(libNodes.keys()):
            node = libNodes[name]
            print(f"  {name}: {len(node.signatures)} signature(s)")

    # Compare nodes
    print("\nComparing node signatures...")
    differences = compareNodes(specNodes, libNodes, libDefaults, geompropNames, opts.compareDefaults)

    # Include invalid spec entries in the differences
    differences = invalidEntries + differences

    # Print differences
    printDifferences(differences)


if __name__ == '__main__':
    main()
