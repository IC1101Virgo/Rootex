#pragma once

#include "renderer/material.h"

class Texture;

class BasicMaterial : public Material
{
	BasicShader* m_BasicShader;
	Ref<Texture> m_DiffuseTexture;
	Ref<Texture> m_NormalTexture;
	Ref<Texture> m_SpecularTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;

	ImageResourceFile* m_DiffuseImageFile;
	ImageResourceFile* m_NormalImageFile;
	ImageResourceFile* m_SpecularImageFile;

	bool m_IsLit;
	bool m_IsNormal;
	Color m_Color;
	float m_SpecularIntensity;
	float m_SpecularPower;
	float m_Reflectivity;
	float m_RefractionConstant;
	float m_Refractivity;
	bool m_IsAffectedBySky;

	void setPSConstantBuffer(const PSDiffuseConstantBufferMaterial& constantBuffer);
	void setVSConstantBuffer(const VSDiffuseConstantBuffer& constantBuffer);

#ifdef ROOTEX_EDITOR
	String m_ImagePathUI;
#endif // ROOTEX_EDITOR
public:
	const static inline String s_MaterialName = "BasicMaterial";
	enum class VertexConstantBufferType
	{
		Model,
		End
	};
	enum class PixelConstantBufferType
	{
		Material,
		End
	};

	BasicMaterial() = delete;
	BasicMaterial(bool isAlpha, const String& imagePath, const String& normalImagePath, const String& specularImagePath, bool isNormal, Color color, bool isLit, float specularIntensity, float specularPower, float reflectivity, float refractionConstant, float refractivity, bool affectedBySky);
	~BasicMaterial() = default;

	void setColor(const Color& color) { m_Color = color; };
	void setTexture(ImageResourceFile* image);
	void setNormal(ImageResourceFile* image);
	void setSpecularTexture(ImageResourceFile* image);
	void removeNormal();
	void setTextureInternal(Ref<Texture> texture);
	void setNormalInternal(Ref<Texture> texture);
	void setSpecularInternal(Ref<Texture> texture);
	void setSpecularIntensity(float specIntensity) { m_SpecularIntensity = specIntensity; }
	void setSpecularPower(float specPower) { m_SpecularPower = specPower; }

	static Material* CreateDefault();
	static Material* Create(const JSON::json& materialData);

	virtual ID3D11ShaderResourceView* getPreview() override;

	void bind() override;
	JSON::json getJSON() const override;

#ifdef ROOTEX_EDITOR
	void draw(const String& id) override;
#endif // ROOTEX_EDITOR
};
