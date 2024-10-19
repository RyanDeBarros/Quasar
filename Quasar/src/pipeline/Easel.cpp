#include "Easel.h"

Checkerboard::Checkerboard(RGBA c1, RGBA c2)
	: c1(c1), c2(c2)
{
	Image img;
	img.width = 2;
	img.height = 2;
	img.chpp = 4;
	img.pixels = new Image::Byte[img.area()];
	img.gen_texture();
	set_image(Images.add(std::move(img)));
	sync_colors();
	set_uv_size(0, 0);
}

void Checkerboard::sync_colors() const
{
	Image* img = Images.get(image);
	for (size_t i = 0; i < 2; ++i)
	{
		img->pixels[0 + 12 * i] = c1.rgb.r;
		img->pixels[1 + 12 * i] = c1.rgb.g;
		img->pixels[2 + 12 * i] = c1.rgb.b;
		img->pixels[3 + 12 * i] = c1.alpha;
	}
	for (size_t i = 0; i < 2; ++i)
	{
		img->pixels[4 + 4 * i] = c2.rgb.r;
		img->pixels[5 + 4 * i] = c2.rgb.g;
		img->pixels[6 + 4 * i] = c2.rgb.b;
		img->pixels[7 + 4 * i] = c2.alpha;
	}
	sync_texture();
}

void Checkerboard::sync_texture() const
{
	Image* img = Images.get(image);
	img->resend_texture();
	TextureParams tparams;
	tparams.wrap_s = TextureWrap::Repeat;
	tparams.wrap_t = TextureWrap::Repeat;
	img->update_texture_params(tparams);
}

void Checkerboard::set_uv_size(float width, float height) const
{
	set_uvs(Bounds{ 0.0f, width, 0.0f, height });
	sync_texture();
}

Canvas::Canvas(RGBA c1, RGBA c2)
	: checkerboard(c1, c2)
{
	set_image(ImageHandle(0));
}

void Canvas::set_image(ImageHandle img)
{
	sprite.set_image(img);
	image = Images.get(img);
	if (image)
	{
		checkerboard.set_uv_size(0.5f * image->width / checker_size, 0.5f * image->height / checker_size);
		checkerboard.set_modulation(ColorFrame());
	}
	else
	{
		checkerboard.set_modulation(ColorFrame(0));
	}
}

void Canvas::sync_transform()
{
	sprite.sync_transform();
	checkerboard.transform = sprite.transform;
	if (image)
		checkerboard.transform.scale *= glm::vec2{ image->width * 0.5f, image->height * 0.5f };
	checkerboard.sync_transform();
}

void Canvas::sync_transform_p()
{
	sprite.sync_transform_p();
	checkerboard.transform.position = sprite.transform.position;
	checkerboard.sync_transform_p();
}

void Canvas::sync_transform_rs()
{
	sprite.sync_transform_rs();
	checkerboard.transform.rotation = sprite.transform.rotation;
	if (image)
		checkerboard.transform.scale = sprite.transform.scale * glm::vec2{ image->width * 0.5f, image->height * 0.5f };
	checkerboard.sync_transform_rs();
}
