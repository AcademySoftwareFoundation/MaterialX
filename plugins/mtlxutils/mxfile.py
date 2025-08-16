'''
    Set of MaterialX I/O utilities
    - Library document creation
    - Working document creation, and write to file and string
    - Dependent element filtering (elements with source URIs)

    Grouped under a MtlxFile class

    Requires: MaterialX package
'''
#import sys, os, subprocess
import MaterialX as mx
#from .mxbase import *

class MtlxFile:

    @staticmethod
    def createLibraryDocument(librarySearchPath = mx.FileSearchPath(), libFolders = []):
        '''
        Create library document containing standard libraries as well as
        any libraries specified as input arguments.
        '''
        lib = mx.createDocument()
        status = ''

        searchPath = mx.getDefaultDataSearchPath()
        searchPath.append(librarySearchPath)

        libraryFolders = mx.getDefaultDataLibraryFolders()
        libraryFolders.extend(libFolders)
        try:
            libFiles = mx.loadLibraries(libraryFolders, searchPath, lib)
            status = '- Loaded %d library definitions from %d files' % (len(lib.getNodeDefs()), len(libFiles))
        except mx.ExceptionFileMissing as err:
            status = '- Failed to load library definitions: "%s"' % err
        
        return lib, libFiles, status
    
    @staticmethod
    def createWorkingDocument(librarySearchPath = mx.FileSearchPath(), libFolders = []):
        '''
        Create working document with libraries.
        Returns new document and list of default data library files loaded.
        '''
        stdlib, libFiles, status = MtlxFile.createLibraryDocument(librarySearchPath, libFolders)

        doc = mx.createDocument()
        if stdlib:
            doc.setDataLibrary(stdlib)
        return doc, libFiles, status

    @staticmethod
    def skipLibraryElement(elem):
        '''
        Predicate to remove elements which are referenced in. That is has a source URI.
        '''
        return not elem.hasSourceUri()
    
    @staticmethod
    def removeReferencedElements(doc):
        '''
        Remove any elements which are referenced in. That is has a source URI.
        '''
        for elem in doc.getChildren():
            if elem.hasSourceUri():
                doc.removeChild(elem.getName())

    @staticmethod
    def writeDocumentToFile(doc, filename, predicate=skipLibraryElement):
        '''
        Write a document to file with a given filter predicate.
        The default is to skip elements from libraries
        '''
        writeOptions = mx.XmlWriteOptions()
        writeOptions.writeXIncludeEnable = False
        writeOptions.elementPredicate = predicate
        mx.writeToXmlFile(doc, filename, writeOptions)

    @staticmethod
    def writeDocumentToString(doc, predicate=skipLibraryElement):
        '''
        Write a document to string with a given filter predicate.
        The default is to skip elements from libraries
        '''
        writeOptions = mx.XmlWriteOptions()
        writeOptions.writeXIncludeEnable = False
        writeOptions.elementPredicate = predicate
        documentContents = mx.writeToXmlString(doc, writeOptions)
        return documentContents
    
    @staticmethod
    def removeLayout(element):
        '''
        Remove any layout attributes from all elements in a document
        '''
        layoutAttributes = ['xpos', 'ypos']

        for elem in element.traverseTree():
            for attr in layoutAttributes:
                elem.removeAttribute(attr)
