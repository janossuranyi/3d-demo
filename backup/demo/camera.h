#pragma once
#include <string>

struct PerspectiveCamera
{
	float aspectRatio;
	float yfov;
	float zfar;
	float znear;
};

struct OrthographicCamera
{
	float xmag;
	float ymag;
	float zfar;
	float znear;
};

struct Camera
{
	enum Type { PERSPECTIVE, ORTHOGRAPHIC };
	using Ptr = std::shared_ptr<Camera>;
	Camera(Type type_) :
		m_id(-1),
		name("unknown"),
		perspective(),
		ortho(),
		type(type_) {}

	Type type;
	std::string name;
	int m_id;

	int id() const { return m_id; }
	void setId(int id) { m_id = id; }
	union {
		PerspectiveCamera perspective;
		OrthographicCamera ortho;
	};
	
};