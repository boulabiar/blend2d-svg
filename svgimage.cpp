#include <chrono>
#include <filesystem>

#include "svg/svg.h"
#include "svg/viewport.h"

#include "app/mappedfile.h"


using namespace waavs;

// Create one of these first, so factory constructor will run
//SVGFactory gSVG;

// Reference to currently active document
std::shared_ptr<SVGDocument> gDoc{ nullptr };

FontHandler gFontHandler{};



static void loadFontDirectory(const char* dir)
{

	const std::filesystem::path fontPath(dir);

	for (const auto& dir_entry : std::filesystem::directory_iterator(fontPath))
	{
		if (dir_entry.is_regular_file())
		{
			if ((dir_entry.path().extension() == ".ttf") ||
				(dir_entry.path().extension() == ".otf") ||
				(dir_entry.path().extension() == ".TTF"))
			{
				BLFontFace ff{};
				bool success = gFontHandler.loadFontFace(dir_entry.path().generic_string().c_str(), ff);
				//if (success)
 			        //  printf("success\n");
			}
			
		}
	}
}


static void setupFonts()
{
	loadFontDirectory("/usr/share/fonts/truetype/freefont/");

}



#define CAN_WIDTH 800
#define CAN_HEIGHT 600


int main(int argc, char **argv)
{

	printf("argc: %d\n", argc);
	
	if (argc < 2)
    {
        printf("Usage: svgimage <xml file>  [output file]\n");
        return 1;
    }
	
	setupFonts();

    // create an mmap for the specified file
    const char* filename = argv[1];

	auto mapped = waavs::MappedFile::create_shared(filename);
    
	// if the mapped file does not exist, return
	if (mapped == nullptr || !mapped->isValid())
	{
		printf("File not found: %s\n", filename);
		return 1;
	}
    
	auto c_start = std::chrono::steady_clock::now();

	printf("Size: %ld\n", mapped->size());
	ByteSpan mappedSpan(mapped->data(), mapped->size());
        gDoc = SVGFactory::createFromChunk(mappedSpan, &gFontHandler, CAN_WIDTH, CAN_HEIGHT, 300.0);


    auto c_dom = std::chrono::steady_clock::now();
	auto c_duration = std::chrono::duration_cast<std::chrono::microseconds>(c_dom - c_start);
	printf("Parsing dom duration: %ld \n", c_duration.count());

    if (gDoc == nullptr)
        return 1;

	// Now create a drawing context so we can
	// do the rendering
	IRenderSVG ctx(&gFontHandler);
	BLImage img(CAN_WIDTH, CAN_HEIGHT, BL_FORMAT_PRGB32);

	// Attach the drawing context to the image
	// We MUST do this before we perform any other
	// operations, including the transform
	ctx.attach(img);
	ctx.clearAll();

	
	// Create a rectangle the size of the BLImage we want to render into
	BLRect surfaceFrame{ 0, 0, CAN_WIDTH, CAN_HEIGHT };

	
	
	// Now that we've processed the document, we have correct sizing
	// Get the frame size from the document
	// This is the extent of the document, in user units
	BLRect sceneFrame = gDoc->getBBox();
	printf("viewport: %3.0f %3.0f %3.0f %3.0f\n", sceneFrame.x, sceneFrame.y, sceneFrame.w, sceneFrame.h);

	
	// At this point, we could just do the render, but we go further
	// and create a viewport, which will handle the scaling and translation
	// This will essentially do a 'scale to fit'
	ViewportTransformer vp{};
	vp.viewBoxFrame(sceneFrame);
	vp.viewportFrame(surfaceFrame);
	
	


	// apply the viewport's sceneToSurface transform to the context
	auto res = ctx.setTransform(vp.viewBoxToViewportTransform());
	//printf("setTransform RESULT: %d\n", res);
	

	c_start = std::chrono::steady_clock::now();

	// Render the document into the context
	//ctx.noStroke();
	gDoc->draw(&ctx, gDoc.get());
	ctx.detach();

	auto c_draw = std::chrono::steady_clock::now();
	c_duration = std::chrono::duration_cast<std::chrono::microseconds>(c_draw - c_start);
	printf("Drawing duration: %ld\n", c_duration.count());

	
	// Save the image from the drawing context out to a file
	// or do whatever you're going to do with it
	const char* outfilename = nullptr;

	
	if (argc >= 3)
		outfilename = argv[2];
	else 
		outfilename = "output.png";

	img.writeToFile(outfilename);


    return 0;
}
