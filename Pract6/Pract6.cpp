/* !
Pract6.cpp

Programa para dibujar un sistema de engranajes de un cronometro

@author		Andrea Nieto Gil <aniegil@upv.edu.es>
@date		Apr, 2024
*/

#define PROYECTO "IGR::S#06::Engranajes Cronometro"

#include <iostream>	
#include <sstream>
#include <codebase.h>

using namespace std;
using namespace cb;

// Velocidad de simulación de 60x
static const int tasaFPS = 60;

// Variables dependientes del tiempo
static float gradosCorona =		0.0F;
static float gradosPinyon =		0.0F;
static float gradosPinyonMin =	0.0F;
static float gradosCamara =		0.0F;

// Parametros de los engranajes
#define MODULO			0.04F

// Parametros del engranaje grande (corona de minutos)
#define N_DIENTES_GRAN  81
#define DIAM_PRIM_GRAN  (float)(MODULO * N_DIENTES_GRAN)
#define DIAM_EXT_GRAN	(float)(DIAM_PRIM_GRAN + 2.0F*MODULO)
#define DIAM_EJE_GRAN	(float)(DIAM_PRIM_GRAN - 60.32F*MODULO)
#define H_DIENTE_GRAN	(float)(1.16F * MODULO)

// Parametros del engranaje mediano
#define N_DIENTES_MED   35
#define DIAM_PRIM_MED   (float)(MODULO * N_DIENTES_MED)
#define DIAM_EXT_MED	(float)(DIAM_PRIM_MED + 2.0F*MODULO)
#define DIAM_EJE_MED	(float)(DIAM_PRIM_MED - 30.32F*MODULO)
#define H_DIENTE_MED	(float)(1.16F * MODULO)

// Parametros del engranaje pequenyo (pinyon de segundos)
#define N_DIENTES_PEQ   15
#define DIAM_PRIM_PEQ   (float)(MODULO * N_DIENTES_PEQ)
#define DIAM_EXT_PEQ	(float)(DIAM_PRIM_PEQ + 2.0F*MODULO)
#define DIAM_EJE_PEQ	(float)(DIAM_PRIM_PEQ - 10.32F*MODULO)
#define H_DIENTE_PEQ	(float)(1.16F * MODULO)


class engranaje
{
private:
	int ndientes;
	float diam_ext;
	float diam_prim;
	float diam_eje;
	GLfloat* coorpeque;  // Coordenadas del diam_eje
	GLfloat* coormed;    // Coordenadas del diam_primitivo
	GLfloat* coorgrande; // Coordenadas del diam_h_diente

	// Funcion que dibuja la cara de un engranaje
	void cara(GLfloat* coorpeque, GLfloat* coormed, GLfloat* coorgrande)
	{
		glBegin(GL_TRIANGLE_STRIP); //órdenes de dibujo

		for (int i = 0; i <= ndientes * 2; i += 2)
		{
			glVertex3f(coormed[i], coormed[i + 1], 0);
			glVertex3f(coorgrande[i], coorgrande[i + 1], 0);
			glVertex3f(coorpeque[i], coorpeque[i + 1], 0);
			glVertex3f(coormed[(i + 2)], coormed[(i + 3)], 0);
			glVertex3f(coorpeque[(i + 2)], coorpeque[(i + 3)], 0);
		}

		glEnd();
	}

	// Funcion que dibuja las uniones interiores de un engranaje
	void uniones_interiores(GLfloat* coorpeque, GLfloat* coormed, GLfloat* coorgrande)
	{
		glBegin(GL_QUAD_STRIP);

		for (int i = 0; i < ndientes * 2; i += 2)
		{
			glVertex3f(coorpeque[i], coorpeque[i + 1], 0);
			glVertex3f(coorpeque[i], coorpeque[i + 1], 0.3);
		}
		// Repetimos el inicio para cerrar la unión
		glVertex3f(coorpeque[0], coorpeque[1], 0);
		glVertex3f(coorpeque[0], coorpeque[1], 0.3);

		glEnd();
	}

