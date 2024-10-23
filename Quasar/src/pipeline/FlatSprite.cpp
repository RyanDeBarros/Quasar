#include "FlatSprite.h"

#include "variety/GLutility.h"

FlatSprite::FlatSprite()
{
	varr = new GLfloat[NUM_VERTICES * STRIDE]{
		-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};
}

FlatSprite::FlatSprite(const FlatSprite& other)
	: image(other.image), transform(other.transform)
{
	varr = new GLfloat[NUM_VERTICES * STRIDE];
	memcpy(varr, other.varr, VLEN_BYTES);
}

FlatSprite::FlatSprite(FlatSprite&& other) noexcept
	: image(other.image), transform(other.transform), varr(other.varr)
{
	other.varr = nullptr;
}

FlatSprite& FlatSprite::operator=(const FlatSprite& other)
{
	if (this != &other)
	{
		memcpy(varr, other.varr, VLEN_BYTES);
		image = other.image;
		transform = other.transform;
	}
	return *this;
}

FlatSprite& FlatSprite::operator=(FlatSprite&& other) noexcept
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

FlatSprite::~FlatSprite()
{
	delete[] varr;
}

void FlatSprite::sync_transform() const
{
	auto p = transform.packed();
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_PACKED, &p[0], sizeof(p));
		row += STRIDE;
	}
}

void FlatSprite::sync_image_dimensions(Dim v_width, Dim v_height) const
{
	Image* img = Images.get(image);
	Dim w = v_width >= 0 ? v_width : (img ? img->buf.width : 0);
	Dim h = v_height >= 0 ? v_height : (img ? img->buf.height : 0);

	varr[size_t(0) * STRIDE + SHADER_POS_VERT_POS] = -0.5f * w;
	varr[size_t(0) * STRIDE + SHADER_POS_VERT_POS + 1] = -0.5f * h;
	varr[size_t(1) * STRIDE + SHADER_POS_VERT_POS] = 0.5f * w;
	varr[size_t(1) * STRIDE + SHADER_POS_VERT_POS + 1] = -0.5f * h;
	varr[size_t(2) * STRIDE + SHADER_POS_VERT_POS] = 0.5f * w;
	varr[size_t(2) * STRIDE + SHADER_POS_VERT_POS + 1] = 0.5f * h;
	varr[size_t(3) * STRIDE + SHADER_POS_VERT_POS] = -0.5f * w;
	varr[size_t(3) * STRIDE + SHADER_POS_VERT_POS + 1] = 0.5f * h;
}

void FlatSprite::sync_texture_slot(float texture_slot) const
{
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		row[SHADER_POS_TEXTURE] = texture_slot;
		row += STRIDE;
	}
}

glm::vec4 FlatSprite::modulation() const
{
	return glm::vec4{ varr[SHADER_POS_MODULATE], varr[SHADER_POS_MODULATE + 1], varr[SHADER_POS_MODULATE + 2], varr[SHADER_POS_MODULATE + 3] };
}

void FlatSprite::set_modulation(const glm::vec4& color) const
{
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_MODULATE, &color[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

ColorFrame FlatSprite::modulation_color_frame() const
{
	return ColorFrame(RGB(varr[SHADER_POS_MODULATE], varr[SHADER_POS_MODULATE + 1], varr[SHADER_POS_MODULATE + 2]), varr[SHADER_POS_MODULATE + 3]);
}

void FlatSprite::set_modulation(ColorFrame color) const
{
	GLfloat* row = varr;
	glm::vec4 cvec = color.rgba_as_vec();
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_MODULATE, &cvec[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

void FlatSprite::set_uvs(const Bounds& bounds) const
{
	bounds.pass_uvs(varr + SHADER_POS_UV, STRIDE);
}

void SharedFlatSprite::initialize_varr() const
{
	static GLfloat initial[SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE]{
		-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};
	memcpy(varr, initial, sizeof(initial));
}

void SharedFlatSprite::sync_transform() const
{
	auto p = transform.packed();
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_PACKED, &p[0], sizeof(p));
		row += STRIDE;
	}
}

void SharedFlatSprite::sync_image_dimensions(Dim v_width, Dim v_height) const
{
	Image* img = Images.get(image);
	Dim w = v_width >= 0 ? v_width : (img ? img->buf.width : 0);
	Dim h = v_height >= 0 ? v_height : (img ? img->buf.height : 0);

	varr[size_t(0) * STRIDE + SHADER_POS_VERT_POS] = -0.5f * w;
	varr[size_t(0) * STRIDE + SHADER_POS_VERT_POS + 1] = -0.5f * h;
	varr[size_t(1) * STRIDE + SHADER_POS_VERT_POS] = 0.5f * w;
	varr[size_t(1) * STRIDE + SHADER_POS_VERT_POS + 1] = -0.5f * h;
	varr[size_t(2) * STRIDE + SHADER_POS_VERT_POS] = 0.5f * w;
	varr[size_t(2) * STRIDE + SHADER_POS_VERT_POS + 1] = 0.5f * h;
	varr[size_t(3) * STRIDE + SHADER_POS_VERT_POS] = -0.5f * w;
	varr[size_t(3) * STRIDE + SHADER_POS_VERT_POS + 1] = 0.5f * h;
}

void SharedFlatSprite::sync_texture_slot(float texture_slot) const
{
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		row[SHADER_POS_TEXTURE] = texture_slot;
		row += STRIDE;
	}
}

glm::vec4 SharedFlatSprite::modulation() const
{
	return glm::vec4{ varr[SHADER_POS_MODULATE], varr[SHADER_POS_MODULATE + 1], varr[SHADER_POS_MODULATE + 2], varr[SHADER_POS_MODULATE + 3] };
}

void SharedFlatSprite::set_modulation(const glm::vec4& color) const
{
	GLfloat* row = varr;
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_MODULATE, &color[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

ColorFrame SharedFlatSprite::modulation_color_frame() const
{
	return ColorFrame(RGB(varr[SHADER_POS_MODULATE], varr[SHADER_POS_MODULATE + 1], varr[SHADER_POS_MODULATE + 2]), varr[SHADER_POS_MODULATE + 3]);
}

void SharedFlatSprite::set_modulation(ColorFrame color) const
{
	GLfloat* row = varr;
	glm::vec4 cvec = color.rgba_as_vec();
	for (size_t i = 0; i < NUM_VERTICES; ++i)
	{
		memcpy(row + SHADER_POS_MODULATE, &cvec[0], 4 * sizeof(GLfloat));
		row += STRIDE;
	}
}

void SharedFlatSprite::set_uvs(const Bounds& bounds) const
{
	bounds.pass_uvs(varr + SHADER_POS_UV, STRIDE);
}
