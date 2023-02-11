#ifndef JSE_TEXTURE_H
#define JSE_TEXTURE_H

class JseTexture {
public:
	JseString name;
	JseImageID imageId;
	JseFormat format;
	int width, height;
	int levelCount;
	int layerCount;
};
#endif // !JSE_TEXTURE_H
