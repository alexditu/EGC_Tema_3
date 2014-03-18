EGC - Tema 3
	Atletism
	Ditu Alexandru Mihai 333CA
	
Cuprins
1. Cerinta
2. Utilizare
3. Implementare
4. Testare
5. Probleme aparute
6. Continutul Arhivei
7. Functionalitati

1. Cerinta
	Sa se implementeze o cursa 3D.

2. Utilizare

	Tastele folosite sunt:
		- Sagetile UP, DOWN, LEFT, RIGHT pentru miscarea masinii.
		- Pentru miscarea camerei FPS de sus se folosesc urmatoarele taste:
			- Translatie:
				- w/s - sus/jos
				- a/d - stanga/dreapta
				- q/e - inainte/inapoi
			- Rotatie:
				- t/y - fata de axa forward
				- g/h - fata de axa right
				- b/n - fata de axa up
			- Pentru resetare camera in pozitia initiala:
				- r
			
3. Implementare
	IDE:	Visual Studio 2012
	SO:		Windows 7 Professional, 64bit
	Framework-ul folosit este cel din lab3.
	Mentionez ca nu am testat tema pe niciun alt calculator.
	
	Pe langa clasele din framework am mai adaugat urmatoarele clase:
	- Enemy.cpp/.h
		- retine datele despre masinile adversarilor: pozitie, unghi, viteza etc.
	- Flag.cpp/.h
		- pentru a putea creea steagul (din vertices si indices)
		- tot aici am si vao, ibo, vbo pentru steag
	- Stadium.cpp/.h
		- creeaza o singura forma pentru stadion (acel dreptunghi si 2 semicercuri)
		- pentru a desena stadionul propriuzis, format din 3 benzi si partea din centru
		  am folosit 4 obiecte de tip Stadium puse unul peste altul de culori diferite 
		  pentru a obtine efectul dorit
	- TpsObject.cpp/.h
		- retine datele pentru obiectul tps ce urmareste player-ul
		- este o camera video importata cu loadObject
	- VerticesAndColorStruct.h
		- retine o structura pentru un punct si o culoare
		- o folosesc pentru a desena obiecte
	- car_shader_fragment.glsl/car_shader_vertex.glsl
		- pentru masini am un shader separat pentru a le putea colora diferit
	- flag_shader_fragment.glsl/flag_shader_vertex.glsl
		- pentru steag am un shader separat pentru a-l putea color in functie de
		  masina care castiga si pentru a putea realiza animatia
		  
	Implementarea:
		- initial in constructorul din clasa Laborator incarc obiectele necesare: 3 masini,
		  o camera, creez stadionul, creez steagul;
		- in NotifyBeginFrame (care e si cea mai stufoasa):
			- update-ez pozitiile obiectelor: atat a masinilor cat si a camerelor atasate
			  acestora
			- mentionez ca nu am folosit functiile din laborator, in cea mai mare
			  parte am actualizat manual camerele folosind cam.set(...)
			- verific daca nu cumva cineva a trecut linia de sosire
		- in notifyDisplayFrame:
			- trimit toate variabilele uniforme necesare fiecarui shader si desenez
			  obiectele avand pozitiie actualizate deja;
			- in cazul in care o masina castiga apare pe ecran doar steagul
			  avand culoarea masinii castigatoare
			- pentru a putea anima steagul am nevoie de o variabila uniforma (time) care
			  o folosesc in variatia lui z si anume:
				v.z = sin(v.x + time*0.5);
				unde v.z este coordonata Z a fiecarui punct de pe steag
			- pentru a rezolva acest punct am folosit informatii de la:
				http://zach.in.tu-clausthal.de/teaching/cg_literatur/glsl_tutorial/
				
	
	
4. Testare
	...
5. Probleme aparute
	...
6. Continutul arhivei
	- tot frameworkul de la laboratorul 6
	- Enemy.cpp/.h
	- Flag.cpp/.h
	- Stadium.cpp/.h
	- TpsObject.cpp/.h
	- VerticesAndColorStruct.h
	- car_shader_fragment.glsl/car_shader_vertex.glsl
	- flag_shader_fragment.glsl/flag_shader_vertex.glsl
	- fisierele .obj:
		- "natla car.obj" pentru masina
		- "cameraTV.obj" pentru obiectul TPS al player-ului
	
7. Functionalitati
	- toate masinile (2 inamici 1 player) incep din pozitia de start (0,0,0)
	- inamicii se misca singuri avand vitezele fixate si isi pastreaza
	  banda pe care au inceput
	- inamicul verde se misca mai repede decat inamicul rosu si mai incet
	  decat viteza maxima a player-ului (care e albastru)
	- in functie de cel care ajunge primul la start (trece de 0,0,0 din nou)
	  pe ecran nu mai apare niciun obiect in afara de steagul avand culoarea
	  castigatorului
	- Mentionez ca viteza masinilor se citeste dintr-un fisier numit "speed.txt"
	
	
	
	
	
	
	
	
	
	
	