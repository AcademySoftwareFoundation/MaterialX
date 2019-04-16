//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//


#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <MaterialXRender/Util.h>
#include <MaterialXRender/ExceptionShaderValidationError.h>

#include <MaterialXTest/RenderUtil.h>

#ifdef MATERIALX_BUILD_CONTRIB
#include <MaterialXContrib/Handlers/TinyEXRImageLoader.h>
#endif
#ifdef MATERIALX_BUILD_OIIO
#include <MaterialXRender/OiioImageLoader.h>
#endif
#include <MaterialXRender/StbImageLoader.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/TinyObjLoader.h>

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <ctime>

namespace mx = MaterialX;

#define LOG_TO_FILE

struct GeomHandlerTestOptions
{
    mx::GeometryHandlerPtr geomHandler;
    std::ofstream* logFile;

    mx::StringSet testExtensions;
    mx::StringVec skipExtensions;
};

void testGeomHandler(GeomHandlerTestOptions& options)
{
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/");
    mx::FilePathVec files;

    unsigned int loadFailed = 0;
    for (const std::string& extension : options.testExtensions)
    {
        if (options.skipExtensions.end() != std::find(options.skipExtensions.begin(), options.skipExtensions.end(), extension))
        {
            continue;
        }
        files = imagePath.getFilesInDirectory(extension);
        for (const mx::FilePath& file : files)
        {
            const mx::FilePath filePath = imagePath / file;
            mx::ImageDesc desc;
            bool loaded = options.geomHandler->loadGeometry(filePath);
            if (options.logFile)
            {
                *(options.logFile) << "Loaded image: " << filePath.asString() << ". Loaded: " << loaded << std::endl;
            }
            if (!loaded)
            {
                loadFailed++;
            }
        }
    }
    CHECK(loadFailed == 0);
}

TEST_CASE("Render: Geometry Handler Load", "[rendercore]")
{
    std::ofstream geomHandlerLog;
    geomHandlerLog.open("render_geom_handler_test.txt");
    bool geomLoaded = false;
    try
    {
        geomHandlerLog << "** Test TinyOBJ geom loader **" << std::endl;
        mx::TinyObjLoaderPtr loader = mx::TinyObjLoader::create();
        mx::GeometryHandlerPtr handler = mx::GeometryHandler::create();
        handler->addLoader(loader);

        GeomHandlerTestOptions options;
        options.logFile = &geomHandlerLog;
        options.geomHandler = handler;
        handler->supportedExtensions(options.testExtensions);
        testGeomHandler(options);

        geomLoaded = true;
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            geomHandlerLog << e.what() << " " << error << std::endl;
        }
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    CHECK(geomLoaded);
    geomHandlerLog.close();
}


struct ImageHandlerTestOptions
{
    mx::ImageHandlerPtr imageHandler;
    std::ofstream* logFile;

    mx::StringSet testExtensions;
    mx::StringVec skipExtensions;
};

void testImageHandler(ImageHandlerTestOptions& options)
{
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Images/");
    mx::FilePathVec files;

    unsigned int loadFailed = 0;
    for (const std::string& extension : options.testExtensions)
    {
        if (options.skipExtensions.end() != std::find(options.skipExtensions.begin(), options.skipExtensions.end(), extension))
        {
            continue;
        }
        files = imagePath.getFilesInDirectory(extension);
        for (const mx::FilePath& file : files)
        {
            const mx::FilePath filePath = imagePath / file;
            mx::ImageDesc desc;
            bool loaded = options.imageHandler->acquireImage(filePath, desc, false, nullptr);
            desc.freeResourceBuffer();
            CHECK(!desc.resourceBuffer);
            if (options.logFile)
            {
                *(options.logFile) << "Loaded image: " << filePath.asString() << ". Loaded: " << loaded << std::endl;
            }
            if (!loaded)
            {
                loadFailed++;
            }
        }
    }
    CHECK(loadFailed == 0);
}

TEST_CASE("Render: Image Handler Load", "[rendercore]")
{
    std::ofstream imageHandlerLog;
    imageHandlerLog.open("render_image_handler_test.txt");
    bool imagesLoaded = false;
    try
    {
        // Create a stock color image
        mx::ImageHandlerPtr imageHandler = mx::ImageHandler::create(nullptr);
        mx::Color4 color(1.0f, 0.0f, 0.0f, 1.0f);
        mx::ImageDesc desc;
        bool createdColorImage = imageHandler->createColorImage(color, desc);
        CHECK(!createdColorImage);
        desc.width = 1;
        desc.height = 1;
        desc.channelCount = 3;
        createdColorImage = imageHandler->createColorImage(color, desc);
        CHECK(createdColorImage);
        desc.freeResourceBuffer();
        CHECK(!desc.resourceBuffer);

        ImageHandlerTestOptions options;
        options.logFile = &imageHandlerLog;

        imageHandlerLog << "** Test STB image loader **" << std::endl;
        mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
        imageHandler->addLoader(stbLoader);
        options.testExtensions = stbLoader->supportedExtensions();
        options.imageHandler = imageHandler;
        testImageHandler(options);

#if defined(MATERIALX_BUILD_OIIO) && defined(OPENIMAGEIO_ROOT_DIR)
        imageHandlerLog << "** Test OpenImageIO image loader **" << std::endl;
        mx::OiioImageLoaderPtr oiioLoader = mx::OiioImageLoader::create();
        mx::ImageHandlerPtr imageHandler3 = mx::ImageHandler::create(nullptr);
        imageHandler3->addLoader(oiioLoader);
        options.testExtensions = oiioLoader->supportedExtensions();
        options.imageHandler = imageHandler3;
        // Getting libpng warning: iCCP: known incorrect sRGB profile for some reason. TBD.
        options.skipExtensions.push_back("gif");
        testImageHandler(options);
#endif
        imagesLoaded = true;
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            imageHandlerLog << e.what() << " " << error << std::endl;
        }
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    CHECK(imagesLoaded);
    imageHandlerLog.close();
}
