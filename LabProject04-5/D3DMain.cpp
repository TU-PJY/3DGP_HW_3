#include "D3DMain.h"
#include "CONF.h"
#include "Camera.h"
#include "Framework.h"


void D3DMain::Init() {
	CmdList->Reset(CmdAllocator, NULL);

	fw.Init(Device, CmdList);

	cam.SetPosition(XMFLOAT3(0.0, 0.0, 0.0));
	cam.SetOffset(XMFLOAT3(0.0f, 5.0f, -13.0f));
	cam.GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 45.0f);
	cam.SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	cam.SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	cam.SetTimeLag(0.1f);
	cam.SetCameraMode(CamMode::MODE1);

	CmdList->Close();
	ID3D12CommandList* CmdLists[] = { CmdList };
	CmdQueue->ExecuteCommandLists(1, CmdLists);

	WaitForGpuComplete();

	fw.ReleaseUploadBuffers();
	Timer.Reset();
}


LRESULT CALLBACK D3DMain::WindowsMessegeFunc(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	switch (nMessageID) {
	case WM_ACTIVATE: 
		if (LOWORD(wParam) == WA_INACTIVE)
			Timer.Stop();
		else
			Timer.Start();
		break;

	case WM_SIZE:
		break;

	case WM_LBUTTONDOWN: case WM_RBUTTONDOWN:
	case WM_LBUTTONUP: case WM_RBUTTONUP:
		fw.MouseController(hWnd, nMessageID, wParam, lParam);
		break;

	case WM_KEYDOWN: case WM_KEYUP:
		cam.CameraKeyboardController(hWnd, nMessageID, wParam, lParam);
		fw.KeyboardController(hWnd, nMessageID, wParam, lParam);
		break;
	}

	return 0;
}


void D3DMain::Update() {
	cam.RegenerateViewMatrix();
	cam.Update(Timer.GetTimeElapsed());

	fw.MouseMotionController(hWnd);
	fw.Update(Timer.GetTimeElapsed());
}


void D3DMain::Render(ID3D12GraphicsCommandList* CmdList) {
	fw.PrepareRender(CmdList);
	//UpdateShaderVariables();
	fw.Render(CmdList);
}


