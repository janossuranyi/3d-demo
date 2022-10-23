#include <string>
#include <stb_image.h>
#include "logger.h"
#include "effect_test_engine.h"

static float UNIT_BOX_POSITIONS[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

static const float UNIT_RECT_WITH_ST[6 * 4] = {
	-1.0f, 1.0,/**/0.0f, 1.0f,
	-1.0f,-1.0f,/**/0.0f,0.0f,
	1.0f,-1.0f,/**/1.0f,0.0f,
	1.0f,1.0f,/**/1.0f,1.0f,
	-1.0f,1.0f,/**/0.0f,1.0f,
	1.0f,-1.0f,/**/1.0f,0.0f
};

static const float kernels[][9] = {
	{
		0.0625f, 0.125f, 0.0625f,
		0.125f, 0.25f, 0.125f,
		0.0625f, 0.125f, 0.0625f
	},
	{
		-1.0f, -2.0f, -1.0f,
		 0.0f,  0.0f,  0.0f,
		 1.0f,  2.0f,  1.0f
	},
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
	},
	{
		-2.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 1.0f,
		 0.0f,  1.0f, 2.0f
	},
	{
		1.0f, 0.0f, -1.0f,
		2.0f, 0.0f, -2.0f,
		1.0f, 0.0f, -1.0f
	},
	{
		-1.0f, -1.0f, -1.0f,
		-1.0f,  8.0f, -1.0f,
		-1.0f, -1.0f, -1.0f
	},
	{
		-1.0f, 0.0f, 1.0f,
		-2.0f, 0.0f, 2.0f,
		-1.0f, 0.0f, 1.0f
	},
	{
		 0.0f, -1.0f,  0.0f,
		-1.0f,  5.0f, -1.0f,
		 0.0f, -1.0f,  0.0f
	},
	{
		 1.0f,  2.0f,  1.0f,
		 0.0f,  0.0f,  0.0f,
		-1.0f, -2.0f, -1.0f
	}
};

bool EngineTestEffect::Init()
{
    renderer.init(gfx::RendererType::OpenGL, 1440, 900, "test", false);

    color_attachment = 
        renderer.createTexture2D(1440, 900, gfx::TextureFormat::RGBA8, gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear, false, Memory());
    fb = 
        renderer.createFrameBuffer(std::vector<gfx::TextureHandle>{color_attachment});


	const std::vector<std::string> textures_faces = {
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"
	};

	{
		std::vector<Memory> sky_images;
		int x, y, n, img_x = 0, img_y = 0;
		stbi_set_flip_vertically_on_load(true);
		for (auto& side : textures_faces)
		{
			auto fname = ResourceManager::get_resource(side);
			uint8_t* image = stbi_load(fname.c_str(), &x, &y, &n, 3);
			if (img_x == 0)
			{
				img_x = x;
				img_y = y;
			}
			else if (img_x != x || img_y != y)
			{
				Warning("Cube faces size not equal");
			}
			sky_images.emplace_back(image, x * y * 3);
			stbi_image_free(image);
		}

		skybox =
			renderer.createTextureCubemap(img_x, img_y, gfx::TextureFormat::RGB8, gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear, true, sky_images);
	}

	{
		const size_t bufSize = sizeof(gfx::DrawVert) * NUMPOINTS;
		std::vector<gfx::DrawVert> buffer(NUMPOINTS);

		const int n = 1000, n2 = n / 2; // particles spread in the cube

		float r0[4];

		for (unsigned i = 0; i < NUMPOINTS; ++i)
		{
			r0[0] = static_cast<float>(rand() % n - n2);
			r0[1] = static_cast<float>(rand() % n - n2);
			r0[2] = static_cast<float>(rand() % n - n2);
			r0[3] = 1.0f;
			buffer[i].set_position(r0);
			r0[0] = (r0[0] / n) + 0.5f;
			r0[1] = (r0[1] / n) + 0.5f;
			r0[2] = (r0[2] / n) + 0.5f;
			r0[3] = 1.0f;
			buffer[i].set_color(r0);
		}

		vb_points =
			renderer.createVertexBuffer(bufSize, gfx::BufferUsage::Static, Memory(buffer));
	}

	{
		const size_t bufSize = sizeof(gfx::DrawVert) * 6;
		std::vector<gfx::DrawVert> buffer(6);
		float r[4]{0.f,0.f,0.f,0.f};

		for (int i = 0; i < 6; ++i)
		{
			r[0] = UNIT_RECT_WITH_ST[i * 4 + 0];
			r[1] = UNIT_RECT_WITH_ST[i * 4 + 1];
			buffer[i].set_position(r);
			r[0] = UNIT_RECT_WITH_ST[i * 4 + 2];
			r[1] = UNIT_RECT_WITH_ST[i * 4 + 3];
			buffer[i].set_texcoord(r);
		}
		vb_pp =
			renderer.createVertexBuffer(bufSize, gfx::BufferUsage::Static, Memory(buffer));
	}

	{
		const int vert_count = (sizeof(UNIT_BOX_POSITIONS) / sizeof(float) / 3);
		const size_t bufSize = sizeof(gfx::DrawVert) * vert_count;

		std::vector<gfx::DrawVert> buffer(vert_count);

		float r[4]{ 0.f,0.f,0.f,0.f };

		for (int i = 0; i < vert_count; ++i)
		{
			r[0] = UNIT_BOX_POSITIONS[i * 3 + 0];
			r[1] = UNIT_BOX_POSITIONS[i * 3 + 1];
			r[2] = UNIT_BOX_POSITIONS[i * 3 + 3];
			buffer[i].set_position(r);
		}

		vb_skybox =
			renderer.createVertexBuffer(bufSize, gfx::BufferUsage::Static, Memory(buffer));
	}



    return false;
}
