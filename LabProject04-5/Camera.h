#pragma once
#include "Conf.h"
#include "Object.h"
#define ASPECT_RATIO				(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

class Camera {
protected:
	XMFLOAT3						CamPos{};

	XMFLOAT3						CamRight{};
	XMFLOAT3						CamUp{};
	XMFLOAT3						CamLook{};

	float           				CamPitch{};
	float           				CamRoll{};
	float           				CamYaw{};

	XMFLOAT3						CamLookAtWorld{};
	XMFLOAT3						CamOffset{};
	float           				CamTimeDelay{};


#ifdef _WITH_DIERECTX_MATH_FRUSTUM
	BoundingFrustum					CamFrustumView{};
	BoundingFrustum					CamFrustumWorld{};
	XMFLOAT4X4						Cam4x4InverseView{};
#else
	XMFLOAT4						CamFrustumPlanes[6]{};
#endif

	D3D12_RECT						CamScissorRect{};

private:
	CamMode RunningMode{};


public:
	D3D12_VIEWPORT					CamViewport{};
	XMFLOAT4X4						Cam4x4View{};
	XMFLOAT4X4						Cam4x4Projection{};

	Camera() {
		Cam4x4View = Mat4::Identity();
		Cam4x4Projection = Mat4::Identity();

		CamViewport = { 0, 0, float(FRAME_BUFFER_WIDTH), float(FRAME_BUFFER_HEIGHT), 0.0f, 1.0f };
		CamScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };

		CamPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		CamPitch = 0.0f;
		CamRoll = 0.0f;
		CamYaw = 0.0f;

		CamRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
		CamLook = XMFLOAT3(0.0f, 0.0f, 1.0f);
		CamUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
		CamOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		CamLookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);

		CamTimeDelay = 0.0f;
	}

	virtual ~Camera() {}

	void Update(float FT);

	void Camera::CameraKeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList) {}

	virtual void ReleaseShaderVariables() {}

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* CmdList) {
		XMFLOAT4X4 xmf4x4View;
		XMStoreFloat4x4(&xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&Cam4x4View)));
		CmdList->SetGraphicsRoot32BitConstants(2, 16, &xmf4x4View, 0);

		XMFLOAT4X4 xmf4x4Projection;
		XMStoreFloat4x4(&xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&Cam4x4Projection)));
		CmdList->SetGraphicsRoot32BitConstants(2, 16, &xmf4x4Projection, 16);

		CmdList->SetGraphicsRoot32BitConstants(2, 3, &CamPos, 32);
	}

	void GenerateViewMatrix() {
		Cam4x4View = Mat4::LookAtLH(CamPos, CamLookAtWorld, CamUp);
	}

	void GenerateViewMatrix(XMFLOAT3 Position, XMFLOAT3 LookAt, XMFLOAT3 Up) {
		CamPos = Position;
		CamLookAtWorld = LookAt;
		CamUp = Up;

		GenerateViewMatrix();
	}

	void RegenerateViewMatrix() {
		CamLook = Vec3::Normalize(CamLook);
		CamRight = Vec3::CrossProduct(CamUp, CamLook, true);
		CamUp = Vec3::CrossProduct(CamLook, CamRight, true);

		Cam4x4View._11 = CamRight.x; Cam4x4View._12 = CamUp.x; Cam4x4View._13 = CamLook.x;
		Cam4x4View._21 = CamRight.y; Cam4x4View._22 = CamUp.y; Cam4x4View._23 = CamLook.y;
		Cam4x4View._31 = CamRight.z; Cam4x4View._32 = CamUp.z; Cam4x4View._33 = CamLook.z;

		Cam4x4View._41 = -Vec3::DotProduct(CamPos, CamRight);
		Cam4x4View._42 = -Vec3::DotProduct(CamPos, CamUp);
		Cam4x4View._43 = -Vec3::DotProduct(CamPos, CamLook);

		Cam4x4InverseView._11 = CamRight.x; Cam4x4InverseView._12 = CamRight.y; Cam4x4InverseView._13 = CamRight.z;
		Cam4x4InverseView._21 = CamUp.x; Cam4x4InverseView._22 = CamUp.y; Cam4x4InverseView._23 = CamUp.z;
		Cam4x4InverseView._31 = CamLook.x; Cam4x4InverseView._32 = CamLook.y; Cam4x4InverseView._33 = CamLook.z;
		Cam4x4InverseView._41 = CamPos.x; Cam4x4InverseView._42 = CamPos.y; Cam4x4InverseView._43 = CamPos.z;

		CamFrustumView.Transform(CamFrustumWorld, XMLoadFloat4x4(&Cam4x4InverseView));
	}

	void GenerateProjectionMatrix(float NearPlane, float FarPlane, float AspRatio, float Fov) {
		//	Cam4x4Projection = Mat4::PerspectiveFovLH(XMConvertToRadians(Fov), AspRatio, NearPlane, FarPlane);
		XMMATRIX Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), AspRatio, NearPlane, FarPlane);
		XMStoreFloat4x4(&Cam4x4Projection, Projection);

