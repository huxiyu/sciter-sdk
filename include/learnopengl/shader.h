#ifndef  SHADER_H
#define SHADER_H

// ����glew��ȡ���е�OpenGL��Ҫheaders
//#include <GL/glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	// ����ID
	GLuint Program;

	Shader(){}

	// ��������ȡ������Shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		// 1. ���ļ�·����� vertex/fragment Դ��
		std::string vertexCode;
		std::string fragmentCode;

		try {
			// ���ļ�
			std::ifstream vShaderFile(vertexPath);
			std::ifstream fShaderFile(fragmentPath);

			std::stringstream vShaderStream, fShaderStream;
			// ��ȡ�ļ����嵽��
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			// �ر��ļ����
			vShaderFile.close();
			fShaderFile.close();

			// ����תΪstring
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch(std::exception e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ"  << std::endl;
		}

		// ��stringתΪGLchar����
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		// 2. ������ɫ��
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];

		// ������ɫ��
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		// ��ӡ����ʱ����
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Ƭ����ɫ��
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		// ��ӡ����ʱ����
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success) 
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// ��ɫ������
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);

		// ��ӡ���Ӵ���
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		// ɾ����ɫ��
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	// ʹ��Program
	void Use() { glUseProgram(this->Program); }
};

#endif