#pragma once



class GCShader
{
public:
	GCShader();
	~GCShader();

	void CompileShader();

	ID3DBlob* GetmvsByteCode();
	ID3DBlob* GetmpsByteCode();

	void RootSign();
	void Pso();

	ID3D12RootSignature* GetRootSign();
	ID3D12PipelineState* GetPso(bool alpha);

	GC_GRAPHICS_ERROR Initialize(GCRender* pRender, const std::string& filePath, const std::string& csoDestinationPath, int& flagEnabledBits, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK);

	int GetFlagEnabledBits() const { return m_flagEnabledBits; }

	ID3DBlob* CompileShaderBase(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target);

	// Precompile by CSO
	void SaveShaderToFile(ID3DBlob* shaderBlob, const std::wstring& filename);
	ID3DBlob* LoadShaderFromFile(const std::wstring& filename);
	void PreCompile(const std::string& filePath, const std::string& csoDestinationPath);

	GC_GRAPHICS_ERROR Load();

	void SetRenderTarget(ID3D12Resource* rtt);

	GCRender* m_pRender;
	ID3D12Resource* m_pRtt = nullptr;
protected:
	// Initialize var
	D3D12_CULL_MODE m_cullMode;

	ID3D12RootSignature* m_RootSignature;
	ID3D12PipelineState* m_PSO1;
	ID3D12PipelineState* m_PSO2;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	ID3DBlob* m_vsByteCode;
	ID3DBlob* m_psByteCode;

	// Var used in Compile Shader override func
	std::wstring m_vsCsoPath;
	std::wstring m_psCsoPath;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc2;
	//int m_type;
	int m_flagEnabledBits;
};

class GCComputeShader
{
public:
	GCComputeShader();
	~GCComputeShader();

	GC_GRAPHICS_ERROR Initialize(GCRender* pRender, const std::string& filePath, const std::string& csoDestinationPath, int& flagEnabledBits, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK);
	void CompileShader();
	void RootSign();
	void Pso();

	ID3D12RootSignature* GetRootSign();
	ID3D12PipelineState* GetPso();
	ID3DBlob* GetmcsByteCode();
	ID3DBlob* CompileShaderBase(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target);
	void SaveShaderToFile(ID3DBlob* shaderBlob, const std::wstring& filename);
	ID3DBlob* LoadShaderFromFile(const std::wstring& filename);
	void PreCompile(const std::string& filePath, const std::string& csoDestinationPath);
	GC_GRAPHICS_ERROR Load();

private:
	ID3D12RootSignature* m_RootSignature;
	ID3D12PipelineState* m_PSO;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
	ID3DBlob* m_csByteCode;
	std::wstring m_csCsoPath;
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
	GCRender* m_pRender;
	int m_flagEnabledBits;
	D3D12_CULL_MODE m_cullMode;
};