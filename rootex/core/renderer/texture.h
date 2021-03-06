#pragma once

#include <d3d11.h>

class ImageResourceFile;

/// Encapsulates all Texture related functionalities, uses DirectXTK behind the scenes
class Texture
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
	ImageResourceFile* m_ImageFile;
	unsigned int m_Width;
	unsigned int m_Height;
	unsigned int m_MipLevels;

	void loadTexture();

public:
	Texture(ImageResourceFile* imageFile);
	Texture(const char* imageData, int width, int height);
	Texture(const char* imageFileData, size_t size);
	Texture(Texture&) = delete;
	Texture& operator=(Texture&) = delete;
	~Texture() = default;

	void reload();

	static Texture* GetCrossTexture();

	ID3D11ShaderResourceView* getTextureResourceView() const { return m_TextureView.Get(); }
	ID3D11Texture2D* getD3D11Texture2D() const { return m_Texture.Get(); }
	unsigned int getWidth() const { return m_Width; }
	unsigned int getHeight() const { return m_Height; }
	unsigned int getMipLevels() const { return m_MipLevels; }
	ImageResourceFile* getImage() const { return m_ImageFile; }
};

/// Cube texture in 3D
class Texture3D
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureView;
	ImageResourceFile* m_ImageFile;
	
	void loadTexture();

public:
	Texture3D(ImageResourceFile* imageFile);
	Texture3D(Texture3D&) = delete;
	Texture3D& operator=(Texture3D&) = delete;
	~Texture3D() = default;

	void reload();

	ID3D11ShaderResourceView* getTextureResourceView() const { return m_TextureView.Get(); }
	ImageResourceFile* getImage() const { return m_ImageFile; }
};
