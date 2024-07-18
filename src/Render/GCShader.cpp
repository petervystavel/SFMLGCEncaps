#include "pch.h"
//test
GCShader::GCShader() 
{
	m_RootSignature = nullptr;
	m_PSO1 = nullptr;
	m_PSO2 = nullptr;
	m_InputLayout.clear(); 
	m_vsByteCode = nullptr;
	m_psByteCode = nullptr;
	m_vsCsoPath.clear();
	m_psCsoPath.clear();
	ZeroMemory(&psoDesc, sizeof(psoDesc));
	m_pRender = nullptr;
	m_flagEnabledBits = 0;
	m_cullMode = D3D12_CULL_MODE_NONE;
}


GCShader::~GCShader()
{
	SAFE_RELEASE(m_RootSignature);
	SAFE_RELEASE(m_PSO1);
	SAFE_RELEASE(m_PSO2);
	SAFE_RELEASE(m_vsByteCode);
	SAFE_RELEASE(m_psByteCode);

	m_InputLayout.clear();
}

GC_GRAPHICS_ERROR GCShader::Initialize(GCRender* pRender, const std::string& filePath, const std::string& csoDestinationPath, int& flagEnabledBits, D3D12_CULL_MODE cullMode)
{
	if (!CHECK_POINTERSNULL("Render ptr is not null", "Render pointer is null", pRender))
		return GCRENDER_ERROR_POINTER_NULL;
	if (!CHECK_FILE(filePath, "Shader not found: " + filePath, "Shader file: " + filePath + " loaded successfully"))
		return GCRENDER_ERROR_SHADER_CREATION_FAILED;

	std::wstring baseCsoPath(csoDestinationPath.begin(), csoDestinationPath.end());
	m_vsCsoPath = baseCsoPath + L"VS.cso";
	m_psCsoPath = baseCsoPath + L"PS.cso";

	m_cullMode = cullMode;
	m_pRender = pRender;
	m_flagEnabledBits = flagEnabledBits;

	PreCompile(filePath, csoDestinationPath);

	return GCRENDER_SUCCESS_OK;
}
 
void GCShader::CompileShader()
{
	m_vsByteCode = LoadShaderFromFile(m_vsCsoPath);
	m_psByteCode = LoadShaderFromFile(m_psCsoPath);

	m_InputLayout.clear();

	UINT offset = 0; 
	if (HAS_FLAG(m_flagEnabledBits, VERTEX_POSITION)) {
		m_InputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += sizeof(DirectX::XMFLOAT3); 
	}

	if (HAS_FLAG(m_flagEnabledBits, VERTEX_COLOR)) {
		m_InputLayout.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += sizeof(DirectX::XMFLOAT4); 
	}

	if (HAS_FLAG(m_flagEnabledBits, VERTEX_UV)) {
		m_InputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += sizeof(DirectX::XMFLOAT2); // Taille des coordonnées de texture
	}

	if (HAS_FLAG(m_flagEnabledBits, VERTEX_NORMAL)) {
		m_InputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		offset += sizeof(DirectX::XMFLOAT3); // Taille des coordonnées de texture
	}
}


