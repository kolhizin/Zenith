#pragma once

#include <glm\glm.hpp>

class SCamera
{
	glm::mat4x4 view_, proj_;
public:
	glm::vec3 pos, dir, up;
	float fovAngle = 90.0f, aspectRatio = 4.0f/3.0f, nearClip = 1e-1f, farClip = 1e3f;

	inline const glm::mat4x4 &getView() const { return view_; }
	inline const glm::mat4x4 &getProj() const { return proj_; }

	inline void * dataViewProj() { return &view_; }
	inline size_t sizeViewProj() const { return sizeof(view_) + sizeof(proj_); }

	inline void updateView()
	{
		glm::vec3 x, y, z;
		z = dir;		
		x = glm::cross(up, z);
		y = glm::normalize(glm::cross(z, x));
		x = glm::normalize(x);
		z = glm::normalize(z);

		float px = -glm::dot(pos, x);
		float py = -glm::dot(pos, y);
		float pz = -glm::dot(pos, z);
		
		
		view_ = glm::mat4x4(x.x, y.x, z.x, 0.0f,
							x.y, y.y, z.y, 0.0f,
							x.z, y.z, z.z, 0.0f,
							px,  py,  pz,  1.0f);
		
		//view_ = glm::mat4x4(1.0f, 0.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f, 0.0f,		0.0f, 0.0f, 1.0f, 0.0f,  	0.0f, 0.0f, 0.0f, 1.0f);
	}
	inline void updateProj()
	{
		float h = 1.0f / glm::tan(glm::radians(fovAngle) * 0.5f);
		float w = h / aspectRatio;
		float zn = nearClip;
		float zf = farClip;
		float q = zf / (zf - zn);
		proj_ = glm::mat4x4(w, 0.0f, 0.0f, 0.0f,
							0.0f, h, 0.0f, 0.0f,
							0.0f, 0.0f, q, 1.0f,
							0.0f, 0.0f, -q*zn, 0.0f);
	}
};