	// Funcion que dibuja las uniones exteriores de un engranaje
	void uniones_exteriores(GLfloat* coorpeque, GLfloat* coormed, GLfloat* coorgrande)
	{
		glBegin(GL_QUAD_STRIP);

		for (int i = 0; i < ndientes * 2; i += 2)
		{
			glVertex3f(coormed[i], coormed[i + 1], 0);
			glVertex3f(coormed[i], coormed[i + 1], 0.3);
			glVertex3f(coorgrande[i], coorgrande[i + 1], 0);
			glVertex3f(coorgrande[i], coorgrande[i + 1], 0.3);
		}
		// Repetimos el inicio para cerrar la unión
		glVertex3f(coormed[0], coormed[1], 0);
		glVertex3f(coormed[0], coormed[1], 0.3);
		glVertex3f(coorgrande[0], coorgrande[1], 0);
		glVertex3f(coorgrande[0], coorgrande[1], 0.3);

		glEnd();
	}

	// Funcion que rellena las coordenadas de los 3 vectores con los parametros del engranaje 
	void rellenar_vector_coor(GLfloat* coorpeque, GLfloat* coormed, GLfloat* coorgrande)
	{
		float desfase = -(2 * PI) / (ndientes * 2);

		// Rellenamos coordenadas del diam_eje
		for (int i = 0; i < (ndientes * 2); i += 2)
		{
			coorpeque[i] = diam_eje * cosf((float(i) / 2) * 2 * PI / ndientes);
			coorpeque[i + 1] = diam_eje * sinf((float(i) / 2) * 2 * PI / ndientes);
		}
		// Repetimos el inicio para cerrar la unión
		coorpeque[ndientes * 2] = coorpeque[0];
		coorpeque[ndientes * 2 + 1] = coorpeque[1];
		coorpeque[ndientes * 2 + 2] = coorpeque[2];
		coorpeque[ndientes * 2 + 3] = coorpeque[3];


		// Rellenamos coordenadas del diam_primitivo
		for (int i = 0; i < ndientes * 2; i += 2)
		{
			coormed[i] = diam_prim * cosf((float(i) / 2) * 2 * PI / ndientes + desfase);
			coormed[i + 1] = diam_prim * sinf((float(i) / 2) * 2 * PI / ndientes + desfase);
		}
		// Repetimos el inicio para cerrar la unión
		coormed[ndientes * 2] = coormed[0];
		coormed[ndientes * 2 + 1] = coormed[1];
		coormed[ndientes * 2 + 2] = coormed[2];
		coormed[ndientes * 2 + 3] = coormed[3];


		// Rellenamos coordenadas del diam_exterior
		for (int i = 0; i < ndientes * 2; i += 2)
		{
			coorgrande[i] = diam_ext * cosf((float(i) / 2) * 2 * PI / ndientes);
			coorgrande[i + 1] = diam_ext * sinf((float(i) / 2) * 2 * PI / ndientes);
		}
		// Repetimos el inicio para cerrar la unión
		coorgrande[ndientes * 2] = coorgrande[0];
		coorgrande[ndientes * 2 + 1] = coorgrande[1];
		coorgrande[ndientes * 2 + 2] = coorgrande[2];
		coorgrande[ndientes * 2 + 3] = coorgrande[3];

	}

public:

	// Constructor que instancia engranaje asignandole sus parametros
	engranaje(int ndientes = 40, float diam_ext = 0.9F, float diam_prim = 0.7F, float diam_eje = 0.3F)
	{
		this->ndientes = ndientes;
		this->diam_ext = diam_ext;
		this->diam_prim = diam_prim;
		this->diam_eje = diam_eje;
		int n = ndientes * 2 + 4;
		coorpeque = new GLfloat[n];
		coormed = new GLfloat[n];
		coorgrande = new GLfloat[n];

		rellenar_vector_coor(coorpeque, coormed, coorgrande);
	}

	// Destructor de engranaje
	~engranaje()
	{
		delete[] coorpeque;
		delete[] coormed;
		delete[] coorgrande;
	}