void GCShader::RootSign()
{
	// Déclaration des paramètres racine
	CD3DX12_ROOT_PARAMETER slotRootParameter[6];

	slotRootParameter[CBV_SLOT_CB0].InitAsConstantBufferView(0);
	slotRootParameter[CBV_SLOT_CB1].InitAsConstantBufferView(1);
	slotRootParameter[CBV_SLOT_CB2].InitAsConstantBufferView(2);
	slotRootParameter[CBV_SLOT_CB3].InitAsConstantBufferView(3);
	UINT numParameters = 4;


	// Configuration de l'échantillonneur statique
	CD3DX12_STATIC_SAMPLER_DESC staticSample = CD3DX12_STATIC_SAMPLER_DESC(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressW
		0.0f, // mipLODBias
		16, // maxAnisotropy
		D3D12_COMPARISON_FUNC_ALWAYS, // comparisonFunc
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, // borderColor
		0.0f, // minLOD
		D3D12_FLOAT32_MAX // maxLOD
	);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;


	if (HAS_FLAG(m_flagEnabledBits, VERTEX_UV)) {
		CD3DX12_DESCRIPTOR_RANGE srvTable;
		srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		slotRootParameter[DESCRIPTOR_TABLE_SLOT_TEXTURE].InitAsDescriptorTable(1, &srvTable);
		numParameters++;

		CD3DX12_DESCRIPTOR_RANGE srvTable2;
		srvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		slotRootParameter[DESCRIPTOR_TABLE_SLOT_TEXTURE2].InitAsDescriptorTable(1, &srvTable2);
		numParameters++;
	}



	rootSigDesc.Init(numParameters, slotRootParameter, 1, &staticSample, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);



	// Sérialisation de la signature racine
	ID3DBlob* serializedRootSig = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);

	// Gestion des erreurs de sérialisation
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

    // Gestion des erreurs de sérialisation
    if (errorBlob != nullptr) {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }

    // Création de la signature racine
    m_pRender->GetRenderResources()->Getmd3dDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
}
void GCShader::Pso() 
{
	// Initialize the graphics pipeline state description
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	psoDesc.pRootSignature = m_RootSignature;

	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	ZeroMemory(&psoDesc2, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc2.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	psoDesc2.pRootSignature = m_RootSignature;

	psoDesc2.VS =
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc2.PS =
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};

	psoDesc2.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc2.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc2.RasterizerState.CullMode = m_cullMode;
	psoDesc.RasterizerState.CullMode = m_cullMode;

	// Customize the blend state for transparency
	CD3DX12_BLEND_DESC blendDesc1(D3D12_DEFAULT);
	blendDesc1.RenderTarget[0].BlendEnable = TRUE;
	blendDesc1.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc1.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc1.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc1.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc1.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc1.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc1.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc2.BlendState = blendDesc1;
	CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
	blendDesc.RenderTarget[0].BlendEnable = FALSE; // Disable blending
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState = blendDesc;

	// Use default depth stencil state
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // #TOTHINK Phenomene etrange dans l'ordre de priorité
	psoDesc2.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // #TOTHINK Phenomene etrange dans l'ordre de priorité

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 4;
	psoDesc.RTVFormats[0] = m_pRender->GetRenderResources()->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = m_pRender->GetRenderResources()->Get4xMsaaState() ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_pRender->GetRenderResources()->Get4xMsaaState() ? (m_pRender->GetRenderResources()->Get4xMsaaQuality() - 1) : 0;
	psoDesc.DSVFormat = m_pRender->GetRenderResources()->GetDepthStencilFormat();
	psoDesc.RTVFormats[0] = m_pRender->GetRenderResources()->GetBackBufferFormat();
	psoDesc.SampleDesc.Count = m_pRender->GetRenderResources()->Get4xMsaaState() ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_pRender->GetRenderResources()->Get4xMsaaState() ? (m_pRender->GetRenderResources()->Get4xMsaaQuality() - 1) : 0;
	psoDesc.DSVFormat = m_pRender->GetRenderResources()->GetDepthStencilFormat();
	m_pRender->GetRenderResources()->Getmd3dDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO1));

	psoDesc2.SampleMask = UINT_MAX;
	psoDesc2.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc2.NumRenderTargets = 1;
	psoDesc2.RTVFormats[0] = m_pRender->GetRenderResources()->GetBackBufferFormat();
	psoDesc2.SampleDesc.Count = m_pRender->GetRenderResources()->Get4xMsaaState() ? 4 : 1;
	psoDesc2.SampleDesc.Quality = m_pRender->GetRenderResources()->Get4xMsaaState() ? (m_pRender->GetRenderResources()->Get4xMsaaQuality() - 1) : 0;
	psoDesc2.DSVFormat = m_pRender->GetRenderResources()->GetDepthStencilFormat();
	// Create the graphics pipeline state
	m_pRender->GetRenderResources()->Getmd3dDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO1));
	m_pRender->GetRenderResources()->Getmd3dDevice()->CreateGraphicsPipelineState(&psoDesc2, IID_PPV_ARGS(&m_PSO2));
}

ID3D12RootSignature* GCShader::GetRootSign() 
{
	return m_RootSignature;
}

