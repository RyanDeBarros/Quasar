#pragma once

#include "variety/Geometry.h"
#include "edit/Color.h"
#include "edit/Image.h"

// LATER use renderables here

struct FlatSprite
{
	static constexpr unsigned char NUM_VERTICES = 4;
	static constexpr unsigned char NUM_INDICES = 6;
	static constexpr unsigned char STRIDE = 13;
	static constexpr unsigned char ILEN_BYTES = NUM_INDICES * sizeof(GLuint);
	static constexpr unsigned char VLEN_BYTES = size_t(NUM_VERTICES) * STRIDE * sizeof(GLfloat);
	static constexpr GLuint IARR[6]{ 0, 1, 2, 2, 3, 0 };
	static constexpr size_t SHADER_POS_TEXTURE = 0;
	static constexpr size_t SHADER_POS_VERT_POS = 1;
	static constexpr size_t SHADER_POS_UV = 3;
	static constexpr size_t SHADER_POS_PACKED = 5;
	static constexpr size_t SHADER_POS_MODULATE = 9;

	GLfloat* varr = nullptr;
	FlatTransform transform{};
	std::shared_ptr<Image> image;

	FlatSprite();
	FlatSprite(const FlatSprite&);
	FlatSprite(FlatSprite&&) noexcept;
	FlatSprite& operator=(const FlatSprite&);
	FlatSprite& operator=(FlatSprite&&) noexcept;
	~FlatSprite();

	void sync_transform() const;

	void set_image(std::shared_ptr<Image>&& img, Dim v_width = -1, Dim v_height = -1) { image = std::move(img); sync_image_dimensions(v_width, v_height); }
	void set_image(const std::shared_ptr<Image>& img, Dim v_width = -1, Dim v_height = -1) { image = img; sync_image_dimensions(v_width, v_height); }
	void sync_image_dimensions(Dim v_width = -1, Dim v_height = -1) const;
	void sync_texture_slot(float texture_slot) const;

	glm::vec4 modulation() const;
	void set_modulation(const glm::vec4& color) const;
	ColorFrame modulation_color_frame() const;
	void set_modulation(ColorFrame color) const;

	void set_uvs(const Bounds& bounds) const;
};

struct SharedFlatSprite
{
	GLfloat* varr = nullptr;
	FlatTransform transform{};
	std::shared_ptr<Image> image;

	SharedFlatSprite() = default;
	SharedFlatSprite(const FlatSprite&) = delete;
	SharedFlatSprite(FlatSprite&&) noexcept = delete;
	~SharedFlatSprite() = default;
	
	void initialize_varr() const;
	void sync_transform() const;

	void set_image(std::shared_ptr<Image>&& img, Dim v_width = -1, Dim v_height = -1) { image = std::move(img); sync_image_dimensions(v_width, v_height); }
	void set_image(const std::shared_ptr<Image>& img, Dim v_width = -1, Dim v_height = -1) { image = img; sync_image_dimensions(v_width, v_height); }
	void sync_image_dimensions(Dim v_width = -1, Dim v_height = -1) const;
	void sync_texture_slot(float texture_slot) const;

	glm::vec4 modulation() const;
	void set_modulation(const glm::vec4& color) const;
	ColorFrame modulation_color_frame() const;
	void set_modulation(ColorFrame color) const;

	void set_uvs(const Bounds& bounds) const;
};
