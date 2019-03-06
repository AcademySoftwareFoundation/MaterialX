//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef GENSHADER_UTIL_H
#define GENSHADER_UTIL_H

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

namespace mx = MaterialX;

namespace GenShaderUtil
{
    //
    // Load in a library. Sets the URI before import
    //
    void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc);

    //
    // Loads all the MTLX files below a given library path
    //
    void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc,
        const std::set<std::string>* excludeFiles = nullptr);
    
    //
    // Get source content, source path and resolved paths for
    // an implementation
    //
    bool getShaderSource(mx::GenContext& context, 
                         const mx::ImplementationPtr implementation,
                         mx::FilePath& sourcePath,
                         mx::FilePath& resolvedPath,
                         std::string& sourceContents);

    // Find all light shaders in a document and register them with a hardware shader generator
    void registerLightType(mx::DocumentPtr doc, mx::GenContext& context);

    // Check that implementations exist for all nodedefs supported per generator
    void checkImplementations(mx::GenContext& context, 
                              const std::set<std::string>& generatorSkipNodeTypes,
                              const std::set<std::string>& generatorSkipNodeDefs);

    // Utility test to  check unique name generation on a shader generator
    void testUniqueNames(mx::GenContext& context, const std::string& stage);

    // Test code generation for a given element
    bool generateCode(mx::GenContext& context, const std::string& shaderName, mx::TypedElementPtr element,
                      std::ostream& log, std::vector<std::string>testStages);

    // Utility class to handle testing of shader generators.
    // Currently only tests source code generation.
    class ShaderGeneratorTester
    {
    public:
        ShaderGeneratorTester(const mx::FilePath& testRootPath, const mx::FilePath& libSearchPath, 
                              const mx::FileSearchPath& srcSearchPath, const mx::FilePath& logFilePath)
        {
            _logFilePath = logFilePath;
            _libSearchPath = libSearchPath;
            _srcSearchPath = srcSearchPath;
            _testRootPath = testRootPath;
        }

        ~ShaderGeneratorTester()
        {
        }

        // Generator is required from derived class
        virtual void createGenerator() = 0;

        // Stages to test is required from derived class
        virtual void setTestStages() = 0;

        // Set library files to not load when loading libraries.
        virtual void setExcludeLibraryFiles();

        // Add files in to not examine
        virtual void addSkipFiles();

        // Add nodedefs to not examine
        virtual void addSkipNodeDefs();

        void addColorManagement();
        void setupDependentLibraries();
        void testGeneration(const mx::GenOptions& generateOptions);

    protected:
        mx::ShaderGeneratorPtr _shaderGenerator;
        mx::DefaultColorManagementSystemPtr _colorManagementSystem;
        mx::DocumentPtr _dependLib;
        mx::FilePath _libSearchPath;
        mx::FileSearchPath _srcSearchPath;
        std::set<std::string> _excludeLibraryFiles;

        mx::FilePath _testRootPath;
        std::set<std::string> _skipFiles;
        std::vector<mx::DocumentPtr> _documents;
        std::vector<std::string> _documentPaths;

        mx::FilePath _logFilePath;
        std::ofstream _logFile;

        std::set<std::string> _skipNodeDefs;
        std::vector<std::string> _testStages;
    };
}

#endif
