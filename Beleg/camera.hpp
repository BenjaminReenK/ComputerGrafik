#ifndef CAMERA_HPP
#define CAMERA_HPP

#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <queue>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

// Defines several possible options for camera movement.
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
	UP,
	DOWN
};

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera {
	
public:
    // Constructor with vectors
	Camera(vec3 position, vec3 front, vec3 right, vec3 up, 
		GLfloat yaw = DEFAULT_YAW, GLfloat pitch = DEFAULT_PITCH,
		GLfloat speed = DEFAULT_SPEED, GLfloat sensivity = DEFAULT_SENSITIVTY, 
		GLfloat fov = DEFAULT_FOV);
    
    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    mat4 GetViewMatrix();

	// return the field of view value
	GLfloat GetFoV();

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(CameraMovement direction);

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset);

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLfloat yoffset);

	void AnimateTo(vec3 target);

private:
	// https://stackoverflow.com/questions/2454019/why-arent-static-const-floats-allowed
	static const GLfloat DEFAULT_YAW;
	static const GLfloat DEFAULT_PITCH;
	static const GLfloat DEFAULT_SPEED;
	static const GLfloat DEFAULT_SENSITIVTY;
	static const GLfloat DEFAULT_FOV;

	// Camera Attributes
	vec3 Position;
	vec3 Front;
	vec3 Right;
	vec3 Up;	
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
	//no roll
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat FoV;
	// currentPos
	vec3 currentFront;
	vec3 currentRight;
	vec3 currentUp;

	// ensure same speed everywhere
	GLfloat deltaTime; // Time between current frame and last frame
	GLfloat lastFrame; // Time of last frame

	// Calculates the front vector from the Camera's (updated) Eular Angles
    void UpdateCameraVectors();

	// support ride to planets
	queue<vec3*> BezierPointPositions;
	queue<vec3*> BezierPointFronts;
	vec3 bezierTarget;
	bool IsAnimating;
	void StopAnimating();
};

#endif