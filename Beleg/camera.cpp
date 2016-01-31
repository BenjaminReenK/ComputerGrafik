#include "camera.hpp"

/**
  * An camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
  */
Camera::Camera(vec3 position, vec3 front, vec3 right, vec3 up, 
			   GLfloat yaw, GLfloat pitch,
			   GLfloat speed, GLfloat sensivity, GLfloat fov) {
	this->Position = position;
	this->Front = front;
	this->Right = right;
	this->Up = up;
    this->Yaw = yaw;
    this->Pitch = pitch;
    this->UpdateCameraVectors();
	this->MovementSpeed = speed;
	this->MouseSensitivity = sensivity;
	this->FoV = fov;
	this->deltaTime = 0.0f;
	this->lastFrame = 0.0f;
}

mat4 Camera::GetViewMatrix() {
	if (IsAnimating) {
		vec3 p = *BezierPointPositions.front();
		delete this->BezierPointPositions.front();
		BezierPointPositions.pop();						
		this->Position = p;
					
		vec3 pf = *BezierPointFronts.front();
		delete this->BezierPointFronts.front();
		BezierPointFronts.pop();
		this->currentFront = pf;
				
		if (BezierPointPositions.empty()){
			IsAnimating = false;
			Pitch = DEFAULT_PITCH;
			Yaw = DEFAULT_YAW;
			UpdateCameraVectors();
		}
		// printf("Bezier: %d %f %f %f\n", this->BezierPoints.size(), p.x, p.y, p.z);
	}
	return lookAt(this->Position, this->Position + this->currentFront, this->currentUp);
}

GLfloat Camera::GetFoV() {
	return this->FoV;
}

/**
  * Processes input received from any keyboard-like input system. 
  * Accepts input parameter in the form of camera defined ENUM
  */
void Camera::ProcessKeyboard(CameraMovement direction) {
	if (IsAnimating) {
		StopAnimating();
		return;
	}
	
	/*
	GLfloat currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	GLfloat velocity = this->MovementSpeed * deltaTime;	
	*/

	GLfloat velocity = this->MovementSpeed;	
	if (direction == FORWARD)
		this->Position += this->currentFront * velocity;
	if (direction == BACKWARD)
		this->Position -= this->currentFront * velocity;
	if (direction == LEFT)
		this->Position -= this->currentRight * velocity;
	if (direction == RIGHT)
		this->Position += this->currentRight * velocity;
	if (direction == UP)
		this->Position += this->currentUp * velocity;
	if (direction == DOWN)
		this->Position -= this->currentUp * velocity;
}

/**
  * Processes input received from a mouse input system. Expects the offset value in both the x and y direction
  */