ID3D12PipelineState* GCShader::GetPso(bool alpha) 
{
	if(alpha)
		return m_PSO2;
	return m_PSO1;
}

ID3DBlob* GCShader::GetmvsByteCode()
{
	return m_vsByteCode;
}

ID3DBlob* GCShader::GetmpsByteCode()
{
	return m_psByteCode;
}

ID3DBlob* GCShader::CompileShaderBase(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint,const std::string& target)
{
	UINT compileFlags = 0;
	#if defined(DEBUG) || defined(_DEBUG)  
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif

	HRESULT hr = S_OK;

	ID3DBlob* byteCode = nullptr;
	ID3DBlob* errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
	{
		OutputDebugStringA((char*)errors->GetBufferPointer());
	}

	return byteCode;
}

void GCShader::SaveShaderToFile(ID3DBlob* shaderBlob, const std::wstring& filename) 
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) 
	{
		throw std::runtime_error("Failed to open file for writing");
	}
	file.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
	file.close();
}

ID3DBlob* GCShader::LoadShaderFromFile(const std::wstring& filename) 
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) 
	{
		throw std::runtime_error("Failed to open file for reading");
	}
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	ID3DBlob* shaderBlob = nullptr;
	HRESULT hr = D3DCreateBlob(size, &shaderBlob);
	if (FAILED(hr)) 
	{
		throw std::runtime_error("Failed to create shader blob");
	}

	if (!file.read(static_cast<char*>(shaderBlob->GetBufferPointer()), size)) 
	{
		shaderBlob->Release();
		throw std::runtime_error("Failed to read file");
	}

	return shaderBlob;
}

void GCShader::PreCompile(const std::string& filePath, const std::string& csoDestinationPath) 
{
	std::wstring wideFilePath(filePath.begin(), filePath.end());
	std::wstring wideCsoDestinationPath(csoDestinationPath.begin(), csoDestinationPath.end());

	ID3DBlob* vsByteCode = CompileShaderBase(wideFilePath, nullptr, "VS", "vs_5_0");
	ID3DBlob* psByteCode = CompileShaderBase(wideFilePath, nullptr, "PS", "ps_5_0");

	SaveShaderToFile(vsByteCode, wideCsoDestinationPath + L"VS.cso");
	SaveShaderToFile(psByteCode, wideCsoDestinationPath + L"PS.cso");
}

GC_GRAPHICS_ERROR GCShader::Load() {
	CompileShader();
	RootSign();
	Pso();

	if (!CHECK_POINTERSNULL("All shader ptr are loaded", "Shader pointers are not correctly loaded", m_RootSignature, m_PSO, m_vsByteCode, m_psByteCode))
		return GCRENDER_ERROR_POINTER_NULL;

	return GCRENDER_SUCCESS_OK;
}

void GCShader::SetRenderTarget(ID3D12Resource* rtt) {
	m_pRtt = rtt;
}




























GCComputeShader::GCComputeShader()
	: m_RootSignature(nullptr), m_PSO(nullptr), m_csByteCode(nullptr), m_pRender(nullptr), m_flagEnabledBits(0), m_cullMode(D3D12_CULL_MODE_NONE)
{
	m_InputLayout.clear();
	m_csCsoPath.clear();
	ZeroMemory(&psoDesc, sizeof(psoDesc));
}

GCComputeShader::~GCComputeShader()
{
	SAFE_RELEASE(m_RootSignature);
	SAFE_RELEASE(m_PSO);
	SAFE_RELEASE(m_csByteCode);
	m_InputLayout.clear();
}

GC_GRAPHICS_ERROR GCComputeShader::Initialize(GCRender* pRender, const std::string& filePath, const std::string& csoDestinationPath, int& flagEnabledBits, D3D12_CULL_MODE cullMode)
{
	if (!pRender) return GCRENDER_ERROR_POINTER_NULL;

	std::ifstream fileCheck(filePath);
	if (!fileCheck.good())
	{
		return GCRENDER_ERROR_SHADER_CREATION_FAILED;
	}

	std::wstring baseCsoPath(csoDestinationPath.begin(), csoDestinationPath.end());
	m_csCsoPath = baseCsoPath + L"CS.cso";

	m_cullMode = cullMode;
	m_pRender = pRender;
	m_flagEnabledBits = flagEnabledBits;

	PreCompile(filePath, csoDestinationPath);

	return GCRENDER_SUCCESS_OK;
}

