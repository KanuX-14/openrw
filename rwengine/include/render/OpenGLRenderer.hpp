#pragma once
#ifndef _OPENGLRENDERER_HPP_
#define _OPENGLRENDERER_HPP_

#include <engine/RWTypes.hpp>
#include <render/DrawBuffer.hpp>
#include <render/GeometryBuffer.hpp>

// Maximum depth of debug group stack
#define MAX_DEBUG_DEPTH 5

typedef std::uint32_t RenderIndex;

struct VertexP3
{
	glm::vec3 position;

	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 3, sizeof(VertexP3),  0ul},
		};
	}
};

/// @todo normalize this to have the same interface as VertexP3
struct VertexP2 {
	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 2, sizeof(VertexP2),  0ul}
		};
	}

	float x, y;
};

class Renderer
{
public:

	typedef std::vector<GLuint> Textures;

	struct DrawParameters
	{
		/// Textures to bind to each texture unit
		Textures textures;
		glm::u8vec4 colour;
		float ambient;
		float diffuse;

		size_t count; /// The number of indicies to draw
		unsigned int start; /// Start index.
	};

	struct ObjectUniformData {
		glm::mat4 model;
		glm::vec4 colour;
		float diffuse;
		float ambient;
		float visibility;
	};

	struct SceneUniformData {
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec4 ambient;
		glm::vec4 dynamic;
		glm::vec4 fogColour;
		glm::vec4 campos;
		float fogStart;
		float fogEnd;
	};

	class ShaderProgram {
		// This just provides an opaque handle for external users.
	};

	virtual std::string getIDString() const = 0;

	virtual ShaderProgram* createShader(const std::string& vert, const std::string& frag) = 0;

	virtual void useProgram(ShaderProgram* p) = 0;

	/// @todo dont use GLint in the interface.
	virtual void setProgramBlockBinding(ShaderProgram* p, const std::string& name, GLint point) = 0;
	virtual void setUniformTexture(ShaderProgram*p, const std::string& name, GLint tex) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::mat4& m) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::vec4& v) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::vec3& v) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, const glm::vec2& v) = 0;
	virtual void setUniform(ShaderProgram*p, const std::string& name, float f) = 0;

	virtual void clear(const glm::vec4& colour, bool clearColour = true, bool clearDepth = true) = 0;

	virtual void setSceneParameters(const SceneUniformData& data) = 0;

	virtual void draw(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p) = 0;
	virtual void drawArrays(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p) = 0;

	void setViewport(const glm::ivec2& vp);
	const glm::ivec2& getViewport() const { return viewport; }

	const glm::mat4& get2DProjection() const { return projection2D; }

	virtual void invalidate() = 0;

	/**
	 * Resets all per-frame counters.
	 */
	void swap();
	
	/**
	 * Returns the number of draw calls issued for the current frame.
	 */
	int getDrawCount();
	int getTextureCount();
	int getBufferCount();
	
	const SceneUniformData& getSceneData() const;

	/**
	 * Profiling data returned by popDebugGroup.
	 * Not all fields will be populated, depending on
	 * USING(RENDER_PROFILER)
	 */
	struct ProfileInfo
	{
		GLuint64 timerStart;
		GLuint64 duration;
		unsigned int primitives;
		unsigned int draws;
		unsigned int textures;
		unsigned int buffers;
		unsigned int uploads;
	};

	/**
	 * Signals the start of a debug group
	 */
	virtual void pushDebugGroup(const std::string& title) = 0;
	/**
	 * Ends the current debug group and returns the profiling information
	 * for that group. The returned value is valid until the next call to
	 * pushDebugGroup
	 */
	virtual const ProfileInfo& popDebugGroup() = 0;

private:
	glm::ivec2 viewport;
	glm::mat4 projection2D;
protected:
	int drawCounter;
	int textureCounter;
	int bufferCounter;
	SceneUniformData lastSceneData;
};

class OpenGLRenderer : public Renderer
{
public:

	class OpenGLShaderProgram : public ShaderProgram {
		GLuint program;
		std::map<std::string, GLint> uniforms;
	public:
		OpenGLShaderProgram(GLuint p)
			: program(p)
		{ }

		GLuint getName() const { return program; }

		GLint getUniformLocation(const std::string& name) {
			auto c = uniforms.find(name.c_str());
			GLint loc = -1;
			if( c == uniforms.end() ) {
				loc = glGetUniformLocation(program, name.c_str());
				uniforms[name] = loc;
			}
			else {
				loc = c->second;
			}
			return loc;
		}
	};

	OpenGLRenderer();

	std::string getIDString() const;

	ShaderProgram* createShader(const std::string &vert, const std::string &frag);
	void setProgramBlockBinding(ShaderProgram* p, const std::string &name, GLint point);
	void setUniformTexture(ShaderProgram* p, const std::string &name, GLint tex);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::mat4& m);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::vec4& m);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::vec3& m);
	void setUniform(ShaderProgram* p, const std::string& name, const glm::vec2& m);
	void setUniform(ShaderProgram* p, const std::string& name, float f);
	void useProgram(ShaderProgram* p);

	void clear(const glm::vec4 &colour, bool clearColour, bool clearDepth);

	void setSceneParameters(const SceneUniformData &data);

	void draw(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p);
	void drawArrays(const glm::mat4& model, DrawBuffer* draw, const DrawParameters& p);

	void invalidate();

	virtual void pushDebugGroup(const std::string& title);
	virtual const ProfileInfo& popDebugGroup();

private:
	DrawBuffer* currentDbuff;

	void useDrawBuffer(DrawBuffer* dbuff);

	std::map<GLuint,GLuint> currentTextures;
	void useTexture(GLuint unit, GLuint tex);

	OpenGLShaderProgram* currentProgram;

	GLuint currentUBO;
	template<class T> void uploadUBO(GLuint buffer, const T& data)
	{
		if( currentUBO != buffer ) {
			glBindBuffer(GL_UNIFORM_BUFFER, buffer);
			currentUBO = buffer;
		}
		glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &data, GL_DYNAMIC_DRAW);
#if RW_USING(RENDER_PROFILER)
		if( currentDebugDepth > 0 )
		{
			profileInfo[currentDebugDepth-1].uploads++;
		}
#endif
	}

	GLuint UBOObject;
	GLuint UBOScene;

	// Debug group profiling timers
	ProfileInfo profileInfo[MAX_DEBUG_DEPTH];
	GLuint debugQuery;
	int currentDebugDepth;
};

/// @todo remove these from here
GLuint compileShader(GLenum type, const char *source);
GLuint compileProgram(const char* vertex, const char* fragment);

#endif
