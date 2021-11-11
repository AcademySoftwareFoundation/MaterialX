
#include <MaterialXRuntime/RtVersionResolver.h>

#include <regex>
#include <cmath>

namespace MaterialX
{

bool isValidIntegerVersionFormat(const std::string& versionFormat)
{
    const std::regex formatIntegerNumberingRegEx("^(([a-zA-Z_]+)?[#]([a-zA-Z_]+)?)$");
    const bool isIntegerVersioningFormat = std::regex_match(versionFormat, formatIntegerNumberingRegEx);
    return isIntegerVersioningFormat;
}

bool isValidFloatVersionFormat(const std::string& versionFormat)
{
    const std::regex formatFloatNumberingRegEx("^(([a-zA-Z_]+)?[.][#]+([a-zA-Z_]+)?)$");
    const bool isFloatVersioningFormat = std::regex_match(versionFormat, formatFloatNumberingRegEx);
    return isFloatVersioningFormat;
}

bool isValidVersionFormat(const std::string& versionFormat)
{
    return (isValidIntegerVersionFormat(versionFormat) || isValidFloatVersionFormat(versionFormat));
}

std::string getFormattedVersionString(const std::string& versionNumber, const std::string& versionFormat)
{
    std::regex numberMaskRegEx("[.]?[#]+");
    std::string result = versionFormat;
    if (isValidVersionFormat(versionFormat)) {
        result = std::regex_replace(result, numberMaskRegEx, versionNumber);
        const char decimalComma = ',';
        const char delim = '_';
        std::replace(result.begin(), result.end(), decimalComma, delim);
        return result;
    } else {
        return result;
    }
}

int getVersionFormatDecimalPrecision(const std::string& versionFormat)
{
    if (isValidFloatVersionFormat(versionFormat)) {
        size_t digitsCount = std::count(versionFormat.begin(), versionFormat.end(), '#');
        return (int)digitsCount;
    } else {
        return 0;
    }
}

double getVersionIncrementStep(const int decimalPrecision)
{
    if (decimalPrecision > 0) {
        const double stepWhole = std::pow(10, (int)decimalPrecision);
        const double incrementStep = 1.0/stepWhole;
        return incrementStep;
    } else {
        return 1.0;
    }
}

} // namespace MaterialX