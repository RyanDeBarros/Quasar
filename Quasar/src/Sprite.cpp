#include "Sprite.h"

#include "Renderer.h"
#include "GLutility.h"

Sprite::Sprite(ImageHandle image)
	: image(image)
{
	Image* img = ImageRegistry.get(image);
	varr = new GLfloat[NUM_VERTICES * STRIDE]{
		0.0f, -0.5f * img->width, -0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f,  0.5f * img->width, -0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		2.0f,  0.5f * img->width,  0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		3.0f, -0.5f * img->width,  0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};
}

Sprite::Sprite(const Sprite& other)
	: image(other.image), transform(other.transform)
{
	varr = new GLfloat[NUM_VERTICES * STRIDE];
	memcpy(varr, other.varr, VLEN_BYTES);
}

Sprite::Sprite(Sprite&& other) noexcept
	: image(other.image), transform(other.transform), varr(other.varr)
{
	other.varr = nullptr;
}

Sprite& Sprite::operator=(const Sprite& other)
{
	if (this != &other)
	{
		memcpy(varr, other.varr, VLEN_BYTES);
		image = other.image;
		transform = other.transform;
	}
	return *this;
}

Sprite& Sprite::operator=(Sprite&& other) noexcept
{
	if (this != &other)
	{
		delete[] varr;
		varr = other.varr;
		other.varr = nullptr;
		image = other.image;
		transform = other.transform;
	}
	return *this;
}

Sprite::~Sprite()
{
	delete[] varr;
}

void Sprite::on_draw(Renderer* renderer) const
{
	renderer->prepare_for_sprite();
	Image* img = ImageRegistry.get(image);
	bind_texture(img->tid, renderer->get_texture_slot(img->tid));
	renderer->pool_over_varr(varr);
}

void Sprite::sync_transform() const
{
	glm::vec2 pp = transform.packed_p();
	glm::vec4 prs = transform.packed_rs();
	GLfloat* row = varr;
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + SHADER_POS_PACKED_P, &pp[0], 2 * sizeof(GLfloat));
		memcpy(row + SHADER_POS_PACKED_RS, &prs[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

void Sprite::sync_transform_p() const
{
	glm::vec2 pp = transform.packed_p();
	GLfloat* row = varr;
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + SHADER_POS_PACKED_P, &pp[0], 2 * sizeof(GLfloat));
		row += STRIDE;
	}
}

void Sprite::sync_transform_rs() const
{
	glm::vec4 prs = transform.packed_rs();
	GLfloat* row = varr;
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + SHADER_POS_PACKED_RS, &prs[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

glm::vec4 Sprite::modulation() const
{
	return glm::vec4{ varr[SHADER_POS_MODULATE], varr[SHADER_POS_MODULATE + 1], varr[SHADER_POS_MODULATE + 2], varr[SHADER_POS_MODULATE + 3] };
}

void Sprite::set_modulation(const glm::vec4& color) const
{
	GLfloat* row = varr;
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + SHADER_POS_MODULATE, &color[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

ColorFrame Sprite::modulation_color_frame() const
{
	return ColorFrame(RGB(varr[SHADER_POS_MODULATE], varr[SHADER_POS_MODULATE + 1], varr[SHADER_POS_MODULATE + 2]), varr[SHADER_POS_MODULATE + 3]);
}

void Sprite::set_modulation(ColorFrame color) const
{
	GLfloat* row = varr;
	glm::vec4 cvec = color.rgba_as_vec();
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + SHADER_POS_MODULATE, &cvec[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}
