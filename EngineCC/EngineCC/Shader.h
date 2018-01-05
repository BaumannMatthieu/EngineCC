#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#include "Dependencies\glew\glew.h"

class Shader
{
public:
	void read_file(const std::string& filename, std::string& content) {
		std::ifstream file(filename);
		std::string line;
		if (file.is_open()) {
			while (getline(file, line)) {
				content += line + '\n';
			}
		}
		file.close();
	}

	void attachShader(const std::string& filemane, GLuint shader_type) {
		// Instantiate object shader (e.g. vertex, geometry, fragment)
		GLuint shader_object = glCreateShader(shader_type);

		// Read the shader object file and store its content in a string
		std::string content;
		read_file(filemane, content);
		const GLchar* p[1];
		p[0] = content.c_str();

		// Association of the string with its object and compilation
		GLint length = content.size();
		glShaderSource(shader_object, 1, p, &length);
		glCompileShader(shader_object);

		GLint success;
		glGetShaderiv(shader_object, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar log[1024];
			glGetShaderInfoLog(shader_object, sizeof(log), NULL, log);
			fprintf(stderr, "Error compiling shader type %d: '%s'\n", GL_VERTEX_SHADER, log);
			exit(0);
		}

		// Attach the shader object to its program
		glAttachShader(m_program, shader_object);
	}

	void linkProgram() {
		// Link the program and check if all is ok
		glLinkProgram(m_program);

		GLint success;
		glGetProgramiv(m_program, GL_LINK_STATUS, &success);
		if (success == 0) {
			GLchar log[1024];
			glGetProgramInfoLog(m_program, sizeof(log), NULL, log);
			fprintf(stderr, "Error linking shader program: '%s'\n", log);
			exit(0);
		}
	}

	Shader(const std::string& vertex_obj_filename,
		   const std::string& fragment_obj_filename) {
		// Create shader program
		m_program = glCreateProgram();
		attachShader(vertex_obj_filename, GL_VERTEX_SHADER);
		attachShader(fragment_obj_filename, GL_FRAGMENT_SHADER);

		linkProgram();

		//Validate the program. Do it once after linking
		glValidateProgram(m_program);
	}

	~Shader() {

	}

	void bind() const {
		glUseProgram(m_program);
	}

	GLuint getUniformLocation(const std::string& uniform) const {
		return glGetUniformLocation(m_program, static_cast<const GLchar*>(uniform.c_str()));
	}
private:
	GLuint m_program;
};

