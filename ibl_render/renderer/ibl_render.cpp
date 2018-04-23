#include "external/glad/glad.c"
#include "external/GLFW/glfw3.h"
#pragma comment (lib, "external/GLFW/glfw3.lib") 
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstddef>
#include <map>
#include <vector>

#define EXPORT extern "C" __declspec( dllexport )

GLFWwindow* window = nullptr;

unsigned int RBO_color, RBO_depth, FBO = 0;
int FBO_width, FBO_height = 0;

unsigned int basic_shader = 0;

struct Mesh
{
	unsigned int VAO = 0;
	int index_count = 0;
};

std::map<std::string, unsigned int> textures;
std::map<std::string, Mesh> meshes;

unsigned int load_shader(const char* dir, const char* vs_path, const char* fs_path)
{
	auto compile_shader = [&](const char* path, GLenum shader_type)
	{
		unsigned int shader = glCreateShader(shader_type);

		try
		{
			std::ifstream file;
			file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			file.open(std::string(dir) + std::string(path));

			std::string std_str = std::string(std::istreambuf_iterator<char>(file),
				std::istreambuf_iterator<char>());
			const char* str = std_str.c_str();

			file.close();

			glShaderSource(shader, 1, &str, NULL);
			glCompileShader(shader);
			// Check for errors
			int success;
			char info_log[1024];
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, info_log);
				std::cout << info_log << std::endl;
			}

		}
		catch (std::ifstream::failure e)
		{
			std::cout << path << " couldn't be read." << std::endl;
		}
			
		return shader;
	};

	unsigned int vertex_shader = compile_shader(vs_path, GL_VERTEX_SHADER);
	unsigned int fragment_shader = compile_shader(fs_path, GL_FRAGMENT_SHADER);

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	// Check for errors
	int success;
	char info_log[1024];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 1024, NULL, info_log);
		std::cout << info_log << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return program;
}


