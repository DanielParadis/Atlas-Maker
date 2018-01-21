#include <iostream>
#include <vector>
#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;
// Hold our menu selection
char choice = 0;

// A simple struct to hold some data for our textures.
struct texture
{
	char name[256];
	int width;
	int height;
	int channels;
	unsigned char* buffer;
	// We are using a second initializer to help move the stb_image into this struct
	texture(char n[256], int w, int h, int c, unsigned char* b)
	{
		// we copy the filename that we used to load the image
		for (int i = 0; i < 256; i++)
		{
			name[i] = n[i];
		}
		width = w;
		height = h;
		channels = c;
		// We want to preserve our alpha channel, but we also want to load multiple filetypes, some of which do not support alpha
		// So what we do is we add a value of 255, or completely opaque to images that do not naturally have an alpha channel.
		// Images that have an alpha channel, like PNG, will preserve their own alpha channels.
		buffer = new unsigned char[width*height * 4];
		for (int i = 0; i < width*height * 4; i+=4)
		{
			buffer[i] = b[i];
			buffer[i + 1] = b[i + 1];
			buffer[i + 2] = b[i + 2];
			// This part right here checks to see how many channels the image has, and if there's only 3, we add our own 4th channel.
			if (channels = 3)
				buffer[i + 3] = 255;
			else
				buffer[i + 3] = b[i + 3];
		}
	}
};
// This vector will be used to store our textures. 
vector<texture> texList;
// Number of textures
int numTextures = 0;
// We want all images loaded to have the same width. setWidth will hold the width of the 1st loaded images.
// Images also need to have the same height.
int setWidth = -1;
// This is for exiting the program
bool exitProg = false;
// Add an image
void addImage();
// Generate the atlas
void genAtlas();
// list images
void listImages();
// Display our options

void displayOptions()
{
	cout << "Enter one of the following options:\n\n";
	cout << "a\tAdd image to atlas.\n";
	cout << "g\tGenerate atlas.\n";
	cout << "l\tList current files.\n";
	cout << "q\tQuit.\n";
}
// Get our choice
void getChoice()
{
	cin >> choice;
	switch (choice)
	{
	case 'a': addImage();
		break;
	case 'g': genAtlas();
		break;
	case 'l': listImages();
		break;
	case 'q': exitProg = true;
		break;
	default: cout << "Not a valid option!\n"; // If there's incorrect input, we redisplay the options and allow for another choice.
		displayOptions();
		getChoice();
	}
}
// displays some basic info
void displayInfo()
{
	cout << "Current number of Textures in Atlas: " << numTextures << endl;
	if (numTextures > 0)
	{
		cout << "Set width/height: " << setWidth << endl;
	}
}

int main(int argc, char** argv)
{
	cout << "Welcome to Atlas Maker v0.1\n\n";
	// We have a loop that will exit if we decide to exit the program.
	while (!exitProg)
	{
		// We dispplay our info, display our options, and check for input.
		displayInfo();
		displayOptions();
		getChoice();
	}
}

void addImage()
{
	cout << endl;
	// To check if we have an error
	bool err = false;
	char fileName[256];
	int width;
	int height;
	int nrChannels;
	cout << "Enter the filename or path.\n";
	cin >> fileName;
	// Load an image and ask for 4 channels if available.
	unsigned char* data = stbi_load(fileName, &width, &height, &nrChannels, 4);
	if (data)
	{
		// display the image width and height.
		cout << "Image width is: " << width << endl;
		cout << "Image height is: " << height << endl;
		// if the image isn't square then we don't continue.
		if (width != height)
		{
			cout << "Width and Height do not match. Only square textures can be used.\n";
			err = true;
		}
		// otherwise we set the allowable width.
		if (setWidth == -1)
		{
			setWidth = width;
		}
		// additional images loaded that don't match the allowable width won't be added.
		if (width != setWidth)
		{
			cout << "Only textures of equal size can be used in an atlas. The texture loaded does not match previous loaded textures.\n";
			err = true;
		}
	}
	else
	{
		// If there is no data loaded, something went wrong.
		cout << "Image failed to load. Check filename.\n";
		err = true;
	}
	if (!err)
	{
		// if there is no error then we push the data to our vector
		texList.push_back(texture(fileName, width, height, nrChannels, data));
		// we free the data
		stbi_image_free(data);
		// we increase the number of textures.
		++numTextures;
	}
	cout << endl;
}

void genAtlas()
{
	int atlasSize = 0;
	// We want a square image. We find the closest square to the number of textures we have.
	for (atlasSize; atlasSize*atlasSize < numTextures; ++atlasSize);
	cout << endl;
	cout << "Generating atlas...\n";
	cout << "Number of textures: " << numTextures << endl;
	cout << "Atlas size is: " << atlasSize << "X" << atlasSize << endl;
	cout << "Pixel width per texture is: " << setWidth << endl;
	// We need to create a buffer large enough to fit all of the image data.
	unsigned char* atlasBuffer = new unsigned char[atlasSize*atlasSize*setWidth*setWidth * 4];
	// curTex holds the value of what texture row we are in.
	int curTex = 0;
	// bufPos is the position in the final buffer that we are in
	int bufPos = 0;
	// iterates through which row we are in the atlas
	for (int atRow = 0; atRow < atlasSize; ++atRow)
	{
		// iterates through which row we are in the texture, or along the texture's y axis
		for (int texH = 0; texH < setWidth; ++texH)
		{
			// iterates through which column we are in on the atlis, or it moves on the atlas x axis
			for (int atCol = 0; atCol < atlasSize; ++atCol)
			{
				// iterates along the length/width of the texture.
				for (int texW = 0; texW < setWidth; ++texW)
				{
					// rgba is the image data byte.
					for (int rgba = 0; rgba < 4; ++rgba)
					{
						// We check to make sure that are accessing textures
						if (curTex + atCol < numTextures)
						{
							// Then we set the current buffer position to the current pixel data of our current texture.
							atlasBuffer[bufPos] = texList[curTex + atCol].buffer[(texH*setWidth * 4) + (texW * 4) + rgba];
						}
						else
						{
							// otherwise we will just write 0 and leave missing squares black.
							atlasBuffer[bufPos] = 0;
						}
						++bufPos;
					}
				}
			}
		}
		// Once we move along the x axis of the atlas, we need to add the width of the atlas to our current textures so we don't
		// look at our already written textures.
		curTex += atlasSize;
	}
	// We are going to get a file name.
	char fname[256];
	cout << "Enter a file name prefferably ending in .png\n";
	cin >> fname;
	// We save the data as a .png
	stbi_write_png(fname, atlasSize*setWidth, atlasSize*setWidth, 4, atlasBuffer, atlasSize*setWidth*4);
	cout << endl;
}

// We list loaded images here.
void listImages()
{
	cout << endl;
	// check if none have been loaded.
	if (numTextures < 1)
	{
		cout << "You have loaded zero images or there was a problem with one you entered.\n";
	}
	else
	{
		// if images are loaded, we iterate over our texture list and display the names.
		cout << "Current images loaded:\n\n";
		for (auto i : texList)
		{
			cout << i.name << endl;
		}
	}
	cout << endl;
}