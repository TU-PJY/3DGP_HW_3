//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once
#include "CONF.h"
#include "Shader.h"
#include "Mesh.h"
#include <cmath>

class Shader;


class OBJ {
public:
	XMFLOAT4X4 Matrix = Mat4::Identity();
	XMFLOAT3 ModelColor{};

	XMFLOAT3 Position{};
	XMFLOAT3 Rotation{};

	XMFLOAT3 Look{0.0, 0.0, 1.0};
	XMFLOAT3 Up{0.0, 1.0, 0.0};
	XMFLOAT3 Right{1.0, 0.0, 0.0};

	Shader* ObjectShader{};
	Mesh* ObjectMesh{};
	Mesh* TerrainMesh{};

	Layer ObjectLayer{};
	std::string Tag{};

	BoundingOrientedBox OOBB = BoundingOrientedBox();


	OBJ() {}
	~OBJ() {}

	void SetMesh(Mesh* MeshData) {
		ObjectMesh = MeshData;
	}

	void SetTerrain(Mesh* TerrainData) {
		TerrainMesh = TerrainData;
	}

	void SetShader(Shader* ShaderData) {
		ObjectShader = ShaderData;
	}

	virtual void Update(float FT) {}

	virtual void Render(ID3D12GraphicsCommandList* CmdList) {
		if (ObjectShader)
			ObjectShader->Render(CmdList);

		UpdateShaderVariables(CmdList);

		if (ObjectMesh)
			ObjectMesh->Render(CmdList);

		if (TerrainMesh)
			TerrainMesh->Render(CmdList);
	}

	virtual void ObjectKeyboardController(UINT nMessageID, WPARAM wParam) {}
	virtual void ObjectMouseController(POINT CursorPos, bool LButtonDownState, bool RButtonDownState) {}
	virtual void ObjectMouseMotionController(POINT PrevCursorPos, bool LButtonDownState, bool RButtonDownState) {}

