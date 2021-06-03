/*==================================================================================
* COSC 363  Computer Graphics (2021)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf  for details.
*===================================================================================
*/
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "TextureBMP.h"
#include "Plane.h"
using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
TextureBMP texture;


vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(10, 40, -3);	
	glm::vec3 lightPos2(-20, 40, -3);               //Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found


	if (ray.index == 4) { 
		int xCurrent = (int)((ray.hit.x + 250) / 5) % 2; 
		int zCurrent = (int)((ray.hit.z + 250) / 5) % 2;

		if ((xCurrent && zCurrent) || (!xCurrent && !zCurrent)) {
			sceneObjects[4]->setColor(glm::vec3(0, 1, 0));
		}
		else {
			sceneObjects[4]->setColor(glm::vec3(1));
		}
	}

	if (ray.index == 1) {
		float texcoords = asin(obj->normal(ray.hit).x / M_PI + 0.5);
		float texcoordt = asin(obj->normal(ray.hit).y / M_PI + 0.5);
		if (texcoords > 0 && texcoords < 1 &&
			texcoordt > 0 && texcoordt < 1) {
			obj->setColor(texture.getColorAt(texcoords, texcoordt));
		}
	}



	//shadows

	color = obj->lighting(lightPos, -ray.dir, ray.hit);						//Object's colour

	glm::vec3 lightVec = lightPos - ray.hit;
	glm::vec3 lightVec2 = lightPos2 - ray.hit;
	Ray shadowRay(ray.hit, lightVec);	
	Ray shadowRay2(ray.hit, lightVec2);
	shadowRay.closestPt(sceneObjects);
	shadowRay2.closestPt(sceneObjects);

	//if ((shadowRay.index > -1) && (shadowRay.dist < (glm::length(lightVec)))) {
	//	color = 0.2f * color; //0.2 = ambient scale factor
	//}


	if (shadowRay.index > -1 && shadowRay.index == shadowRay2.index) {
		color = 0.1f * color;
		if (shadowRay.index == 2 || shadowRay.index == 3) {
			glm::vec3 shadColor(0);
			shadColor = obj->lighting(lightPos, -ray.dir, ray.hit);
			color = 0.25f * shadColor;
		}
	}
	else if (shadowRay.index > -1) {
		color = 0.2f * color;
		if (shadowRay.index == 2 || shadowRay.index == 3) {
			glm::vec3 shadColor(0);
			shadColor = obj->lighting(lightPos, -ray.dir, ray.hit);
			color = 0.45f * shadColor;
		}
	}
	else if (shadowRay2.index > -1) {
		color = 0.2f * color;
		if (shadowRay2.index == 2 || shadowRay2.index == 3) {
			glm::vec3 shadColor(0);
			shadColor = obj->lighting(lightPos, -ray.dir, ray.hit);
			color = 0.45f * shadColor;
		}
	}



	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}

	if (obj->isTransparent() && step < MAX_STEPS) {
		glm::vec3 g = ray.dir;
		Ray throughRay(ray.hit, g);
		throughRay.closestPt(sceneObjects);
		Ray outRay(throughRay.hit, g);
		glm::vec3 col2 = trace(outRay, step + 1);
		color = color + (obj->getTransparencyCoeff() * col2);
	}

    if (obj->isRefractive() && step < MAX_STEPS) {
        glm::vec3 d = ray.dir;
        glm::vec3 n = obj->normal(ray.hit);
        float eta = 1/1.33;
        Ray throughRay(ray.hit, glm::refract(d, n, eta));
        throughRay.closestPt(sceneObjects);
        n = obj->normal(throughRay.hit);
        d = throughRay.dir;
        Ray outRay(throughRay.hit, glm::refract(d,-n, 1/eta));
        glm::vec3 col2 = trace(outRay, step+1);
        color = color + (obj->getRefractionCoeff() * col2);
    }

	// fog
	//int z1 = -65;
	//int z2 = -215;
	//float t = (ray.hit.z - z1) / (z2 - z1);
	//color = (1 - t) * color + glm::vec3(t, t, t);
	


	return color;
}

void drawPyramid() {
	
	Plane* plane2 = new Plane(glm::vec3(-2, -9, -100),
		glm::vec3(-5, -15, -90),
		glm::vec3(-1, -15, -80));
	plane2->setColor(glm::vec3(1, 0, 0.8));
	sceneObjects.push_back(plane2);

	Plane* plane3 = new Plane(glm::vec3(-2, -9, -100),
		glm::vec3(-1, -15, -80),
		glm::vec3(3, -15, -90));
	plane3->setColor(glm::vec3(0, 0, 0.8));
	sceneObjects.push_back(plane3);

	Plane* plane4 = new Plane(glm::vec3(-2, -9, -100),
		glm::vec3(-5, -15, -90),
		glm::vec3(-1, -15, -100));
	plane4->setColor(glm::vec3(1, 0, 0.8));
	sceneObjects.push_back(plane4);

	Plane* plane5 = new Plane(glm::vec3(-2, -9, -100),
		glm::vec3(-1, -15, -100),
		glm::vec3(3, -15, -90));
	plane5->setColor(glm::vec3(0, 0, 0.8));
	sceneObjects.push_back(plane5);


}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

		    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    Ray ray = Ray(eye, dir);

		    glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

	Sphere* sphere1 = new Sphere(glm::vec3(-7, -6, -120), 8);
	sphere1->setColor(glm::vec3(1, 0, 0));
	sphere1->setReflectivity(true, 0.8);
	sceneObjects.push_back(sphere1);

	Sphere* sphere2 = new Sphere(glm::vec3(10, -11, -100), 4);
	sphere2->setColor(glm::vec3(1, 0, 0));
	sceneObjects.push_back(sphere2);

	Sphere* sphere3 = new Sphere(glm::vec3(10, -12, -80), 3);
	sphere3->setColor(glm::vec3(0, 1, 1));
	sphere3->setTransparency(true, 1.0f);
	sceneObjects.push_back(sphere3);
	
	Sphere* sphere4 = new Sphere(glm::vec3(-13, -12, -80), 3);
	sphere4->setColor(glm::vec3(0, 1, 0));
	sphere4->setRefractivity(true, 0.8f, 1.33f);
	sceneObjects.push_back(sphere4);

	Plane* plane = new Plane(glm::vec3(-20., -15, -40), //Point A
		glm::vec3(20., -15, -40), //Point B
		glm::vec3(20., -15, -200), //Point C
		glm::vec3(-20., -15, -200)); //Point D
	plane->setColor(glm::vec3(0.8, 0.8, 0));
	plane->setSpecularity(false);
	sceneObjects.push_back(plane);

	drawPyramid();

	texture = TextureBMP("Butterfly.bmp");


}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
