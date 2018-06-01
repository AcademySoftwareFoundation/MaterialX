#ifndef MATERIALX_SGOPTIONS_H
#define MATERIALX_SGOPTIONS_H

namespace MaterialX
{

/// Class holding options to configure shader generation.
class SgOptions
{
public:
	SgOptions();

    // TODO: Add options for:
    //  - shader gen optimization level
    //  - graph flattening or not
    //  - etc.

	// Dummy data to prevent empty class
    int dummy;
};

} // namespace MaterialX

#endif