void GCComputeShader::CompileShader()
{
	m_csByteCode = LoadShaderFromFile(m_csCsoPath);
	if (!m_csByteCode)
	{
		throw std::runtime_error("Failed to load compute shader bytecode.");
	}
}

void GCComputeShader::RootSign()
{
	// Descriptor table for SRV (Texture2D<float4>)
	CD3DX12_DESCRIPTOR_RANGE srvTable;
	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // OriginalImage

	// Descriptor table for UAV (RWTexture2D<float4>)
	CD3DX12_DESCRIPTOR_RANGE uavTable;
	uavTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // OutputImage

	// Define root parameters
	CD3DX12_ROOT_PARAMETER slotRootParameter[2]; // Only 2 parameters: SRV and UAV
	slotRootParameter[0].InitAsDescriptorTable(1, &srvTable); // SRV for OriginalImage
	slotRootParameter[1].InitAsDescriptorTable(1, &uavTable); // UAV for OutputImage

	// Create a root signature descriptor
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// Serialize the root signature
	ID3DBlob* serializedRootSig = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		if (serializedRootSig != nullptr)
		{
			serializedRootSig->Release();
		}
		throw std::runtime_error("Failed to serialize root signature.");
	}

	// Create the root signature
	hr = m_pRender->GetRenderResources()->Getmd3dDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_RootSignature)
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create root signature.");
	}

	// Release the serialized root signature blob
	if (serializedRootSig != nullptr)
	{
		serializedRootSig->Release();
	}
}






void GCComputeShader::Pso()
{
	psoDesc = {};
	psoDesc.pRootSignature = m_RootSignature;
	psoDesc.CS.pShaderBytecode = reinterpret_cast<BYTE*>(m_csByteCode->GetBufferPointer());
	psoDesc.CS.BytecodeLength = m_csByteCode->GetBufferSize();

	HRESULT hr = m_pRender->GetRenderResources()->Getmd3dDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create compute pipeline state object.");
	}
}

ID3D12RootSignature* GCComputeShader::GetRootSign()
{
	return m_RootSignature;
}

ID3D12PipelineState* GCComputeShader::GetPso()
{
	return m_PSO;
}

ID3DBlob* GCComputeShader::GetmcsByteCode()
{
	return m_csByteCode;
}

ID3DBlob* GCComputeShader::CompileShaderBase(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ID3DBlob* byteCode = nullptr;
	ID3DBlob* errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
	{
		OutputDebugStringA((char*)errors->GetBufferPointer());
	}

	return byteCode;
}

void GCComputeShader::SaveShaderToFile(ID3DBlob* shaderBlob, const std::wstring& filename)
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file for writing");
	}
	file.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
	file.close();
}

ID3DBlob* GCComputeShader::LoadShaderFromFile(const std::wstring& filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file for reading");
	}
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	ID3DBlob* shaderBlob = nullptr;
	HRESULT hr = D3DCreateBlob(size, &shaderBlob);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create shader blob");
	}

	if (!file.read(static_cast<char*>(shaderBlob->GetBufferPointer()), size))
	{
		shaderBlob->Release();
		throw std::runtime_error("Failed to read file");
	}

	return shaderBlob;
}

void GCComputeShader::PreCompile(const std::string& filePath, const std::string& csoDestinationPath)
{
	std::wstring wideFilePath(filePath.begin(), filePath.end());
	std::wstring wideCsoDestinationPath(csoDestinationPath.begin(), csoDestinationPath.end());

	ID3DBlob* csByteCode = CompileShaderBase(wideFilePath, nullptr, "CSMain", "cs_5_0");
	SaveShaderToFile(csByteCode, wideCsoDestinationPath + L"CS.cso");
	csByteCode->Release();
}

GC_GRAPHICS_ERROR GCComputeShader::Load()
{
	CompileShader();
	RootSign();
	Pso();

	if (!m_RootSignature || !m_PSO || !m_csByteCode)
	{
		return GCRENDER_ERROR_POINTER_NULL;
	}

	return GCRENDER_SUCCESS_OK;
}