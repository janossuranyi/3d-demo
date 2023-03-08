#ifndef JSE_TEXTURE_H
#define JSE_TEXTURE_H

namespace js {

	class Texture {
	public:
		JsString name;
		JseImageID imageId;
		JseFormat format;
		int width, height;
		int levelCount;
		int layerCount;
	};
}
#endif // !JSE_TEXTURE_H
