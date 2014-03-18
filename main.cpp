//-------------------------------------------------------------------------------------------------
// Descriere: fisier main
//
// Autor: student
// Data: today
//-------------------------------------------------------------------------------------------------

//incarcator de meshe
#include "lab_mesh_loader.hpp"

//geometrie: drawSolidCube, drawWireTeapot...
#include "lab_geometry.hpp"

//incarcator de shadere
#include "lab_shader_loader.hpp"

//interfata cu glut, ne ofera fereastra, input, context opengl
#include "lab_glut.hpp"

//camera
#include "lab_camera.hpp"

//time
#include <ctime>

#include <vector>
#include "Stadium.h"
#include "Enemy.h"
#include "Flag.h"
#include "TpsObject.h"
using namespace std;

#define PI 3.141592653589793238462
#define NUM_ENEMIES 2


class Laborator : public lab::glut::WindowListener{

//variabile
private:
	glm::mat4 model_matrix, projection_matrix;											//matrici 4x4 pt modelare vizualizare proiectie
	unsigned int gl_program_shader;														//id-ul de opengl al obiectului de tip program shader
	unsigned int gl_flag_shader;
	unsigned int gl_car_shader;
	unsigned int screen_width, screen_height;
	unsigned int mesh_vbo, mesh_ibo, mesh_vao, mesh_num_indices;						//containere opengl pentru vertecsi, indecsi si stare
	unsigned int stadium_vbo[4], stadium_ibo[4], stadium_vao[4], stadium_num_indices[4];
	int stadiumNoOfLayers;

	unsigned int location_model_matrix, location_view_matrix, location_projection_matrix;//locatii la nivel de shader ale matricilor (un pointer/offset)
	unsigned int location_object_color;													//locatie uniform culoare obiect
	
	bool keyStates[256];
	bool keySpecialStates[256];

	unsigned int car2_vbo, car2_ibo, car2_vao, car2_num_indices;
	float car2_alpha, car2_betha, car2_radius, car2_x, car2_y, car2_z, car_lap_count;
	int car_lap;
	int end_game, win_color;

	float carPos[3];
	float carAngle;
	float carSpeed;

	/*
		cam[0] = TPS de sus
		cam[1] = FPS player
		cam[2] = TPS player
		cam[3-4] = FPS inamici
	*/
	lab::Camera cam[5];
	unsigned int activeCam;

	Stadium stadium[4];
	Enemy enemies[NUM_ENEMIES];

	Flag flag;
	float time;

	// pt culori
	glm::vec3 light_position;
	glm::vec3 eye_position;
	unsigned int material_shininess;
	float material_kd;
	float material_ks;

	//pt obiect tps player
	TpsObject tps_obj;


//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
	Laborator(){
		//setari pentru desenare, clear color seteaza culoarea de clear pentru ecran (format R,G,B,A)
		glClearColor(0.5,0.5,0.5,1);
		glClearDepth(1);			//clear depth si depth test (nu le studiem momentan, dar avem nevoie de ele!)
		glEnable(GL_DEPTH_TEST);	//sunt folosite pentru a determina obiectele cele mai apropiate de camera (la curs: algoritmul pictorului, algoritmul zbuffer)
		
		//incarca un shader din fisiere si gaseste locatiile matricilor relativ la programul creat
		gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");
		gl_flag_shader = lab::loadShader("shadere\\flag_shader_vertex.glsl", "shadere\\flag_shader_fragment.glsl");
		gl_car_shader = lab::loadShader("shadere\\car_shader_vertex.glsl", "shadere\\car_shader_fragment.glsl");
		
		//incarca un mesh pentru Player
		lab::loadObj("resurse\\natla car.obj",mesh_vao, mesh_vbo, mesh_ibo, mesh_num_indices);
		lab::loadObj("resurse\\cameraTV.obj",tps_obj.vao, tps_obj.vbo, tps_obj.ibo, tps_obj.num_indices);

		// incarca meshe pentru Enemies:
		for (unsigned i = 0; i < NUM_ENEMIES; i++) {
			lab::loadObj("resurse\\natla car.obj",enemies[i].vao, enemies[i].vbo, enemies[i].ibo, enemies[i].num_indices);
		}
		createMeshManual();
		createFlag();


		//matrici de modelare si vizualizare
		model_matrix = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);