	void UpdateOOBB() {
		if (ObjectMesh) {
			ObjectMesh->OOBB.Transform(OOBB, XMLoadFloat4x4(&Matrix));
			XMStoreFloat4(&OOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&OOBB.Orientation)));
		}
	}

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* CmdList) {
		XMFLOAT4X4 xmf4x4World;
		XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&Matrix)));
		CmdList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
		CmdList->SetGraphicsRoot32BitConstants(1, 3, &ModelColor, 16);
	}

	virtual void ReleaseUploadBuffers() {
		if (ObjectMesh)
			ObjectMesh->ReleaseUploadBuffers();
	}

	virtual void ReleaseShaderVariables() {}


	void InitTransform() {
		Matrix = Mat4::Identity();
	}

	void SetPosition(float x, float y, float z) {
		Matrix._41 = x;
		Matrix._42 = y;
		Matrix._43 = z;
	}

	void SetPosition(XMFLOAT3 Position) {
		Matrix._41 = Position.x;
		Matrix._42 = Position.y;
		Matrix._43 = Position.z;
	}

	void SetColor(XMFLOAT3 Color) { 
		ModelColor = Color; 
	}

	void MoveStrafe(float Distance) {
		Position = Vec3::Add(Position, Right, Distance);
		SetPosition(Position);
	}


	void MoveForward(float Distance) {
		Position = Vec3::Add(Position, Look, Distance);
		SetPosition(Position);
	}


	void MoveUp(float Distance) {
		Position = Vec3::Add(Position, Up, Distance);
		SetPosition(Position);
	}

	void Rotate(float Pitch, float Yaw, float Roll) {
		XMVECTOR UpVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR LookVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMVECTOR RightVector = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

		XMMATRIX RotateMat = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Pitch), 
			XMConvertToRadians(Yaw), 
			XMConvertToRadians(Roll)
		);

		Matrix = Mat4::Multiply(RotateMat, Matrix);

		Up = {
			XMVectorGetX(XMVector3TransformNormal(UpVector, RotateMat)),
			XMVectorGetY(XMVector3TransformNormal(UpVector, RotateMat)),
			XMVectorGetZ(XMVector3TransformNormal(UpVector, RotateMat))
		};

		Look = {
			XMVectorGetX(XMVector3TransformNormal(LookVector, RotateMat)),
			XMVectorGetY(XMVector3TransformNormal(LookVector, RotateMat)),
			XMVectorGetZ(XMVector3TransformNormal(LookVector, RotateMat))
		};

		Right = {
			XMVectorGetX(XMVector3TransformNormal(RightVector, RotateMat)),
			XMVectorGetY(XMVector3TransformNormal(RightVector, RotateMat)),
			XMVectorGetZ(XMVector3TransformNormal(RightVector, RotateMat))
		};
	}

	void Rotate(XMFLOAT3* Axis, float Angle) {
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(Axis), XMConvertToRadians(Angle));
		Matrix = Mat4::Multiply(mtxRotate, Matrix);
	}

	void LookAt(XMFLOAT3& TargetPosition, XMFLOAT3& UpVector){
		XMFLOAT4X4 xmf4x4View = Mat4::LookAtLH(Position, TargetPosition, UpVector);
		Matrix._11 = xmf4x4View._11; Matrix._12 = xmf4x4View._21; Matrix._13 = xmf4x4View._31;
		Matrix._21 = xmf4x4View._12; Matrix._22 = xmf4x4View._22; Matrix._23 = xmf4x4View._32;
		Matrix._31 = xmf4x4View._13; Matrix._32 = xmf4x4View._23; Matrix._33 = xmf4x4View._33;

		Up = Vec3::Normalize(XMFLOAT3(Matrix._21, Matrix._22, Matrix._23));
		Right = Vec3::Normalize(XMFLOAT3(Matrix._11, Matrix._12, Matrix._13));
		Look = Vec3::Normalize(XMFLOAT3(Matrix._31, Matrix._32, Matrix._33));
	}

	void UpdateRotation(float Pitch, float Yaw, float Roll) {
		Rotation.x += Pitch;
		Rotation.y += Yaw;
		Rotation.z += Roll;
	}

	void Scale(float ScaleX, float ScaleY, float ScaleZ ) {
		XMMATRIX scaleMatrix = XMMatrixScaling(ScaleX, ScaleY, ScaleZ);
		Matrix = Mat4::Multiply(scaleMatrix, Matrix);
	}

	void LinearAcc(float& CurrentSpeed, float SpeedLimit, float AccelerationValue, float FT) {
		if (SpeedLimit > 0.0) {
			CurrentSpeed += AccelerationValue * FT;
			if (CurrentSpeed >= SpeedLimit)
				CurrentSpeed = SpeedLimit;
		}

		else if (SpeedLimit < 0.0) {
			CurrentSpeed -= AccelerationValue * FT;
			if (CurrentSpeed <= SpeedLimit)
				CurrentSpeed = SpeedLimit;
		}
	}

	void LinearDcc(float& CurrentSpeed, float DecelerationValue, float FT) {
		if (CurrentSpeed > 0.0) {
			CurrentSpeed -= DecelerationValue * FT;
			if (CurrentSpeed <= 0.0)
				CurrentSpeed = 0.0;
		}

		else if (CurrentSpeed < 0.0) {
			CurrentSpeed += DecelerationValue * FT;
			if (CurrentSpeed >= 0.0)
				CurrentSpeed = 0.0;
		}
	}

	void LerpAcc(float& CurrentSpeed, float SpeedLimit, float AccelerationValue, float FT) {
		CurrentSpeed = std::lerp(CurrentSpeed, SpeedLimit, AccelerationValue * FT);
	}

	void LerpDcc(float& CurrentSpeed, float DecelerationValue, float FT) {
		CurrentSpeed = std::lerp(CurrentSpeed, 0.0, DecelerationValue * FT);
	}


	void GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection) {
		XMMATRIX xmmtxToModel = XMMatrixInverse(NULL, XMLoadFloat4x4(&Matrix) * xmmtxView);
		XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
		xmvPickRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraOrigin), xmmtxToModel);
		xmvPickRayDirection = XMVector3TransformCoord(xmvPickPosition, xmmtxToModel);
		xmvPickRayDirection = XMVector3Normalize(xmvPickRayDirection - xmvPickRayOrigin);
	}

	int PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance) {
		int nIntersected = 0;

		if (ObjectMesh){
			XMVECTOR xmvPickRayOrigin, xmvPickRayDirection;
			GenerateRayForPicking(xmvPickPosition, xmmtxView, xmvPickRayOrigin, xmvPickRayDirection);
			nIntersected = ObjectMesh->CheckRayIntersection(xmvPickRayOrigin, xmvPickRayDirection, pfHitDistance);
		}

		return(nIntersected);
	}
};