void D3DMain::Routine() {
	Timer.Tick(165.0f);

	HRESULT hResult = CmdAllocator->Reset();
	hResult = CmdList->Reset(CmdAllocator, NULL);

	D3D12_RESOURCE_BARRIER ResourceBarrier;
	::ZeroMemory(&ResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.Transition.pResource = SwapChainBackBuffers[SwapChainBufferIndex];
	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	CmdList->ResourceBarrier(1, &ResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE RtvCPUDescriptorHandle = RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	RtvCPUDescriptorHandle.ptr += (SwapChainBufferIndex * RtvDescriptorIncrementSize);

	float BackGroundColor[4] = { 0.196078, 0.6, 0.8, 1.0f };
	CmdList->ClearRenderTargetView(RtvCPUDescriptorHandle, BackGroundColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE DsvCPUDescriptorHandle = DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	CmdList->ClearDepthStencilView(DsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	CmdList->OMSetRenderTargets(1, &RtvCPUDescriptorHandle, TRUE, &DsvCPUDescriptorHandle);

	Update();
	Render(CmdList);

#ifdef _WITH_PLAYER_TOP
	CmdList->ClearDepthStencilView(DsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif

	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	CmdList->ResourceBarrier(1, &ResourceBarrier);

	hResult = CmdList->Close();

	ID3D12CommandList* CmdLists[] = { CmdList };
	CmdQueue->ExecuteCommandLists(1, CmdLists);

	WaitForGpuComplete();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	DxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	DxgiSwapChain->Present(1, 0);
#else
	DxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	Timer.GetFrameRate(FrameRate + (D3D_WindowNameLength - 1), 37);
	::SetWindowText(hWnd, FrameRate);
}

void D3DMain::SwitchToWindowMode(HWND hwnd) {
	FRAME_BUFFER_WIDTH /= 2;
	FRAME_BUFFER_HEIGHT /= 2 * ASPECT_RATIO;

	SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
	ShowWindow(hwnd, SW_NORMAL);
	SetWindowPos(hwnd, NULL, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, SWP_FRAMECHANGED | SWP_NOZORDER );
	FullScreenState = FALSE;
}

void D3DMain::SwitchToFullscreenMode(HWND hwnd) {
	FRAME_BUFFER_WIDTH = GetSystemMetrics(SM_CXSCREEN);
	FRAME_BUFFER_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

	SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
	ShowWindow(hwnd, SW_MAXIMIZE);
	SetWindowPos(hwnd, HWND_TOP, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, SWP_FRAMECHANGED | SWP_NOZORDER);
	FullScreenState = TRUE;
}

D3DMain::D3DMain() {
	DxgiFactory = NULL;
	DxgiSwapChain = NULL;
	Device = NULL;

	for (int i = 0; i < SwapChainBuffers; i++) 
		SwapChainBackBuffers[i] = NULL;

	SwapChainBufferIndex = 0;

	CmdAllocator = NULL;
	CmdQueue = NULL;
	CmdList = NULL;

	RtvDescriptorHeap = NULL;
	DsvDescriptorHeap = NULL;

	RtvDescriptorIncrementSize = 0;
	DsvDescriptorIncrementSize = 0;

	FenceEvent = NULL;
	m_pd3dFence = NULL;

	for (int i = 0; i < SwapChainBuffers; i++) 
		FenceValues[i] = 0;

	CLIENT_WIDTH = FRAME_BUFFER_WIDTH;
	CLIENT_HEIGHT = FRAME_BUFFER_HEIGHT;

	memset(WindowName, 0, sizeof(WindowName));
	_tcscpy_s(WindowName, D3D_WindowNameLength, D3D_WindowName);
	_tcscpy_s(FrameRate, WindowName);
}


D3DMain::~D3DMain() {}


bool D3DMain::Create(HINSTANCE hInstance, HWND hMainWnd) {
	hInstance = hInstance;
	hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	Init();

	return(true);
}


void D3DMain::CreateSwapChain() {
	RECT rcClient;
	::GetClientRect(hWnd, &rcClient);
	CLIENT_WIDTH = rcClient.right - rcClient.left;
	CLIENT_HEIGHT = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 DxgiSwapChainDesc;
	::ZeroMemory(&DxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	DxgiSwapChainDesc.Width = CLIENT_WIDTH;
	DxgiSwapChainDesc.Height = CLIENT_HEIGHT;
	DxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DxgiSwapChainDesc.SampleDesc.Count = (MSAA_4x_Option) ? 4 : 1;
	DxgiSwapChainDesc.SampleDesc.Quality = (MSAA_4x_Option) ? (MSAA_4x_QualityLevels - 1) : 0;
	DxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	DxgiSwapChainDesc.BufferCount = SwapChainBuffers;
	DxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	DxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	DxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DxgiSwapChainDesc.Flags = 0;
#else
	DxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = DxgiFactory->CreateSwapChainForHwnd(CmdQueue, hWnd, &DxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)&DxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC DxgiSwapChainDesc;
	::ZeroMemory(&DxgiSwapChainDesc, sizeof(DxgiSwapChainDesc));
	DxgiSwapChainDesc.BufferCount = SwapChainBuffers;
	DxgiSwapChainDesc.BufferDesc.Width = CLIENT_WIDTH;
	DxgiSwapChainDesc.BufferDesc.Height = CLIENT_HEIGHT;
	DxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	DxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	DxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	DxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	DxgiSwapChainDesc.OutputWindow = hWnd;
	DxgiSwapChainDesc.SampleDesc.Count = (MSAA_4x_Option) ? 4 : 1;
	DxgiSwapChainDesc.SampleDesc.Quality = (MSAA_4x_Option) ? (MSAA_4x_QualityLevels - 1) : 0;
	DxgiSwapChainDesc.Windowed = TRUE;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DxgiSwapChainDesc.Flags = 0;
#else
	DxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	HRESULT hResult = DxgiFactory->CreateSwapChain(CmdQueue, &DxgiSwapChainDesc, (IDXGISwapChain**)&DxgiSwapChain);
#endif

	hResult = DxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
	SwapChainBufferIndex = DxgiSwapChain->GetCurrentBackBufferIndex();

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}


void D3DMain::CreateDirect3DDevice() {
	HRESULT hResult;

	UINT DxgiFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(DxgiFactoryFlags, __uuidof(IDXGIFactory4), (void**)&DxgiFactory);

	IDXGIAdapter1* Adapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != DxgiFactory->EnumAdapters1(i, &Adapter); i++) {
		DXGI_ADAPTER_DESC1 DxgiAdapterDesc;
		Adapter->GetDesc1(&DxgiAdapterDesc);
		if (DxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) 
			continue;

		if (SUCCEEDED(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&Device))) 
			break;
	}

	if (!Device) {
		DxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)&Adapter);
		hResult = D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&Device);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAA_QualityLevels;
	MSAA_QualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MSAA_QualityLevels.SampleCount = 4;
	MSAA_QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MSAA_QualityLevels.NumQualityLevels = 0;
	hResult = Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MSAA_QualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	MSAA_4x_QualityLevels = MSAA_QualityLevels.NumQualityLevels;
	MSAA_4x_Option = (MSAA_4x_QualityLevels > 1) ? true : false;

	hResult = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);

	for (UINT i = 0; i < SwapChainBuffers; i++)
		FenceValues[i] = 0;

	FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if (Adapter) 
		Adapter->Release();

	::CbvSrvDescriptorIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	RtvDescriptorIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}


void D3DMain::CreateCommandQueueAndList() {
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	::ZeroMemory(&CommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = Device->CreateCommandQueue(&CommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&CmdQueue);

	hResult = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&CmdAllocator);

	hResult = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CmdAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&CmdList);
	hResult = CmdList->Close();
}


