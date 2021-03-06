#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"
#include "material.h"
#include "rendering_device.h"
#include "viewport.h"

/// Makes the rendering draw call and set viewport, instrumental in seperating Game and HUD rendering
class Renderer
{
public:
	Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	virtual ~Renderer() = default;

	void setViewport(Viewport& viewport);
	
	void bind(Material* material) const;
	void draw(const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer) const;
};