		// camera TPS de sus
		cam[0].set(glm::vec3(0,120,1), glm::vec3(0,0,0), glm::vec3(0,1,0));
		// camera FPS a player-ului
		cam[1].set(glm::vec3(14,5,10), glm::vec3(14,0,0), glm::vec3(0,1,0));
		activeCam = 1;

		//desenare wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(1);
		glPointSize(1);

		// init keyStates:
		for (int i = 0; i < 256; i++) {
			keyStates[i] = false;
			keySpecialStates[i] = false;
		}

		time = 0;
		end_game = 0;

		// citeste vitezele din fisier
		ifstream fin;
		fin.open("speed.txt");
		float a,b,c;
		fin >> carSpeed;
		fin >> enemies[0].speed;
		fin >> enemies[1].speed;
		fin.close();
	}

	//destructor .. e apelat cand e distrusa clasa
	~Laborator(){
		//distruge shader
		glDeleteProgram(gl_program_shader);

		//distruge mesh incarcat
		glDeleteBuffers(1,&mesh_vbo);
		glDeleteBuffers(1,&mesh_ibo);
		glDeleteVertexArrays(1,&mesh_vao);
	}

	void createFlag() {
		int n = 10;
		flag = Flag(n, n);
		vector<VerticesAndColor> vertices;
		vector <glm::uvec3> indices;

		vertices = flag.vertices;
		indices = flag.indices;

		for (int i = 0; i < vertices.size(); i++) {
			cout << "(" << vertices[i].pos.x << " " << vertices[i].pos.y << ") ";
			if (i % n == n-1)
				cout << endl;
		}

		// creeare VAO
		glDeleteVertexArrays(1, &flag.vao);
		glGenVertexArrays(1, &flag.vao);
		// bind VAO
		glBindVertexArray(flag.vao);

		// creez VBO
		glDeleteBuffers(1, &flag.vbo);
		glGenBuffers(1, &flag.vbo);
		// bind VBO
		glBindBuffer(GL_ARRAY_BUFFER, flag.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesAndColor)*vertices.size(), &vertices[0], GL_STATIC_DRAW);


		// creez IBO
		glDeleteBuffers(1, &flag.ibo);
		glGenBuffers(1, &flag.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flag.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(glm::uvec3)*indices.size(),&indices[0], GL_STATIC_DRAW);

		// legare atribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VerticesAndColor),(void*)0);
	
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VerticesAndColor),(void*)sizeof(glm::uvec3));

		flag.num_indices = indices.size() * 3;

	}

	void createMeshManual(){

		stadiumNoOfLayers = 4;
		
		vector<VerticesAndColor> vertices[4];
		vector <glm::uvec3> indices[4];
		float length = 50;
		float width = 18;
		float width2 = 10;
		stadium[0] = Stadium(width, 0, length, glm::vec3(0,0.5,0));
		stadium[1] = Stadium(width + width2, -0.25, length, glm::vec3(0.5,0,0));
		stadium[2] = Stadium(width + width2 * 2, -0.5, length, glm::vec3(0.5,0.5,0));
		stadium[3] = Stadium(width + width2 * 3, -0.75, length, glm::vec3(0.5,0,0));

		for (int i = 0; i < stadiumNoOfLayers; i++) {
			
			vertices[i] = stadium[i].getVertices();
			indices[i] = stadium[i].getIndices();

			// creeare VAO
			glDeleteVertexArrays(1, &stadium_vao[i]);
			glGenVertexArrays(1, &stadium_vao[i]);
			// bind VAO
			glBindVertexArray(stadium_vao[i]);

			// creez VBO
			glDeleteBuffers(1, &stadium_vbo[i]);
			glGenBuffers(1, &stadium_vbo[i]);
			// bind VBO
			glBindBuffer(GL_ARRAY_BUFFER, stadium_vbo[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesAndColor)*vertices[i].size(), &vertices[i][0], GL_STATIC_DRAW);


			// creez IBO
			glDeleteBuffers(1, &stadium_ibo[i]);
			glGenBuffers(1, &stadium_ibo[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stadium_ibo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(glm::uvec3)*indices[i].size(),&indices[i][0], GL_STATIC_DRAW);

			// legare atribute
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VerticesAndColor),(void*)0);
	
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VerticesAndColor),(void*)sizeof(glm::uvec3));

			stadium_num_indices[i] = indices[i].size() * 3;
			

			// init masini in functie de stadion
			// car2:
			for (int i = 0; i < NUM_ENEMIES; i++) {
				enemies[i].x = stadium[i].getWidth() + (stadium[i+1].getWidth() - stadium[i].getWidth()) / 2;			
				enemies[i].y = enemies[i].z = 0;
				enemies[i].radius = enemies[i].x;
				enemies[i].betha = 0;
				enemies[i].alpha = 270;
				enemies[i].speed = 0.1;
			}

			carPos[0] = stadium[2].getWidth() + (stadium[3].getWidth() - stadium[2].getWidth()) / 2;
			carPos[1] = 0;
			carPos[2] = 0;
			carAngle = 270;
			car_lap = -1;
		}

	}

	
	//--------------------------------------------------------------------------------------------
	//functii de cadru ---------------------------------------------------------------------------

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei ( modelam/simulam scena)
	void notifyBeginFrame(){

		// calculez daca s-a facut un tur / lap:
		// player:
		if (end_game == 0) {
			if (carPos[2] <= 0 &&
				carPos[2] >= -0.5 &&
				carPos[0] >= stadium[0].getWidth()) {
				car_lap_count = 1;
			}
			if (carPos[2] <= -1 &&
				carPos[0] >= stadium[0].getWidth() &&
				car_lap_count == 1) {
				car_lap_count = 0;
				car_lap ++;
				if (car_lap == 1) {
					end_game = 1;
					win_color = 2;
				}
			}
			// inamici:
			for (int i = 0; i < NUM_ENEMIES; i++) {
				if (enemies[i].z <= 0 && enemies[i].z >= -0.5 && enemies[i].x >= stadium[0].getWidth()) {
					enemies[i].lap_count = 1;
				}
				if (enemies[i].z <= -1 && enemies[i].x >= stadium[0].getWidth() &&
					enemies[i].lap_count == 1) {

					enemies[i].lap_count = 0;
					enemies[i].lap ++;
					if (enemies[i].lap == 1) {
						end_game = 1;
						win_color = i;
					}
				}
			}
		}

		// modific pozitia player-ului in functie de tastele apasate
		if (keySpecialStates[GLUT_KEY_UP]) {
			float newX, newZ;
			newX = carPos[0];
			newZ = carPos[2];

			newZ += carSpeed * sin((carAngle * PI) / 180);
			newX += carSpeed * cos((carAngle * PI) / 180);

			isInBounds(newX, newZ);
		}
		if (keySpecialStates[GLUT_KEY_DOWN]) {
			float newX, newZ;
			newX = carPos[0];
			newZ = carPos[2];

			newZ -= carSpeed * sin((carAngle * PI) / 180);
			newX -= carSpeed * cos((carAngle * PI) / 180);
			isInBounds(newX, newZ);

		}
		if (keySpecialStates[GLUT_KEY_LEFT]) {
			carAngle -= 2;
		}
		if (keySpecialStates[GLUT_KEY_RIGHT]) {
			carAngle += 2;
		}
		// modificare unghi camera in functie de miscarea palyer-ului
		float car_cam_angle = 360 - carAngle;
		car_cam_angle = (car_cam_angle * PI) / 180;
		cam[1].set(glm::vec3(carPos[0], carPos[1] + 4, carPos[2]),
					glm::vec3(carPos[0] + 8*cos(car_cam_angle), carPos[1] + 4, carPos[2] - 8*sin(car_cam_angle)),
					glm::vec3(0,1,0));
		// Animatie pentru inamici:
		for (int i = 0; i < NUM_ENEMIES; i++) {
			float maxZ, minZ;
			maxZ = stadium[0].getLength();
			minZ = -stadium[0].getLength();
			// merge pe dreapta, din jos in sus
			if (enemies[i].z < maxZ && enemies[i].z > minZ && enemies[i].alpha == 270) {
				enemies[i].z += enemies[i].speed * sin((enemies[i].alpha * PI) / 180);

				// calcul pozitie camera FPS
				float camX, camY, camZ;
				camX = enemies[i].x;
				camY = 4;
				camZ = enemies[i].z + 8 * sin((enemies[i].alpha * PI) / 180);
				cam[i+3].set(glm::vec3(enemies[i].x, camY, enemies[i].z), glm::vec3(camX, camY, camZ),
							 glm::vec3(0, 1, 0));
				
			}

			// a ajuns pe semicercul de sus
			if (enemies[i].z <= minZ) {
				enemies[i].z = enemies[i].radius * sin((enemies[i].betha * PI) / 180) - maxZ;
				enemies[i].x = enemies[i].radius * cos((enemies[i].betha * PI) / 180);
				enemies[i].betha -= enemies[i].speed * 2;

				// calcul pozitie camera FPS
				float camX, camY, camZ;
				camX = enemies[i].x + 8 * cos(((enemies[i].betha - 90) * PI) / 180);
				camY = 4;
				camZ = enemies[i].z + 8 * sin(((enemies[i].betha - 90) * PI) / 180);
				cam[i+3].set(glm::vec3(enemies[i].x, camY, enemies[i].z), glm::vec3(camX, camY, camZ), 
				glm::vec3(0, 1, 0));

				if (enemies[i].betha <= -180) {
					enemies[i].alpha = 90;
					enemies[i].betha = 0;
					enemies[i].z = minZ;
				}
			}

			// merge pe stanga de sus in jos
			if (enemies[i].z <= maxZ && enemies[i].z >= minZ && enemies[i].alpha == 90) {
				enemies[i].z += enemies[i].speed * sin((enemies[i].alpha * PI) / 180);

				// calcul pozitie camera FPS
				float camX, camY, camZ;
				camX = enemies[i].x;
				camY = 4;
				camZ = enemies[i].z + 8 * sin((enemies[i].alpha * PI) / 180);
				cam[i+3].set(glm::vec3(enemies[i].x, camY, enemies[i].z), glm::vec3(camX, camY, camZ),
							 glm::vec3(0, 1, 0));

				if (enemies[i].z > maxZ) {
					enemies[i].betha = 180;
					enemies[i].alpha = 270;
					enemies[i].z = maxZ + 1;
				}
			}

			// e pe semicercul de jos
			if (enemies[i].z >= maxZ) {
				enemies[i].z = enemies[i].radius * sin((enemies[i].betha * PI) / 180) + maxZ;
				enemies[i].x = enemies[i].radius * cos((enemies[i].betha * PI) / 180);
				enemies[i].betha -= enemies[i].speed * 2;

				// calcul pozitie camera FPS
				float camX, camY, camZ;
				camX = enemies[i].x + 8 * cos(((enemies[i].betha - 90) * PI) / 180);
				camY = 4;
				camZ = enemies[i].z + 8 * sin(((enemies[i].betha - 90) * PI) / 180);
				cam[i+3].set(glm::vec3(enemies[i].x, camY, enemies[i].z), glm::vec3(camX, camY, camZ),
							 glm::vec3(0, 1, 0));
			}
		}
		// sfarsit animatie inamici

		// calcul pozitie camera TPS a player-ului
		float camX, camY, camZ;
		float dist = 20;
		float dist_obj = 0;
		camY = 10;
		tps_obj.y = 9;
		float zMin = -stadium[3].getLength();
		float zMax = -zMin;
		// daca e pe jumatatea din dreapta a terenului
		if (carPos[0] >= 0) {
			
			// daca e pe portiunea dreapta a terenului
			if (carPos[2] <= stadium[3].getLength() && carPos[2] >= -stadium[3].getLength()) {
				camX = stadium[3].getWidth() + dist;
				camZ = carPos[2];

				cam[2].set(glm::vec3(camX, camY, camZ), glm::vec3(0,0,camZ), glm::vec3(0,1,0));
				tps_obj.x = camX - dist_obj;
				tps_obj.z = camZ;
				tps_obj.alpha = 0;
			}
			
			if (carPos[0] != 0) {
				// daca e pe semicercul de sus:
				if (carPos[2] < zMin) {
					float cam_angle, co, ca;
					co = zMin - carPos[2];
					ca = carPos[0];
					cam_angle = atan2(co, ca);
										
					float camRadius = stadium[3].getWidth() + dist;
					camX = camRadius * cos(-cam_angle);
					camZ = zMin + camRadius * sin(-cam_angle);
					cam[2].set(glm::vec3(camX, camY, camZ), glm::vec3(0,0,zMin), glm::vec3(0,1,0));
					tps_obj.x = (camRadius  - dist_obj) * cos(-cam_angle);
					tps_obj.z = zMin + (camRadius  - dist_obj) * sin(-cam_angle);
					tps_obj.alpha = (-cam_angle * 180) / PI;

				}
				if (carPos[2] > zMax) {
					float cam_angle, co, ca;
					co = carPos[2] - zMax;
					ca = carPos[0];
					cam_angle = 2*PI - atan(co / ca);
					
					float camRadius = stadium[3].getWidth() + dist;
					camX = camRadius * cos(-cam_angle);
					camZ = zMax + camRadius * sin(-cam_angle);
					cam[2].set(glm::vec3(camX, camY, camZ), glm::vec3(0,0,zMax), glm::vec3(0,1,0));
					tps_obj.x = (camRadius  - dist_obj) * cos(-cam_angle);
					tps_obj.z = zMax + (camRadius  - dist_obj) * sin(-cam_angle);
					tps_obj.alpha = (-cam_angle * 180) / PI;
				}

			}
		} else {
			// daca e pe jumatatea din stanga a terenului

			// daca e pe portiunea dreapta a terenului
			if (carPos[2] <= stadium[3].getLength() && carPos[2] >= -stadium[3].getLength()) {
				camX = -stadium[3].getWidth() - dist;
				camZ = carPos[2];
				cam[2].set(glm::vec3(camX, camY, camZ), glm::vec3(0,0,camZ), glm::vec3(0,1,0));
				tps_obj.x = camX + dist_obj;
				tps_obj.z = camZ;
				tps_obj.alpha = 180;
			}
			if (carPos[0] != 0) {
				// daca e pe semicercul de sus:
				if (carPos[2] < zMin) {
					float cam_angle, co, ca;
					co = zMin - carPos[2];
					ca = -carPos[0];
					cam_angle = atan2(co, ca) + PI;
										
					float camRadius = stadium[3].getWidth() + dist;
					camX = camRadius * cos(cam_angle);
					camZ = zMin + camRadius * sin(cam_angle);
					cam[2].set(glm::vec3(camX, camY, camZ), glm::vec3(0,0,zMin), glm::vec3(0,1,0));
					tps_obj.x = (camRadius - dist_obj) * cos(cam_angle);
					tps_obj.z = zMin + (camRadius - dist_obj) * sin(cam_angle);
					tps_obj.alpha = (cam_angle * 180) / PI;
				}
				// daca e pe semicercul de jos
				if (carPos[2] > zMax) {
					float cam_angle, co, ca;
					co = carPos[2] - zMax;
					ca = -carPos[0];
					cam_angle = PI - atan(co / ca);
					
					float camRadius = stadium[3].getWidth() + dist;
					camX = camRadius * cos(cam_angle);
					camZ = zMax + camRadius * sin(cam_angle);
					cam[2].set(glm::vec3(camX, camY, camZ), glm::vec3(0,0,zMax), glm::vec3(0,1,0));
					tps_obj.x = (camRadius - dist_obj) * cos(cam_angle);
					tps_obj.z = zMax + (camRadius - dist_obj) * sin(cam_angle);
					tps_obj.alpha = (cam_angle * 180) / PI;
				}
			}			
		}
		// sf calcul poz TPS player

		
		
	}
	//functia de afisare (lucram cu banda grafica)
	void notifyDisplayFrame(){
		
		//pe tot ecranul
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 car_model_matrix; // = model_matrix;
		glm::mat4 car2_model_matrix;
		glm::mat4 obj_model_matrix;
		float sc = 0.2;

		

		//cadru normal
		{
			glViewport(0,0, screen_width, screen_height);
	
			if (end_game == 0) {
				//foloseste shaderul
				glUseProgram(gl_program_shader);
				
				//trimite variabile uniforme la shader
				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"model_matrix"),1,false,glm::value_ptr(model_matrix));
				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"view_matrix"),1,false,glm::value_ptr(cam[activeCam].getViewMatrix()));
				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"projection_matrix"),1,false,glm::value_ptr(projection_matrix));
		
				// afisare Stadium:
				for (int i = 0; i < stadiumNoOfLayers; i++) {
					glBindVertexArray(stadium_vao[i]);
					glDrawElements(GL_TRIANGLES, stadium_num_indices[i], GL_UNSIGNED_INT, 0);
				}

				// afisare obiect tps
				obj_model_matrix = glm::mat4();
				float angle = 90;
				obj_model_matrix = glm::translate(obj_model_matrix, glm::vec3(tps_obj.x, tps_obj.y, tps_obj.z));
				
				obj_model_matrix = glm::rotate(obj_model_matrix, -angle, glm::vec3(0,0,1));
				obj_model_matrix = glm::rotate(obj_model_matrix, angle, glm::vec3(0,1,0));
				obj_model_matrix = glm::rotate(obj_model_matrix, tps_obj.alpha, glm::vec3(0,0,1));
				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"model_matrix"),1,false,glm::value_ptr(obj_model_matrix));
				glBindVertexArray(tps_obj.vao);
				glDrawElements(GL_TRIANGLES, tps_obj.num_indices, GL_UNSIGNED_INT, 0);

	
				// afisare masina:
				glUseProgram(gl_car_shader);
				glUniformMatrix4fv(glGetUniformLocation(gl_car_shader,"view_matrix"),1,false,glm::value_ptr(cam[activeCam].getViewMatrix()));
				glUniformMatrix4fv(glGetUniformLocation(gl_car_shader,"projection_matrix"),1,false,glm::value_ptr(projection_matrix));
				
				car_model_matrix = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
			
				car_model_matrix = glm::translate(car_model_matrix, glm::vec3(carPos[0], carPos[1], carPos[2]));
				car_model_matrix = glm::rotate(car_model_matrix, 90 - carAngle, glm::vec3(0, 1, 0));
			
				glUniformMatrix4fv(glGetUniformLocation(gl_car_shader,"model_matrix"),1,false,glm::value_ptr(car_model_matrix));
				glUniform1i(glGetUniformLocation(gl_car_shader, "color"), 2);

				glBindVertexArray(mesh_vao);			
				glDrawElements(GL_TRIANGLES, mesh_num_indices, GL_UNSIGNED_INT, 0);

				// Adaugare inamici in teren si animatie
				for (int i = 0; i < NUM_ENEMIES; i++) {
					car2_model_matrix = glm::mat4();
					car2_model_matrix = glm::translate(car2_model_matrix, glm::vec3(enemies[i].x, enemies[i].y, enemies[i].z));
					car2_model_matrix = glm::rotate(car2_model_matrix, -enemies[i].betha + enemies[i].alpha - 90, glm::vec3(0,1,0));
					glUniformMatrix4fv(glGetUniformLocation(gl_car_shader,"model_matrix"),1,false,glm::value_ptr(car2_model_matrix));
					glUniform1i(glGetUniformLocation(gl_car_shader, "color"), i);

					glBindVertexArray(enemies[i].vao);			
					glDrawElements(GL_TRIANGLES, enemies[i].num_indices, GL_UNSIGNED_INT, 0);
				}
			} else {
				glViewport(0,0, screen_width, screen_height);
				// animatie steag
				glUseProgram(gl_flag_shader);
				cam[activeCam].set(glm::vec3(-0.5,-0.5,20), glm::vec3(-0.5,-0.5,0), glm::vec3(0,1,0));
				model_matrix = glm::mat4();
				//trimite variabile uniforme la shader
				glUniformMatrix4fv(glGetUniformLocation(gl_flag_shader,"model_matrix"),1,false,glm::value_ptr(model_matrix));
				glUniformMatrix4fv(glGetUniformLocation(gl_flag_shader,"view_matrix"),1,false,glm::value_ptr(cam[activeCam].getViewMatrix()));
				glUniformMatrix4fv(glGetUniformLocation(gl_flag_shader,"projection_matrix"),1,false,glm::value_ptr(projection_matrix));
			
				glUniform1f(glGetUniformLocation(gl_flag_shader, "time"), time);
				glUniform1i(glGetUniformLocation(gl_flag_shader, "win_color"), win_color);
			
				glBindVertexArray(flag.vao);			
				glDrawElements(GL_TRIANGLES, flag.num_indices, GL_UNSIGNED_INT, 0);
				time ++;
			}

		}		
	}
	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previos_width, int previous_height){
		//reshape
		if(height==0) height=1;
		screen_width = width;
		screen_height = height;
		float aspect = width / height;
		projection_matrix = glm::perspective(75.0f, aspect,0.1f, 10000.0f);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		keyStates[key_pressed] = true;
		if(key_pressed == 27) lab::glut::close();	//ESC inchide glut si 
		if(key_pressed == 32) {
			//SPACE reincarca shaderul si recalculeaza locatiile (offseti/pointeri)
			glDeleteProgram(gl_program_shader);
			gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");
		}
		if(key_pressed == 'i'){
			static bool wire =true;
			wire=!wire;
			glPolygonMode(GL_FRONT_AND_BACK, (wire?GL_LINE:GL_FILL));
		}
		if(key_pressed == 'w') { cam[0].translateForward(1.0f); }
		if(key_pressed == 's') { cam[0].translateForward(-1.0f); }
		if(key_pressed == 'a') { cam[0].translateRight(-1.0f);}		
		if(key_pressed == 'd') { cam[0].translateRight(1.0f);}
		if(key_pressed == 'e') { cam[0].translateUpword(1.0f);}
		if(key_pressed == 'q') { cam[0].translateUpword(-1.0f);}
		
		if(key_pressed == 't') { cam[0].rotateFPSoX(1.0f); }
		if(key_pressed == 'y') { cam[0].rotateFPSoX(-1.0f); }

		if(key_pressed == 'g') { cam[0].rotateFPSoY(1.0f); }
		if(key_pressed == 'h') { cam[0].rotateFPSoY(-1.0f); }

		if(key_pressed == 'b') { cam[0].rotateFPSoZ(-1.0f); }
		if(key_pressed == 'n') { cam[0].rotateFPSoZ(1.0f); }

		if(key_pressed == 'r') { cam[0].set(glm::vec3(0,120,0), glm::vec3(0,0,0), glm::vec3(0,1,0)); }
		
		// activare camera TPS de sus
		if(key_pressed == '1') {
			activeCam = 0;
		}

		// activare camera FPS a player-ului
		if(key_pressed == '2') {
			activeCam = 1;
		}

		// activare camera TPS a player-ului (de pe margine)
		if(key_pressed == '3') {
			activeCam = 2;
		}
		// activare camera FPS a inamicului 1
		if(key_pressed == '4') {
			activeCam = 3;
		}
		// activare camera FPS a inamicului 2
		if(key_pressed == '5') {
			activeCam = 4;
		}
	}
	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y){
		keyStates[key_released] = false;
	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y){
		keySpecialStates[key_pressed] = true;
		if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
		if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();
	}
	//tasta speciala ridicata
	void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y){
		keySpecialStates[key_released] = false;
	}
	//drag cu mouse-ul
	void notifyMouseDrag(int mouse_x, int mouse_y){ }
	//am miscat mouseul (fara sa apas vreun buton)
	void notifyMouseMove(int mouse_x, int mouse_y){ }
	//am apasat pe un boton
	void notifyMouseClick(int button, int state, int mouse_x, int mouse_y){ }
	//scroll cu mouse-ul
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y){ std::cout<<"Mouse scroll"<<std::endl;}

	void isInBounds(float newX, float newZ) {
	// cazul 1: e pe protiunea dreapta:
		if (newZ > -stadium[0].getLength() && newZ < stadium[0].getLength()) {
			// e pe jum din dreapta:
			if (newX >= stadium[0].getWidth() && newX <= stadium[3].getWidth()) {
					carPos[0] = newX;
					carPos[2] = newZ;
			}
			// e pe jum din stanga
			if (newX <= -stadium[0].getWidth() && newX >= -stadium[3].getWidth()) {
					carPos[0] = newX;
					carPos[2] = newZ;
			}
		}

		// cazul 2: e pe semicercul de sus:
		if (newZ <= -stadium[0].getLength()) {
			// 2.1: e pe jum din dreapta:
			if(newX > 0) {
				float co, ca;
				float crt_angle;
				co = -stadium[0].getLength() - newZ;
				ca = newX;
				crt_angle = atan2(co, ca);
				float min[2], max[2];
				min[0] = cos(crt_angle) * stadium[0].getWidth(); // x
				min[1] = sin(crt_angle) * stadium[0].getWidth() - stadium[0].getLength(); // z
				max[0] = cos(crt_angle) * stadium[3].getWidth(); // x
				max[1] = sin(crt_angle) * stadium[3].getWidth() - stadium[3].getLength(); // z
				if (newX >= min[0] && newX <= max[0] || 
					newZ >= min[1] && newZ <= max[1]) {
					carPos[0] = newX;
					carPos[2] = newZ;
				}
			} else {
				// 2.2: e pe jumatatea din stanga
				float co, ca;
				float crt_angle;
				co = -stadium[0].getLength() - newZ;
				ca = newX;
				crt_angle = atan2(co, ca);
				float min[2], max[2];
				min[0] = cos(crt_angle) * stadium[0].getWidth(); // x
				min[1] = -sin(crt_angle) * stadium[0].getWidth() - stadium[0].getLength(); // z
				max[0] = cos(crt_angle) * stadium[3].getWidth(); // x
				max[1] = -sin(crt_angle) * stadium[3].getWidth() - stadium[3].getLength(); // z
				if (newX <= min[0] && newX >= max[0] ||
					newZ <= min[1] && newZ >= max[1]) {
					carPos[0] = newX;
					carPos[2] = newZ;
				}
			}
		}

		// e pe semicercul de jos:
		if (newZ >= stadium[0].getLength()) {
			float crt_angle;
			if (newX > 0) {
				//3.1: e pe jum din dreapta:
				float co, ca;
					
				co = newZ - stadium[0].getLength();
				ca = newX;
				crt_angle = -atan2(co, ca);
				float min[2], max[2];
				min[0] = cos(crt_angle) * stadium[0].getWidth(); // x
				min[1] = sin(crt_angle) * stadium[0].getWidth() + stadium[0].getLength(); // z
				max[0] = cos(crt_angle) * stadium[3].getWidth(); // x
				max[1] = sin(crt_angle) * stadium[3].getWidth() + stadium[0].getLength(); // z
				if (newX >= min[0] && newX <= max[0] || 
					newZ >= min[1] && newZ <= max[1]) {
					carPos[0] = newX;
					carPos[2] = newZ;
				}
			} else {
				//3.2: jum din stanga
				float co, ca;
				co = newZ - stadium[0].getLength();
				ca = newX;
				crt_angle = -atan2(co, ca);
				float min[2], max[2];
				min[0] = cos(crt_angle) * stadium[0].getWidth(); // x
				min[1] = -sin(crt_angle) * stadium[0].getWidth() + stadium[0].getLength(); // z

				max[0] = cos(crt_angle) * stadium[3].getWidth(); // x
				max[1] = -sin(crt_angle) * stadium[3].getWidth() + stadium[0].getLength(); // z
				if (newX >= min[0] && newX <= max[0] || 
					newZ >= min[1] && newZ <= max[1]) {
					carPos[0] = newX;
					carPos[2] = newZ;
				}
			}
		}
	}

};

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("lab shadere 3 - camera"),800,600,100,100,true);
	lab::glut::ContextInfo context(3,3,false);
	lab::glut::FramebufferInfo framebuffer(true,true,true,true);
	lab::glut::init(window,context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
	glewExperimental = true;
	glewInit();
	std::cout<<"GLEW:initializare"<<std::endl;

	//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
	//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
	Laborator mylab;
	lab::glut::setListener(&mylab);

	//taste
	std::cout<<"Taste:"<<std::endl<<"\tESC ... iesire"<<std::endl<<"\tSPACE ... reincarca shadere"<<std::endl<<"\ti ... toggle wireframe"<<std::endl<<"\to ... reseteaza camera"<<std::endl;
	std::cout<<"\tw ... translatie camera in fata"<<std::endl<<"\ts ... translatie camera inapoi"<<std::endl;
	std::cout<<"\ta ... translatie camera in stanga"<<std::endl<<"\td ... translatie camera in dreapta"<<std::endl;
	std::cout<<"\tr ... translatie camera in sus"<<std::endl<<"\tf ... translatie camera jos"<<std::endl;
	std::cout<<"\tq ... rotatie camera FPS pe Oy, minus"<<std::endl<<"\te ... rotatie camera FPS pe Oy, plus"<<std::endl;
	std::cout<<"\tz ... rotatie camera FPS pe Oz, minus"<<std::endl<<"\tc ... rotatie camera FPS pe Oz, plus"<<std::endl;

	//run
	lab::glut::run();

	return 0;
}