#ifdef _WITH_DIERECTX_MATH_FRUSTUM
		BoundingFrustum::CreateFromMatrix(CamFrustumView, Projection);
#endif
	}

	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float zMin = 0.0f, float zMax = 1.0f) {
		CamViewport.TopLeftX = float(xTopLeft);
		CamViewport.TopLeftY = float(yTopLeft);
		CamViewport.Width = float(nWidth);
		CamViewport.Height = float(nHeight);
		CamViewport.MinDepth = zMin;
		CamViewport.MaxDepth = zMax;
	}

	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom) {
		CamScissorRect.left = xLeft;
		CamScissorRect.top = yTop;
		CamScissorRect.right = xRight;
		CamScissorRect.bottom = yBottom;
	}

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList* CmdList) {
		CmdList->RSSetViewports(1, &CamViewport);
		CmdList->RSSetScissorRects(1, &CamScissorRect);
	}

	void SetCameraMode(CamMode Mode) {
		RunningMode = Mode;
	}

	void SetPosition(XMFLOAT3 Position) { CamPos = Position; }
	XMFLOAT3& GetPosition() { return(CamPos); }

	void SetLookAtPosition(XMFLOAT3 LookAt) { CamLookAtWorld = LookAt; }
	XMFLOAT3& GetLookAtPosition() { return(CamLookAtWorld); }

	XMFLOAT3& GetRightVector() { return(CamRight); }
	XMFLOAT3& GetUpVector() { return(CamUp); }
	XMFLOAT3& GetLookVector() { return(CamLook); }

	float& GetPitch() { return(CamPitch); }
	float& GetRoll() { return(CamRoll); }
	float& GetYaw() { return(CamYaw); }

	void SetOffset(XMFLOAT3 Offset) { CamOffset = Offset; }
	XMFLOAT3& GetOffset() { return(CamOffset); }

	void SetTimeLag(float DelayValue) { CamTimeDelay = DelayValue; }
	float GetTimeLag() { return(CamTimeDelay); }

	XMFLOAT4X4 GetViewMatrix() { return(Cam4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(Cam4x4Projection); }
	D3D12_VIEWPORT GetViewport() { return(CamViewport); }
	D3D12_RECT GetScissorRect() { return(CamScissorRect); }

	void Move(const XMFLOAT3& Shift) { 
		CamPos.x += Shift.x; 
		CamPos.y += Shift.y; 
		CamPos.z += Shift.z; 
	}

	void Track(XMFLOAT3& LookAt, OBJ* Object, float fTimeElapsed) { 
		XMFLOAT4X4 xmf4x4Rotate = Mat4::Identity();
		XMFLOAT3 xmf3Right = Object->Right;
		XMFLOAT3 xmf3Up = Object->Up;
		XMFLOAT3 xmf3Look = Object->Look;

		xmf4x4Rotate._11 = xmf3Right.x;
		xmf4x4Rotate._12 = xmf3Right.y;
		xmf4x4Rotate._13 = xmf3Right.z;

		xmf4x4Rotate._21 = xmf3Up.x;
		xmf4x4Rotate._22 = xmf3Up.y;
		xmf4x4Rotate._23 = xmf3Up.z;

		xmf4x4Rotate._31 = xmf3Look.x;
		xmf4x4Rotate._32 = xmf3Look.y;
		xmf4x4Rotate._33 = xmf3Look.z;

		XMFLOAT3 xmf3Offset = Vec3::TransformCoord(CamOffset, xmf4x4Rotate);
		XMFLOAT3 xmf3Position = Vec3::Add(Object->Position, xmf3Offset);
		XMFLOAT3 xmf3Direction = Vec3::Subtract(xmf3Position, CamPos);

		float fLength = Vec3::Length(xmf3Direction);
		xmf3Direction = Vec3::Normalize(xmf3Direction);

		float fTimeLagScale = (CamTimeDelay) ? fTimeElapsed * (1.0f / CamTimeDelay) : 1.0f;
		float fDistance = fLength * fTimeLagScale;

		if (fDistance > fLength) 
			fDistance = fLength;

		if (fLength < 0.01f) 
			fDistance = fLength;

		CamPos = Vec3::Add(CamPos, xmf3Direction, fDistance);
		SetLookAt(LookAt, Object);
	}

	void SetLookAt(XMFLOAT3& LookAt, OBJ* Object) {
		XMFLOAT4X4 mtxLookAt = Mat4::LookAtLH(CamPos, LookAt, Object->Up);
		CamRight = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
		CamUp = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
		CamLook = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
	}

	void CalculateFrustumPlanes() {
#ifdef _WITH_DIERECTX_MATH_FRUSTUM
		CamFrustumView.Transform(CamFrustumWorld, XMMatrixInverse(NULL, XMLoadFloat4x4(&Cam4x4View)));
#else
		XMFLOAT4X4 CamViewProjection = Mat4::Multiply(Cam4x4View, Cam4x4Projection);

		CamFrustumPlanes[0].x = -(CamViewProjection._14 + CamViewProjection._11);
		CamFrustumPlanes[0].y = -(CamViewProjection._24 + CamViewProjection._21);
		CamFrustumPlanes[0].z = -(CamViewProjection._34 + CamViewProjection._31);
		CamFrustumPlanes[0].w = -(CamViewProjection._44 + CamViewProjection._41);

		CamFrustumPlanes[1].x = -(CamViewProjection._14 - CamViewProjection._11);
		CamFrustumPlanes[1].y = -(CamViewProjection._24 - CamViewProjection._21);
		CamFrustumPlanes[1].z = -(CamViewProjection._34 - CamViewProjection._31);
		CamFrustumPlanes[1].w = -(CamViewProjection._44 - CamViewProjection._41);

		CamFrustumPlanes[2].x = -(CamViewProjection._14 - CamViewProjection._12);
		CamFrustumPlanes[2].y = -(CamViewProjection._24 - CamViewProjection._22);
		CamFrustumPlanes[2].z = -(CamViewProjection._34 - CamViewProjection._32);
		CamFrustumPlanes[2].w = -(CamViewProjection._44 - CamViewProjection._42);

		CamFrustumPlanes[3].x = -(CamViewProjection._14 + CamViewProjection._12);
		CamFrustumPlanes[3].y = -(CamViewProjection._24 + CamViewProjection._22);
		CamFrustumPlanes[3].z = -(CamViewProjection._34 + CamViewProjection._32);
		CamFrustumPlanes[3].w = -(CamViewProjection._44 + CamViewProjection._42);

		CamFrustumPlanes[4].x = -(CamViewProjection._13);
		CamFrustumPlanes[4].y = -(CamViewProjection._23);
		CamFrustumPlanes[4].z = -(CamViewProjection._33);
		CamFrustumPlanes[4].w = -(CamViewProjection._43);

		CamFrustumPlanes[5].x = -(CamViewProjection._14 - CamViewProjection._13);
		CamFrustumPlanes[5].y = -(CamViewProjection._24 - CamViewProjection._23);
		CamFrustumPlanes[5].z = -(CamViewProjection._34 - CamViewProjection._33);
		CamFrustumPlanes[5].w = -(CamViewProjection._44 - CamViewProjection._43);

		for (int i = 0; i < 6; i++) 
			CamFrustumPlanes[i] = Plane::Normalize(CamFrustumPlanes[i]);
#endif
	}

	bool IsInFrustum(BoundingBox& BoundingBox) {
#ifdef _WITH_DIERECTX_MATH_FRUSTUM
		return(CamFrustumWorld.Intersects(BoundingBox));
#else
		XMFLOAT3 NearPoint, Normal;
		XMFLOAT3 Min = Vec3::Subtract(BoundingBox.Center, BoundingBox.Extents);
		XMFLOAT3 Max = Vec3::Add(BoundingBox.Center, BoundingBox.Extents);
		for (int i = 0; i < 6; i++) {
			Normal = XMFLOAT3(CamFrustumPlanes[i].x, CamFrustumPlanes[i].y, CamFrustumPlanes[i].z);

			if (Normal.x >= 0.0f) {
				if (Normal.y >= 0.0f) {
					if (Normal.z >= 0.0f) 
						NearPoint = XMFLOAT3(Min.x, Min.y, Min.z);
					
					else
						NearPoint = XMFLOAT3(Min.x, Min.y, Max.z);
				}

				else {
					if (Normal.z >= 0.0f)
						NearPoint = XMFLOAT3(Min.x, Max.y, Min.z);
					
					else
						NearPoint = XMFLOAT3(Min.x, Max.y, Max.z);
				}
			}

			else {
				if (Normal.y >= 0.0f) {
					if (Normal.z >= 0.0f)
						NearPoint = XMFLOAT3(Max.x, Min.y, Min.z);
					
					else
						NearPoint = XMFLOAT3(Max.x, Min.y, Max.z);
				}

				else {
					if (Normal.z >= 0.0f)
						NearPoint = XMFLOAT3(Max.x, Max.y, Min.z);
					
					else
						NearPoint = XMFLOAT3(Max.x, Max.y, Max.z);
				}
			}

			if ((Vec3::DotProduct(Normal, NearPoint) + CamFrustumPlanes[i].w) > 0.0f)
				return(false);
		}

		return(true);
#endif
	}

	bool IsInFrustum(BoundingOrientedBox& BoundingBox) {
#ifdef _WITH_DIERECTX_MATH_FRUSTUM
		return(CamFrustumWorld.Intersects(BoundingBox));
#else
#endif
	}
};

extern Camera cam;