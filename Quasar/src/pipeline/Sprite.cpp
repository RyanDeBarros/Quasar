#include "Sprite.h"

#include "variety/GLutility.h"

Sprite::Sprite()
{
	varr = new GLfloat[NUM_VERTICES * STRIDE]{
		-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
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

//void Sprite::on_draw(Renderer* renderer) const
//{
//	renderer->prepare_for_sprite();
//	Image* img = Images.get(image);
//	if (img)
//	{
//		auto texture_slot = renderer->get_texture_slot(img->tid);
//		bind_texture(img->tid, texture_slot);
//		sync_texture_slot(texture_slot);
//	}
//	else
//		sync_texture_slot(-1.0f);
//	renderer->pool_over_varr(varr);
//}

void Sprite::sync_transform() const
{
	glm::vec2 pp = transform.packed_p();
	glm::vec4 prs = transform.packed_rs();
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
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
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_PACKED_P, &pp[0], 2 * sizeof(GLfloat));
		row += STRIDE;
	}
}

void Sprite::sync_transform_rs() const
{
	glm::vec4 prs = transform.packed_rs();
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_PACKED_RS, &prs[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

void Sprite::sync_image_dimensions(Image::Dim v_width, Image::Dim v_height) const
{
	Image* img = Images.get(image);
	Image::Dim w = v_width >= 0 ? v_width : (img ? img->width : 0);
	Image::Dim h = v_height >= 0 ? v_height : (img ? img->height : 0);
	
	varr[size_t(0) * STRIDE + SHADER_POS_VERT_POS    ] = -0.5f * w;
	varr[size_t(0) * STRIDE + SHADER_POS_VERT_POS + 1] = -0.5f * h;
	varr[size_t(1) * STRIDE + SHADER_POS_VERT_POS    ] =  0.5f * w;
	varr[size_t(1) * STRIDE + SHADER_POS_VERT_POS + 1] = -0.5f * h;
	varr[size_t(2) * STRIDE + SHADER_POS_VERT_POS    ] =  0.5f * w;
	varr[size_t(2) * STRIDE + SHADER_POS_VERT_POS + 1] =  0.5f * h;
	varr[size_t(3) * STRIDE + SHADER_POS_VERT_POS    ] = -0.5f * w;
	varr[size_t(3) * STRIDE + SHADER_POS_VERT_POS + 1] =  0.5f * h;
}

void Sprite::sync_texture_slot(float texture_slot) const
{
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		row[SHADER_POS_TEXTURE] = texture_slot;
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
	for (size_t i = 0; i < NUM_VERTICES; ++i)
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
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_MODULATE, &cvec[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

void Sprite::set_uvs(const Bounds& bounds) const
{
	bounds.pass_uvs(varr + SHADER_POS_UV, STRIDE);
}