void D3DMain::CreateRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
	::ZeroMemory(&DescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	DescriptorHeapDesc.NumDescriptors = SwapChainBuffers;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&RtvDescriptorHeap);

	DescriptorHeapDesc.NumDescriptors = 1;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&DsvDescriptorHeap);
}


void D3DMain::CreateRenderTargetViews() {
	D3D12_CPU_DESCRIPTOR_HANDLE RtvCPUDescriptorHandle = RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < SwapChainBuffers; i++) {
		DxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&SwapChainBackBuffers[i]);
		Device->CreateRenderTargetView(SwapChainBackBuffers[i], NULL, RtvCPUDescriptorHandle);
		RtvCPUDescriptorHandle.ptr += RtvDescriptorIncrementSize;
	}
}


void D3DMain::CreateDepthStencilView() {

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = CLIENT_WIDTH;
	ResourceDesc.Height = CLIENT_HEIGHT;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ResourceDesc.SampleDesc.Count = (MSAA_4x_Option) ? 4 : 1;
	ResourceDesc.SampleDesc.Quality = (MSAA_4x_Option) ? (MSAA_4x_QualityLevels - 1) : 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES HeapProperties;
	::ZeroMemory(&HeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.CreationNodeMask = 1;
	HeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE DsvCPUDescriptorHandle = DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &ClearValue, __uuidof(ID3D12Resource), (void**)&DepthStencilBuffer);
	/*
		Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_COMMON, &ClearValue, __uuidof(ID3D12Resource), (void **)&DepthStencilBuffer);
		Device->CreateDepthStencilView(DepthStencilBuffer, NULL, DsvCPUDescriptorHandle);

		D3D12_RESOURCE_BARRIER ResourceBarrier;
		::ZeroMemory(&ResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
		ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		ResourceBarrier.Transition.pResource = DepthStencilBuffer;
		ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		CmdList->ResourceBarrier(1, &ResourceBarrier);
	*/

	D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
	::ZeroMemory(&DepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	Device->CreateDepthStencilView(DepthStencilBuffer, &DepthStencilViewDesc, DsvCPUDescriptorHandle);
}


void D3DMain::CreateRenderTargetViewsAndDepthStencilView() {
	CmdList->Reset(CmdAllocator, NULL);

	CreateRenderTargetViews();
	CreateDepthStencilView();

	CmdList->Close();
	ID3D12CommandList* CmdLists[] = { CmdList };
	CmdQueue->ExecuteCommandLists(1, CmdLists);

	WaitForGpuComplete();
}


void D3DMain::ChangeSwapChainState() {
	WaitForGpuComplete();

	BOOL FullScreenState = FALSE;
	DxgiSwapChain->GetFullscreenState(&FullScreenState, NULL);
	DxgiSwapChain->SetFullscreenState(!FullScreenState, NULL);

	DXGI_MODE_DESC DxgiTargetParameters;
	DxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DxgiTargetParameters.Width = CLIENT_WIDTH;
	DxgiTargetParameters.Height = CLIENT_HEIGHT;
	DxgiTargetParameters.RefreshRate.Numerator = 60;
	DxgiTargetParameters.RefreshRate.Denominator = 1;
	DxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	DxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	DxgiSwapChain->ResizeTarget(&DxgiTargetParameters);

	for (int i = 0; i < SwapChainBuffers; i++) 
		if (SwapChainBackBuffers[i]) 
			SwapChainBackBuffers[i]->Release();

#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DXGI_SWAP_CHAIN_DESC DxgiSwapChainDesc;
	DxgiSwapChain->GetDesc(&DxgiSwapChainDesc);
	DxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
#else
	DXGI_SWAP_CHAIN_DESC DxgiSwapChainDesc;
	DxgiSwapChain->GetDesc(&DxgiSwapChainDesc);
	DxgiSwapChain->ResizeBuffers(SwapChainBuffers, CLIENT_WIDTH, CLIENT_HEIGHT, DxgiSwapChainDesc.BufferDesc.Format, DxgiSwapChainDesc.Flags);
#endif
	SwapChainBufferIndex = DxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}


void D3DMain::Destroy() {
	ReleaseObjects();

	::CloseHandle(FenceEvent);

	if (DepthStencilBuffer) 
		DepthStencilBuffer->Release();

	if (DsvDescriptorHeap) 
		DsvDescriptorHeap->Release();

	for (int i = 0; i < SwapChainBuffers; i++) {
		if (SwapChainBackBuffers[i])
			SwapChainBackBuffers[i]->Release();
	}

	if (RtvDescriptorHeap)
		RtvDescriptorHeap->Release();

	if (CmdAllocator) 
		CmdAllocator->Release();

	if (CmdQueue) 
		CmdQueue->Release();

	if (CmdList) 
		CmdList->Release();

	if (m_pd3dFence) 
		m_pd3dFence->Release();

	DxgiSwapChain->SetFullscreenState(FALSE, NULL);

	if (DxgiSwapChain) 
		DxgiSwapChain->Release();

	if (Device) 
		Device->Release();

	if (DxgiFactory) 
		DxgiFactory->Release();

#if defined(_DEBUG)
	if (DebugController) DebugController->Release();
#endif
}


void D3DMain::ReleaseObjects() {
	fw.ReleaseObjects();
}


void D3DMain::CreateShaderVariables(){}


void D3DMain::ReleaseShaderVariables(){}


void D3DMain::UpdateShaderVariables() {
	float CurrentTime = Timer.GetTotalTime();
	float ElapsedTime = Timer.GetTimeElapsed();

	CmdList->SetGraphicsRoot32BitConstants(0, 1, &CurrentTime, 0);
	CmdList->SetGraphicsRoot32BitConstants(0, 1, &ElapsedTime, 1);

	POINT CursorPos;
	::GetCursorPos(&CursorPos);
	::ScreenToClient(hWnd, &CursorPos);
	float xCursorPos = (CursorPos.x < 0) ? 0.0f : float(CursorPos.x);
	float yCursorPos = (CursorPos.y < 0) ? 0.0f : float(CursorPos.y);

	CmdList->SetGraphicsRoot32BitConstants(0, 1, &xCursorPos, 2);
	CmdList->SetGraphicsRoot32BitConstants(0, 1, &yCursorPos, 3);
}


void D3DMain::WaitForGpuComplete() {
	UINT64 FenceValue = ++FenceValues[SwapChainBufferIndex];
	HRESULT hResult = CmdQueue->Signal(m_pd3dFence, FenceValue);

	if (m_pd3dFence->GetCompletedValue() < FenceValue) {
		hResult = m_pd3dFence->SetEventOnCompletion(FenceValue, FenceEvent);
		::WaitForSingleObject(FenceEvent, INFINITE);
	}
}


void D3DMain::MoveToNextFrame() {
	SwapChainBufferIndex = DxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 FenceValue = ++FenceValues[SwapChainBufferIndex];
	HRESULT hResult = CmdQueue->Signal(m_pd3dFence, FenceValue);

	if (m_pd3dFence->GetCompletedValue() < FenceValue) {
		hResult = m_pd3dFence->SetEventOnCompletion(FenceValue, FenceEvent);
		::WaitForSingleObject(FenceEvent, INFINITE);
	}
}