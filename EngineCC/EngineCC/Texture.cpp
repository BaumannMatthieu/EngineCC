#include <memory>
#include "Texture.h"

Texture::Texture(const std::string& filename) : m_filename(filename) {
}
Texture::~Texture() {
}

bool Texture::load() {
	bool res = true;
	
	glGenTextures(1, &m_index);
	glBindTexture(GL_TEXTURE_2D, m_index);

	SDL_Surface* data = IMG_Load(m_filename.c_str());
	if (data != NULL) {

		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			data->w,
			data->h,
			0,
			GL_BGR,
			GL_UNSIGNED_BYTE,
			data->pixels);

		SDL_FreeSurface(data);
		std::cout << "Texture succeded to load at path: " << m_filename.c_str() << std::endl;
	}
	else {
		std::cout << "Texture failed to load at path: " << m_filename.c_str() << std::endl;
		res = false;
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	return res;
	//glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(const std::weak_ptr<Shader> program, const std::string& location) const {
	glActiveTexture(GL_TEXTURE0);
	if(auto program_str = program.lock())
		glUniform1i(program_str->getUniformLocation(location), 0);
	glBindTexture(GL_TEXTURE_2D, m_index);
}
