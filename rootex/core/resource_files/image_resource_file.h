#pragma once

#include "resource_file.h"
#include "renderer/texture.h"

/// Representation of an image file. Supports BMP, JPEG, PNG, TIFF, GIF, HD Photo, or other WIC supported file containers
class ImageResourceFile : public ResourceFile
{
	Ref<Texture> m_ImageTexture;

	explicit ImageResourceFile(const FilePath& path);

	friend class ResourceLoader;

public:
	static void RegisterAPI(sol::table& rootex);
	
	explicit ImageResourceFile(const ImageResourceFile&) = delete;
	explicit ImageResourceFile(const ImageResourceFile&&) = delete;
	~ImageResourceFile() = default;

	void reimport() override;

	const Ref<Texture> getTexture() { return m_ImageTexture; }
};