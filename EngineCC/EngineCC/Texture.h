#pragma once

#include <string>
#include <iostream>

#include "Dependencies\glew\glew.h"
#include <SDL_image.h>

#include "Shader.h"

class Texture
{
public:
	Texture(const std::string& filename);
	~Texture();

	virtual bool load() = 0;
	virtual void bind(const std::weak_ptr<Shader> program, const std::string& location) const = 0;

protected:
	GLuint m_index;
	std::string m_filename;
};

class SimpleTexture : public Texture {
public:
	SimpleTexture(const std::string& filename);
	~SimpleTexture();

	bool load();
	void bind(const std::weak_ptr<Shader> program, const std::string& location) const;
};

class CubeMapTexture : public Texture {
public:
	CubeMapTexture(const std::string& filename);
	~CubeMapTexture();

	bool load();
	void bind(const std::weak_ptr<Shader> program, const std::string& location) const;
};

