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

	bool load();
	void bind(const std::weak_ptr<Shader> program, const std::string& location) const;

private:
	GLuint m_index;
	std::string m_filename;
};