void draw_cube()
{
	static unsigned int VAO = 0;
	static unsigned int VBO = 0;
    if (VAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(VAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

unsigned int panorama_to_cubemap_shader = 0;
unsigned int background_shader = 0;

unsigned int hdr_texture = 0;
unsigned int environment_cubemap = 0;

EXPORT void load_hdri(unsigned char* data, int width, int height, int components)
{
	int resolution = 512;

	//Setup FrameBuffers
	unsigned int capture_FBO;
	unsigned int capture_RBO;
	glGenFramebuffers(1, &capture_FBO);
	glGenRenderbuffers(1, &capture_RBO);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_RBO);

	GLenum format = components == 3 ? GL_RGB : GL_RGBA;
	//Load HDRI
	glGenTextures(1, &hdr_texture);
	glBindTexture(GL_TEXTURE_2D, hdr_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, format, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Setup cubemap
	glGenTextures(1, &environment_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
	for (unsigned int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 
			GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 views[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

	glUseProgram(panorama_to_cubemap_shader);
	glUniform1i(glGetUniformLocation(panorama_to_cubemap_shader, "panorama"), 0);

	glUniformMatrix4fv(glGetUniformLocation(
		panorama_to_cubemap_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdr_texture);

	glViewport(0, 0, resolution, resolution);
	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);
	for (unsigned int i = 0; i < 6; i++)
	{
		glUniformMatrix4fv(glGetUniformLocation(
			panorama_to_cubemap_shader, "view"), 1, GL_FALSE, glm::value_ptr(views[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment_cubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw_cube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

EXPORT void load_texture(const char* name, unsigned char* data, int width, int height, int components)
{
	if (textures[name]) return;

	unsigned int texture;
	glGenTextures(1, &texture);

	GLenum format;
	if (components == 1)
		format = GL_RED;
	else if (components == 3)
		format = GL_RGB;
	else if (components == 4)
		format = GL_RGBA;

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	textures[name] = texture;
}

struct vertex
{
	float position[3];
	float normal[3];
	float uv[2];
};

EXPORT void load_mesh(const char* name, vertex* vertices, int vertex_count, unsigned int* indices, int index_count)
{
	if (meshes[name].VAO) return;

	unsigned int VAO;
	unsigned int VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vertex), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));

	glBindVertexArray(0);

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	meshes[name] = { VAO, index_count };
}

void glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	void *userParam)
{
	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

void glfwErrorOutput(int error, const char* message)
{
	std::cout << "---------------" << std::endl;
	std::cout << "GLFW ERROR :" << error << std::endl << message << std::endl;
}

EXPORT void initialize(const char* path, int width, int height, int msaa)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, msaa);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	glfwSetErrorCallback(glfwErrorOutput);

	window = glfwCreateWindow(width, height, "IBLRender", NULL, NULL);
	glfwMakeContextCurrent(window);

	//glfwSetWindowSizeLimits(window, 0, 0, 16384, 16384);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	std::cout << glGetString(GL_VERSION) << std::endl;
	
	//glEnable(GL_DEBUG_OUTPUT);
	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//glDebugMessageCallback((GLDEBUGPROC)glDebugOutput, nullptr);

	basic_shader = load_shader(path, "basic.vs", "basic.fs");
	panorama_to_cubemap_shader = load_shader(path, "cubemap.vs", "panorama_to_cubemap.fs");
	background_shader = load_shader(path, "background.vs", "background.fs");
	glUniform1i(glGetUniformLocation(background_shader, "cubemap"), 0);
}

EXPORT void resize(int width, int height, int msaa)
{
	//TODO: MSAA

	glDeleteFramebuffers(1, &RBO_color);
	glDeleteFramebuffers(1, &RBO_depth);
	glDeleteFramebuffers(1, &FBO);

	glGenRenderbuffers(1, &RBO_color);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO_color);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glGenRenderbuffers(1, &RBO_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER, RBO_color);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, RBO_depth);
	glViewport(0, 0, width, height);

	FBO_width = width;
	FBO_height = height;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

EXPORT void render_begin(float camera_position[3], float view_matrix[16], float projection_matrix[16], float background_color[3])
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, FBO_width, FBO_height);

	glClearColor(background_color[0], background_color[1], background_color[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(basic_shader);
	glUniform3fv(glGetUniformLocation(basic_shader, "camPos"), 1, camera_position);
	glUniformMatrix4fv(glGetUniformLocation(basic_shader, "projection"), 1, GL_FALSE, projection_matrix);
	glUniformMatrix4fv(glGetUniformLocation(basic_shader, "view"), 1, GL_FALSE, view_matrix);

	glUseProgram(background_shader);
	glUniformMatrix4fv(glGetUniformLocation(background_shader, "projection"), 1, GL_FALSE, projection_matrix);
	glUniformMatrix4fv(glGetUniformLocation(background_shader, "view"), 1, GL_FALSE, view_matrix);
}

EXPORT void draw_mesh(float transform[16], const char* mesh, float color[3])//,
	//const char* albedo, const char* normal, const char* metallic, const char* roughness, const char* ao)
{
	glUseProgram(basic_shader);
	glUniformMatrix4fv(glGetUniformLocation(basic_shader, "model"), 1, GL_FALSE, transform);
	glUniform3fv(glGetUniformLocation(basic_shader, "color"), 1, color);

	glBindVertexArray(meshes[mesh].VAO);
	glDrawElements(GL_TRIANGLES, meshes[mesh].index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

std::vector<float> pixels = {};

EXPORT float* render_end()
{
	glUseProgram(background_shader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
	draw_cube();

	pixels.reserve(FBO_width * FBO_height * 3);

	glReadPixels(0, 0, FBO_width, FBO_height, GL_RGB, GL_FLOAT, pixels.data());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return pixels.data();
}

EXPORT void clean_resources()
{
	glfwMakeContextCurrent(window);

	for (auto& texture : textures)
	{
		glDeleteTextures(1, &texture.second);
	}
	textures.clear();

	for (auto& mesh : meshes)
	{
		glDeleteVertexArrays(1, &mesh.second.VAO);
	}
	meshes.clear();
}

EXPORT void terminate_context()
{
	glfwMakeContextCurrent(window);
	glfwTerminate();
}