void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset) {
	xoffset *= this->MouseSensitivity;
	yoffset *= this->MouseSensitivity;

	this->Yaw   += xoffset;
	this->Pitch += yoffset;

	// no bounds 
	//if (this->Pitch > 89.0f) this->Pitch = 89.0f;
	//if (this->Pitch < -89.0f) this->Pitch = -89.0f;
	
	// Update Front, Right and Up Vectors using the updated Eular angles
	this->UpdateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(GLfloat yoffset) {
	if (this->FoV >= 1.0f && this->FoV <= 45.0f)
		this->FoV -= yoffset;
	if (this->FoV <= 1.0f)
		this->FoV = 1.0f;
	if (this->FoV >= 45.0f)
		this->FoV = 45.0f;
}

void Camera::AnimateTo(vec3 target){	
	printf("animate to target");
	StopAnimating();	
	const vec3 StartPosition = Position;
	bezierTarget = target;
	target.y -= 1;
	target.z = -1;	
	vec3 direction = target - StartPosition;
	vec3 perpendicularPoint = StartPosition + direction * 0.3f;
	vec3 perpendicular = vec3(-direction.y, direction.x, 0.0f);
	vec3 helpPoint = perpendicularPoint + 0.7f * perpendicular;

	/*
	printf("StartPos: %f %f %f\n", StartPosition.x, StartPosition.y, StartPosition.z);
	printf("target: %f %f %f\n", target.x, target.y, target.z);
	printf("direction: %f %f %f\n", direction.x, direction.y, direction.z);
	printf("perpendicularPoint: %f %f %f\n", perpendicularPoint.x, perpendicularPoint.y, perpendicularPoint.z);
	printf("perpendicular: %f %f %f\n", perpendicular.x, perpendicular.y, perpendicular.z);
	printf("helpPoint: %f %f %f\n\n", helpPoint.x, helpPoint.y, helpPoint.z);
	*/

	// calc quad. BezierCurve
	for (float i = 0; i < 1; i += 0.0004f) {		
		vec3* p = new vec3();		
		p->x = (1 - i) * (1 - i) * StartPosition.x + 2 * (1 - i) * i * helpPoint.x + i * i * target.x;
		p->y = (1 - i) * (1 - i) * StartPosition.y + 2 * (1 - i) * i * helpPoint.y + i * i * target.y;
		p->z = (1 - i) * (1 - i) * StartPosition.z + 2 * (1 - i) * i * helpPoint.z + i * i * target.z;
		// printf("pp: %f %f %f\n", p->x, p->y, p->z);
		this->BezierPointPositions.push(p);

		vec3* pt = new vec3();
		vec3 t = bezierTarget - *p;
		pt->x = (1 - i) * currentFront.x *.9 + i * t.x *.1;
		pt->y = (1 - i) * currentFront.y *.9 + i * t.y *.1;
		pt->z = (1 - i) * currentFront.z *.9 + i * t.z *.1;
		// printf("pt: %f %f %f\n", pt->x, pt->y, pt->z);
		this->BezierPointFronts.push(pt);		
	}
	IsAnimating = true;
}

void Camera::UpdateCameraVectors() {
	mat4 rotationMatrix = mat4(1.0f);		
	//rotationMatrix = rotate(rotationMatrix, cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch)), glm::vec3(1.0, 0.0, 0.0));		
	//rotationMatrix = rotate(rotationMatrix, sin(glm::radians(this->Pitch)),								 glm::vec3(0.0, 1.0, 0.0));	
	//rotationMatrix = rotate(rotationMatrix, sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch)), glm::vec3(0.0, 0.0, 1.0));

	rotationMatrix = rotate(rotationMatrix, -Pitch, vec3(1.0, 0.0, 0.0));		
	rotationMatrix = rotate(rotationMatrix, -Yaw,   vec3(0.0, 1.0, 0.0));	
	// no roll in z 
	
	vec4 frontTmp = normalize(rotationMatrix * vec4(this->Front.x, this->Front.y, this->Front.z, 0));
	vec4 rightTmp = normalize(rotationMatrix * vec4(this->Right.x, this->Right.y, this->Right.z, 0));
	vec4 upTmp =	normalize(rotationMatrix * vec4(this->Up.x,	   this->Up.y,    this->Up.z,    0));
	
	this->currentFront = vec3(frontTmp.x, frontTmp.y, frontTmp.z);
	this->currentRight = vec3(rightTmp.x, rightTmp.y, rightTmp.z);
	this->currentUp =	 vec3(upTmp.x,	  upTmp.y,	  upTmp.z);
}

void Camera::StopAnimating(){
	IsAnimating = false;
	queue<vec3*>().swap(BezierPointFronts); // clear queue
	queue<vec3*>().swap(BezierPointPositions); // clear queue
}

const GLfloat Camera::DEFAULT_YAW        =  180.0f;
const GLfloat Camera::DEFAULT_PITCH		 =  45.0f;
const GLfloat Camera::DEFAULT_SPEED      =  0.001f;
const GLfloat Camera::DEFAULT_SENSITIVTY =  0.05f;
const GLfloat Camera::DEFAULT_FOV        =  45.0f;