/*
*        Computer Graphics Course - Shenzhen University
*    Week 9 - Phong Reflectance Model (per-fragment shading)
* ============================================================
*
* - ����������ǲο����룬����Ҫ����ο���ҵ˵��������˳������ɡ�
* - ��������OpenGL�����������������У���ο���һ��ʵ��γ�����ĵ���
*/

#include "include/Angel.h"
#include "include/TriMesh.h"

#pragma comment(lib, "glew32.lib")

#include <cstdlib>
#include <iostream>

using namespace std;

GLuint programID;
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint vertexNormalID;
GLuint vertexIndexBuffer;

GLuint vPositionID;
GLuint vNormalID;
GLuint modelViewMatrixID;
GLuint modelViewProjMatrixID;

GLuint lightPosID;

TriMesh* mesh = new TriMesh();
vec3 lightPos(0.0, 0.0, 2.0);

//////////////////////////////////////////////////////////////////////////
// �����������

namespace Camera
{
	mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;

	mat4 ortho( const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar )
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		mat4 c;
		c[0][0] = 2.0 / (right - left);
		c[1][1] = 2.0 / (top - bottom);
		c[2][2] = -2.0 / (zFar - zNear);
		c[3][3] = 1.0;
		c[0][3] = -(right + left) / (right - left);
		c[1][3] = -(top + bottom) / (top - bottom);
		c[2][3] = -(zFar + zNear) / (zFar - zNear);
		return c;
	}

	mat4 perspective( const GLfloat fovy, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar)
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		GLfloat top = tan(fovy * M_PI / 180 / 2) * zNear;
		GLfloat right = top * aspect;

		mat4 c;
		c[0][0] = zNear / right;
		c[1][1] = zNear / top;
		c[2][2] = -(zFar + zNear) / (zFar - zNear);
		c[2][3] = -(2.0*zFar*zNear) / (zFar - zNear);
		c[3][2] = -1.0;
		c[3][3] = 0.0;
		return c;
	}

	mat4 lookAt( const vec4& eye, const vec4& at, const vec4& up )
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		vec4 n = normalize(eye - at);
		vec4 u = normalize(vec4(cross(up, n), 0.0));
		vec4 v = normalize(vec4(cross(n, u), 0.0));

		vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
		mat4 c = mat4(u, v, n, t);
		return c * Translate(-eye);
	}
}

//////////////////////////////////////////////////////////////////////////
// OpenGL ��ʼ��

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	programID = InitShader("vshader_frag.glsl", "fshader_frag.glsl");

	// �Ӷ�����ɫ����ƬԪ��ɫ���л�ȡ������λ��
	vPositionID = glGetAttribLocation(programID, "vPosition");
	vNormalID = glGetAttribLocation(programID, "vNormal");
	modelViewMatrixID = glGetUniformLocation(programID, "modelViewMatrix");
	modelViewProjMatrixID = glGetUniformLocation(programID, "modelViewProjMatrix");
	lightPosID = glGetUniformLocation(programID, "lightPos");

	// ��ȡ�ⲿ��άģ��
	mesh->read_off("sphere.off");

	vector<vec3f> vs = mesh->v();
	vector<vec3i> fs = mesh->f();
	vector<vec3f> ns;

	// TODO ������ģ����ÿ������ķ����������洢��ns������
	for (int i = 0; i < vs.size(); ++i) {
		ns.push_back(vs[i] - vec3(0.0, 0.0, 0.0));
	}

	// ����VAO
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(vec3f), vs.data(), GL_STATIC_DRAW);

	// ����VBO�����󶨷���������
	glGenBuffers(1, &vertexNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalID);
	glBufferData(GL_ARRAY_BUFFER, ns.size() * sizeof(vec3f), ns.data(), GL_STATIC_DRAW);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fs.size() * sizeof(vec3i), fs.data(), GL_STATIC_DRAW);

	// OpenGL��Ӧ״̬����
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

//////////////////////////////////////////////////////////////////////////
// ��Ⱦ

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	
	//Ĭ������
	Camera::modelMatrix=mat4(1.0);
	Camera::viewMatrix=Camera::lookAt(vec4(0,0,1,1),vec4(0,0,0,1),vec4(0,1,0,0));
	Camera::projMatrix=Camera::ortho(-1, 1, -1, 1, -1, 1);
	
	
	// TODO ��������۲�����ͶӰ���󣬲����붥����ɫ��
	// mat4 modelViewMatrix = ...;  //����
	// mat4 modelViewProjMatrix = ...; //����
	mat4 modelViewMatrix = Camera::viewMatrix * Camera::modelMatrix;
	mat4 modelViewProjMatrix = Camera::projMatrix * modelViewMatrix;


	// glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, ...);
	// glUniformMatrix4fv(modelViewProjMatrixID, 1, GL_TRUE, ...);
	glUniformMatrix4fv(modelViewMatrixID, 1, GL_TRUE, &modelViewMatrix[0][0]);
	glUniformMatrix4fv(modelViewProjMatrixID, 1, GL_TRUE, &modelViewProjMatrix[0][0]);


	// ����Դλ�ô��붥����ɫ��
	glUniform3fv(lightPosID, 1, &lightPos[0]);

	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(
		vPositionID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glEnableVertexAttribArray(vNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalID);
	glVertexAttribPointer(
		vNormalID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);

	glDrawElements(
		GL_TRIANGLES,
		int(mesh->f().size() * 3),
		GL_UNSIGNED_INT,
		(void*)0
	);

	glDisableVertexAttribArray(vPositionID);
	glUseProgram(0);

	glutSwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
// �������ô���

void reshape(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

//////////////////////////////////////////////////////////////////////////
void idle(void)
{
	glutPostRedisplay();
}
//////////////////////////////////////////////////////////////////////////
// �����Ӧ����

void mouse(int x, int y)
{
	lightPos[0] = (x - 250) / 250.0 * 10;
	lightPos[1] = (250 - y) / 250.0 * 10;
	glutIdleFunc(idle);
	return;
	// TODO �������ƹ�Դ��λ�ã���ʵʱ���¹���Ч��
}

//////////////////////////////////////////////////////////////////////////
// ������Ӧ����

void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
	case 033:	// ESC�� �� 'q' ���˳���Ϸ
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit (EXIT_SUCCESS);
		break;
	}
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

void clean()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);

	if (mesh) {
		delete mesh;
		mesh = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("OpenGL-Tutorial");

	glewInit();
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMotionFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	clean();

	return 0;
}