	// Funcion que dibuja un engranaje completo
	void dibujar()
	{
		// Primera cara del engranaje
		glPushMatrix();
		glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT); 
		glColor3f(0, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		cara(coorpeque, coormed, coorgrande);
		glPopAttrib();		
		glPopMatrix();
		 
		// Segunda cara del engranaje
		glPushMatrix();
		glTranslatef(0, 0, 0.3);
		glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT); 
		glColor3f(0, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		cara(coorpeque, coormed, coorgrande);
		glPopAttrib();
		glPopMatrix();

		// Uniones Interiores y exteriores del engranaje
		glPushMatrix();
		glColor3f(0, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		uniones_interiores(coorpeque, coormed, coorgrande);
		glColor3f(0, 0, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		uniones_exteriores(coorpeque, coormed, coorgrande);
		glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT); 
		glColor3f(1, 1, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		uniones_exteriores(coorpeque, coormed, coorgrande);
		glPopMatrix();

	}

};

// Funcion que muestra FPS en la ventana de dibujo
void muestraFPS()
{
	static int antes = glutGet(GLUT_ELAPSED_TIME);
	static int fotogramas = 0;
	int ahora, tiempo_transcurrido;

	// Cada que pase por aqui incremento los fotogramas
	fotogramas++;

	// Calculo el tiempo transcurrido
	ahora = glutGet(GLUT_ELAPSED_TIME);
	tiempo_transcurrido = ahora - antes;
	// si ha transcurrido un segundo, mostrar los fotogramas
	if (tiempo_transcurrido > 1000) {
		// modificar la barra de titulo
		stringstream titulo;
		titulo << "FPS= " << fotogramas;
		glutSetWindowTitle(titulo.str().c_str());
		// reinicio a cuenta
		fotogramas = 0;
		antes = ahora;
	}

}

void update()
{
	// Animacion coherente con el tiempo transcurrido
	static const float omega = 1; // Vueltas por segundo

	// Incialmente la hora de arranque
	static int antes = glutGet(GLUT_ELAPSED_TIME);

	// Hora actual
	int ahora = glutGet(GLUT_ELAPSED_TIME);

	// Tiempo transcurrido
	float tiempo_transcurrido = (ahora - antes) / 1000.0f;

	// Velocidad_corona = 1 vuelta/hora
	gradosCorona += omega / 3600.0F * 360.0F * tiempo_transcurrido;   

	// Velocidad_pinyon = vel_corona * n_dien_corona/n_dien_pinyon
	gradosPinyonMin += ((omega / 3600.0F * 360.0F * tiempo_transcurrido) * DIAM_EXT_GRAN) / DIAM_PRIM_PEQ;

	// Velocidad_pinyon = 1 vuelta/minuto
	//gradosPinyon += omega / 60.0F * 360.0F * tiempo_transcurrido;		
	gradosPinyon += ((((omega / 3600.0F * 360.0F * tiempo_transcurrido) * DIAM_EXT_GRAN) / DIAM_PRIM_PEQ) * DIAM_EXT_MED) / DIAM_PRIM_PEQ;

	// Velocidad_camara = 1 vuelta/minuto
	gradosCamara += omega / 60.0F * 360.0F * tiempo_transcurrido;         
	antes = ahora;


	// Encolar un evento de redibujo
	glutPostRedisplay();
}

void onTimer(int tiempo)
{
	// Sirve para re-arrancar el reloj de cuenta atras
	glutTimerFunc(tiempo, onTimer, tiempo);

	update();
}

void init() //1 sola vez
// Inicializaciones
{
	cout << "GL version " << glGetString(GL_VERSION) << endl;

	//AÑADIDO
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Habilitamos la visibilidad
	glEnable(GL_DEPTH_TEST); // Dibujar lo que está delante encima de lo de detrás, lo que está detrás permanece oculto

	// Inicializaciones 
	glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
}

void display()
// Funcion de atencion al dibujo
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Borra tambien el buffer de profundidad
	glMatrixMode(GL_MODELVIEW);							// Selecciona la modelview
	glLoadIdentity();									// Carga la matriz identidad

	//Animación de la cámara

	// Para que gire 1 vuelta/min alrededor del eje y
	Vec3 camPosition(15 * cosf(rad(gradosCamara)), 1, -15 * sinf(rad(gradosCamara)));
	gluLookAt(camPosition.x, camPosition.y, camPosition.z, 0, 0, 0, 0, 1, 0); 
	
	// Para que la camara este estatica 
	//gluLookAt(1, 1, 15, 0, 0, 0, 0, 1, 0);									  


	ejes();
	glRotatef(20, 0, 1, 0);
	glColor3f(1, 1, 1);
	glLineWidth(2); 

	// Instanciamos engranajes grande, mediano y pequenyo 
	engranaje en_grande(N_DIENTES_GRAN, DIAM_EXT_GRAN, DIAM_PRIM_GRAN, DIAM_EJE_GRAN);
	engranaje en_peq(N_DIENTES_PEQ, DIAM_EXT_PEQ, DIAM_PRIM_PEQ, DIAM_EJE_PEQ);
	engranaje en_med(N_DIENTES_MED, DIAM_EXT_MED, DIAM_PRIM_MED, DIAM_EJE_MED);

	// Dibujar engranaje grande (corona minutos)
	glPushMatrix();
	glRotatef(-gradosCorona, 0, 0, 1);														// Rotacion de la corona
	glRotatef((PI / (float)N_DIENTES_GRAN), 0, 0, 1);										// Rotamos PI/n_dientes para que encajen
	en_grande.dibujar();
	glPopMatrix();

	// Trasladamos 'Diametros ext - altura_diente' en x
	glTranslatef((DIAM_EXT_GRAN + DIAM_EXT_PEQ) - (DIAM_EXT_GRAN - DIAM_PRIM_GRAN), 0, 0); 

	// Dibujar engranaje pequeño
	glPushMatrix();
	glRotatef(gradosPinyonMin, 0, 0, 1);													// Rotacion del pinyon de la corona de min
	en_peq.dibujar();
	glPopMatrix();

	// Trasladamos 'anchura_diente' en -z
	glTranslatef(0, 0, -0.32);

	// Dibujar engranaje mediano
	glPushMatrix();
	glRotatef(gradosPinyonMin, 0, 0, 1);													// Rotacion del pinyon de la corona de min
	glRotatef((PI / (float)N_DIENTES_MED), 0, 0, 1);										// Rotamos PI/n_dientes para que encajen
	en_med.dibujar();
	glPopMatrix();

	// Trasladamos 'Diametros ext - altura_diente' en x
	glTranslatef((DIAM_EXT_MED + DIAM_EXT_PEQ) - (DIAM_EXT_MED - DIAM_PRIM_MED), 0, 0); 

	// Dibujar engranaje pequeño
	glPushMatrix();
	glRotatef(-gradosPinyon, 0, 0, 1);														// Rotacion del pinyon de segundos 
	en_peq.dibujar();
	glPopMatrix();

	// Trasladamos 'anchura_diente' en -z
	glTranslatef(0, 0, -0.32);

	// Dibujar engranaje mediano
	glPushMatrix();
	glRotatef(-gradosPinyon, 0, 0, 1);														// Rotacion del pinyon de segundos
	glRotatef((PI / (float)N_DIENTES_MED), 0, 0, 1);										// Rotamos PI/n_dientes para que encajen
	en_med.dibujar();
	glPopMatrix();

	
	muestraFPS();

	glutSwapBuffers();
}

void reshape(GLint w, GLint h)
// Funcion de atencion al redimensionamiento
{
	// Marco dentro del area de dibujo
	glViewport(0, 0, w, h);		
	float razon = (float)w / h;	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/*
	//ORTOGRÁFICA: un cajón (observador en el infinito)
	if (razon > 1) {									// El marco es mas ancho que alto
		glOrtho(-2 * razon, 2 * razon, -2, 2, -2, 2);	// amplio la parte horizontal
	}
	else {												//marco más alto que ancho
		glOrtho(-2, 2, -2 / razon, 2 / razon, -2, 2);	// amplio las dimensiones en la parte vertical
	}
	*/
	//PERSPECTIVA: pirámide
	gluPerspective(45, razon, 0.1, 40);
}

int main(int argc, char** argv)
// Programa principal
{
	// Inicializaciones
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow(PROYECTO);
	init(); 

	// Registro de callbacks	
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutTimerFunc(2000, onTimer, 1000 / tasaFPS);		// Se llama 2 seg despues de arrancar

	// Bucle de atencion a eventos
	glutMainLoop();
}