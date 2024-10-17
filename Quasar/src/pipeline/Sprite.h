#pragma once

#include "variety/Geometry.h"
#include "edit/Color.h"
#include "user/Machine.h"

struct Sprite
{
	static constexpr unsigned char NUM_VERTICES = 4;
	static constexpr unsigned char NUM_INDICES = 6;
	static constexpr unsigned char STRIDE = 14;
	static constexpr unsigned char ILEN_BYTES = NUM_INDICES * sizeof(GLuint);
	static constexpr unsigned char VLEN_BYTES = size_t(NUM_VERTICES) * STRIDE * sizeof(GLfloat);
	static constexpr GLuint IARR[6]{ 0, 1, 2, 2, 3, 0 };
	static constexpr size_t SHADER_POS_VERT = 0;
	static constexpr size_t SHADER_POS_COORD = 1;
	static constexpr size_t SHADER_POS_TEXTURE = 3;
	static constexpr size_t SHADER_POS_PACKED_P = 4;
	static constexpr size_t SHADER_POS_PACKED_RS = 6;
	static constexpr size_t SHADER_POS_MODULATE = 10;
	
	GLfloat* varr = nullptr;
	Transform transform{};
	MachineImpl::ImageHandle image = MachineImpl::ImageHandle(0);
	
	Sprite();
	Sprite(const Sprite&);
	Sprite(Sprite&&) noexcept;
	Sprite& operator=(const Sprite&);
	Sprite& operator=(Sprite&&) noexcept;
	~Sprite();

	void on_draw(class Renderer*) const;
	void sync_transform() const;
	void sync_transform_p() const;
	void sync_transform_rs() const;

	void set_image(MachineImpl::ImageHandle img, Image::Dim v_width = -1, Image::Dim v_height = -1) { image = img; sync_image_dimensions(v_width, v_height); }
	void sync_image_dimensions(Image::Dim v_width = -1, Image::Dim v_height = -1) const;
	void sync_texture_slot(float texture_slot) const;

	glm::vec4 modulation() const;
	void set_modulation(const glm::vec4& color) const;
	ColorFrame modulation_color_frame() const;
	void set_modulation(ColorFrame color) const;
};
