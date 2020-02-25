#pragma once

#include "common/common.h"

#include <d3d11.h>

#include <d3dcompiler.h>
#include <string>

#include "resource_file.h"

#ifdef ROOTEX_EDITOR
class Editor;
#endif // ROOTEX_EDITOR


class RenderingDevice
{
	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_RenderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetTextureView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetBackBufferView;
	ID3D11RenderTargetView** m_CurrentRenderTarget;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_RenderTextureShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_OldSkyDepthStencilState;
	UINT m_StencilRef;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_NewSkyDepthStencilState;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	unsigned int m_4XMSQuality;

	RenderingDevice();
	RenderingDevice(RenderingDevice&) = delete;
	~RenderingDevice();

	// Should only be called by Window class
	void swapBuffers();

	friend class Window;

#ifdef ROOTEX_EDITOR
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	friend class Editor;
#endif // ROOTEX_EDITOR

public:
	static RenderingDevice* GetSingleton();

	void initialize(HWND hWnd, int width, int height);

	void enableSkyDepthStencilState();
	void disableSkyDepthStencilState();
	ID3D11Buffer* initVertexBuffer(D3D11_BUFFER_DESC* vbd, D3D11_SUBRESOURCE_DATA* vsd, const UINT* stride, const UINT* offset);
	ID3D11Buffer* initIndexBuffer(D3D11_BUFFER_DESC* ibd, D3D11_SUBRESOURCE_DATA* isd, DXGI_FORMAT format);
	void initVSModelConstantBuffer(D3D11_BUFFER_DESC* cbd, D3D11_SUBRESOURCE_DATA* csd);
	void initVSModelInverseConstantBuffer(D3D11_BUFFER_DESC* cbd, D3D11_SUBRESOURCE_DATA* csd);
	void initVSViewConstantBuffer(D3D11_BUFFER_DESC* cbd, D3D11_SUBRESOURCE_DATA* csd);
	void initVSProjectionConstantBuffer(D3D11_BUFFER_DESC* cbd, D3D11_SUBRESOURCE_DATA* csd);
	void initPSConstantBuffer(D3D11_BUFFER_DESC* cbd, D3D11_SUBRESOURCE_DATA* csd, UINT offset);
	ID3D11PixelShader* initPixelShader(ID3DBlob* blob);
	ID3D11VertexShader* initVertexShader(ID3DBlob* blob);
	void initVertexLayout(ID3DBlob* vertexShaderBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT size);
	
	ID3DBlob* createBlob(LPCWSTR path);
	void createRenderTextureTarget(int width, int height);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> createTexture(ImageResourceFile* imageRes);
	Microsoft::WRL::ComPtr<ID3D11SamplerState> createSamplerState();

	void bind(ID3D11Buffer* vertexBuffer, const unsigned int* stride, const unsigned int* offset);
	void bind(ID3D11Buffer* indexBuffer, DXGI_FORMAT format);
	void bind(ID3D11VertexShader* vertexShader);
	void bind(ID3D11PixelShader* pixelShader);
	
	void setInPixelShader(unsigned int slot, unsigned int number, ID3D11ShaderResourceView* texture);
	void setInPixelShader(ID3D11SamplerState* samplerState);

	void unbindShaderResources();

	void setTextureRenderTarget();
	void setBackBufferRenderTarget();

	ID3D11ShaderResourceView* getRenderTextureShaderResourceView();

	void setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY pt);
	void setViewport(const D3D11_VIEWPORT* vp);
	
	void drawIndexed(UINT number);
	void clearBuffer(float r, float g, float b);
};
