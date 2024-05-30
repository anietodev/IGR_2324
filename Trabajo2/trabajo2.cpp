/*!
	trabajo2.cpp

	Programa que crea un entorno geométrico plano con objetos sobre él 
	donde se puede mover acelerando, desacelerando y rotando con las 
	flechas del teclado. En el caso de movimiento hacia atrás, los 
	giros cambian de sentido.
	
	La interfaz desarrollada consiste en una simulacion de un almacen de cajas encantado, en el que por la noche las cajas empiezan a levitar y 
	rotar sobre su eje con una secuencia definida, mientras que el personaje enciende una linterna a la que se le agotara la bateria gradualmente 
	hasta apagarse (indicador de cabina). Para volver a tener luz, se puede desactivar la noche con la tecla l o desactivar la cabina pulsando la 
	tecla c, o en el menu haciendo click derecho seleccionando cualquiera de las otras dos implementadas: una espada sangrienta (activada tambien 
	con la tecla 2) con luz roja, que cambia el aspecto de las cajas por unas con manchas de sangre, o un baston magico con luz azul (activado con 
	la tecla 3). El medio participante activado con la tecla n simula ser polvo, acorde al ambiente de almacen abandonado.

	Mas Ampliaciones:

	* Pantalla inicial explicando controles y objetivo de la simulaci¾n interactiva.
	* Cambiar apariencia del entorno con las teclas: e (almacen encantado), a (alambrico) y h (hielo). Tambien se puede hacer desde el menu.
	* Se implementan colisiones con las paredes y las cajas.
	* Se implementa movimiento de cabeza de 90 grados.
	* Vista en tercera persona pulsando la tecla t y vista en primera persona con la tecla p.
	* Se implementa minimapa para una mejor orientacion del espectador. Se activa con la tecla m o con el menu.
	* Parpadeo inicial de la linterna al activarla siendo de noche.
	* Se dibujan ventanas, estanterias de cajas y una puerta, ademas de los objetos y las paredes y suelo del entorno.
	* El almacen se puede convertir en una habitacion/almacen de cubitos de hielo presionando la tecla h, cambiando las texturas y el color del medio 
	  participante para que parezca vapor de agua, al mismo tiempo que se cambian los colores del minimapa para adecuarse a los del nuevo entorno.
	* Se implementa interaccion con el objetivo principal de la simulacion: encontrar una llave, cogerla clickando en ella, y dirigirse hacia la puerta 
	  mientras sostienes la llave para, finalmente, abrir la puerta (haciendo click en ella) y terminar la ejecucion, cambiando la textura de la puerta 
	  con una imagen de nubes dentro del marco simulando la libertad, y con un mensaje en pantalla.
	* Si se pulsa esc o la opcion salir en el menu, se imprime un mensaje distinto por pantalla.

	@author		Andrea Nieto Gil <aniegil@upv.edu.es>
	@date		May,2024
 */

#define PROYECTO "Practica 11/12 - Trabajo #2"
#define AUTOR "Andrea Nieto Gil"
#pragma warning(disable : 4305)


#include <iostream>	
#include <codebase.h>

using namespace std;
using namespace cb;

// Constantes -------------------------------------

// Coordenada maxima del plano xy
#define TAMANYO_PLANO	40.0F

// Resolucion del plano del suelo y de las paredes
#define RESOLUCION_PLANO  TAMANYO_PLANO * 4.0F

// Tamanyo del lado de la caja 
#define LADO_CAJA	1.75F

// Globales -------------------------------------

// Velocidad de simulación 
static const int tasaFPS = 60;

// Enumeraciones 
static enum { ALMACEN, ALAMBRICO, HIELO } modo; // Almacen encantado, alambrico y habitacion de hielo
static enum { LINTERNA, ESPADA, BASTON } primeraPersona;
static enum { PRIMERA_PERSONA, TERCERA_PERSONA } vista;
// NOMOV(NO Movement) y NOROT(NO Rotation) indican que no se ha pulsado flecha
static enum { NOMOV, UP, DOWN } direccion; 
static enum { NOROT, RIGHT, LEFT } rotacion;

// Variables para la camara
static Vec3 camPosition(0.0f, 0.0f, 1.0f);
static Vec3 camLookAt(0.0f, 1.0f, 1.0f);
static float girox = 0, giroy = 0;
int xanterior, yanterior;
static float alfav = 0.0f;	// º
static float vel   = 0.0f;	// km/h
// Vector con desplazamiento actual (magnitud) y direccion actual (angulo)
static float posAct[2] = { vel, 90.0f };	// Empezamos con 0 desplazamiento y angulo 90

// Variable para guardar el color del pixel
GLubyte objeto[1];

// Posicion y tamanyo de la llave para el picking
static float posActLlave[2] = { -0.02f, -3.0f}; // x e y 
GLfloat ladoLLave = 0.2f;

// Posicion, tamanyo y resolucion de la puerta para el picking
static float posActPuerta[2] = { 0.00f, TAMANYO_PLANO - 0.01f }; // x e y 
GLfloat anchoPuerta = TAMANYO_PLANO / 28.0f, alturaPuerta = TAMANYO_PLANO / 14.0f;
GLfloat resPuertax = anchoPuerta * 4.0f, resPuertay = alturaPuerta * 4.0f;

// Variables para activar/desactivar funciones
static bool pantallaControles = true; // Para activar/desactivar la pantalla inicial con los controles
static bool noche = false; // Para activar/desactivar el dibujado de las luces fijas
static bool cabina = true; // Indica si esta activada la cabina o no
static bool miniMapa = true; // Indica si esta activada el miniMapa o no
static bool salir = false; // Indica si se ha pulsado en menu la opcion salir
static bool llave = false; // Indica si se ha clickado en la llave o no. Sirve para hacer el picking de la llave
static bool puerta = false; // Indica si se ha clickado en la puerta cuando se tiene la llave. Sirve para abrir la puerta y acabar el juego
static bool linterna = false; // Indica si esta activa la linterna. Sirve para el parpadeo de la linterna

// Variable para el modo de la linterna
GLuint linternaBateria = 0; // Indica la bateria que tiene la linterna, si es 0, es de dia y no esta encendida

// Variables para el movimiento de las cajas
GLuint cajaMov = 0; // Indica la caja que se esta moviendo, si es 0, ninguna se mueve
// Velocidad y velocidad angular de una caja cuando es de noche
static float angCaja = 0.02f;	// º
static float velCaja = 0.2f;	// km/h
static float posActCaja = 0.0f;	// Las cajas empiezan con 0 desplazamiento 
static float rotActCaja[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };// Las cajas empiezan con 0º de rotacion, la 1era pos del array no se usa

// Identificadores de texturas
GLuint suelo, caja_textura, pared_textura, puerta_textura, ventana_textura, 
	linterna_textura, espada_textura, baston_textura, estanteria_textura, minimapa_textura,
	gameover_textura, paredHielo_textura, hielo_textura, estanteriaHielo_textura, 
	minimapahielo_textura, batteryFull_textura, batteryAlmostFull_textura, batteryHalf_textura, 
	batteryLessHalf_textura, batteryAlmostOff_textura, batteryOff_textura, llave_textura,
	llaveEnMano_textura, gamefinished_textura, puertanubes_textura, persona_textura, 
	cajaSangre_textura, estanteriaCajaSangre_textura, controles_textura;

// Valores default para materiales
GLfloat Em[] = { 0.0f,0.0f,0.0f,1.0f };

// Posiciones de las cajas para detectar colisiones
static float Cajax[8] = { -5.0f, -25.0f, -15.0f, -35.0f, 5.0f, 25.0f, 15.0f, 35.0f };
static float CajayTanda10[10] = { -33.0f, -26.0f, -19.0f, -12.0f, -5.0f, 2.0f, 9.0f, 16.0f, 23.0f, 30.0f };
static float CajayTanda9[9] = { -29.5f, -22.5f, -15.5f, -8.5f, -1.5f, 5.5f, 12.5f, 19.5f, 26.5f };

// Funciones -------------------------------------

// Clase contenedor para instanciar un caja con sus parametros
class caja
{
private:
	GLfloat lado;
	Vec3 pos_eje;

	// Funcion que dibuja la cara de un caja
	void cara()
	{
		GLfloat mitad_lado = lado / 2.0f;
		GLfloat v0[3] = {-mitad_lado, -mitad_lado, 0.0f}, v1[3] = { -mitad_lado, mitad_lado, 0.0f},
			v2[3] = { mitad_lado, mitad_lado, 0.0f }, v3[3] = { mitad_lado, -mitad_lado, 0.0f };
		quadtex(v0, v1, v2, v3, 0, 1, 0, 1, 10, 10);
	}

public:

	// Constructor que instancia caja asignandole sus parametros
	caja(GLfloat lado = 4.0f, Vec3 pos_eje = { 0.0f, 0.0f, 0.0f })
	{
		this->lado = lado;
		this->pos_eje.x = pos_eje.x;
		this->pos_eje.y = pos_eje.y;
		this->pos_eje.z = pos_eje.z;
	}

	// Funcion que dibuja un contenedor completo
	void dibujar()
	{
		GLfloat mitad_lado = lado / 2.0f;
		// Fondo de la caja
		cara();

		// Segunda cara de la caja
		glPushMatrix();
		glTranslatef(mitad_lado, 0, 0);
		glTranslatef(0, 0, mitad_lado);
		glRotatef(-90, 0, 1, 0);
		cara();
		glPopMatrix();


		// Tercera cara de la caja
		glPushMatrix();
		glTranslatef(-mitad_lado, 0, 0);
		glTranslatef(0, 0, mitad_lado);
		glRotatef(90, 0, 1, 0);
		cara();
		glPopMatrix();


		// Cuarta cara de la caja
		glPushMatrix();
		glTranslatef(0, 0, mitad_lado);
		glTranslatef(0, mitad_lado, 0);
		glRotatef(-90, 0, 0, 1);
		glRotatef(90, 0, 1, 0);
		cara();
		glPopMatrix();

		// Quinta cara de la caja
		glPushMatrix();
		glTranslatef(0, 0, mitad_lado);
		glTranslatef(0, -mitad_lado, 0);
		glRotatef(-90, 0, 0, 1);
		glRotatef(-90, 0, 1, 0);
		cara();
		glPopMatrix();

		// Tapa de la caja
		glPushMatrix();
		glTranslatef(0, 0, lado);
		cara();
		glPopMatrix();

	}

};

// Funcion para comprobar si hay colision con alguna pared
bool colision_paredes(GLfloat newCamx, GLfloat newCamy)
{
	bool colPared = false;
	if ((newCamx > (TAMANYO_PLANO - 0.5f)) || (newCamx < (-TAMANYO_PLANO + 0.5f)))
	{
		colPared = true;
	} 
	else if ((newCamy > (TAMANYO_PLANO - 0.5f)) || (newCamy < (-TAMANYO_PLANO + 0.5f)))
	{
		colPared = true;
	}

	return colPared;
}

// Funcion para comprobar si hay colision con las y de las cajas que estan en tandas de 10
bool colision_y_Tanda10(GLfloat newCamy)
{
	bool colCaja = false;

	for (int i = 0; i < 10; i++)
	{
		// Si esta dentro del espacio de la caja, detectar colision

		// Si es negativa la y
		if (CajayTanda10[i] < 0.0f)
		{
			if ((newCamy < (CajayTanda10[i] + 0.5f)) && (newCamy > (CajayTanda10[i] - (LADO_CAJA + 0.5f))))
			{
				colCaja = true;
				break;
			}
		}
		// Si es positiva la y
		else
		{
			if ((newCamy > (CajayTanda10[i] - 0.5f)) && (newCamy < (CajayTanda10[i] + LADO_CAJA + 0.5f)))
			{
				colCaja = true;
				break;
			}
		}	
	}
	
	return colCaja;
}

// Funcion para comprobar si hay colision con las y de las cajas que estan en tandas de 9
bool colision_y_Tanda9(GLfloat newCamy)
{
	bool colCaja = false;

	for (int i = 0; i < 9; i++)
	{
		// Si esta dentro del espacio de la caja, detectar colision

		// Si es negativa la y
		if (CajayTanda9[i] < 0.0f)
		{
			if ((newCamy < (CajayTanda9[i] + 0.5f)) && (newCamy > (CajayTanda9[i] - (LADO_CAJA + 0.5f))))
			{
				colCaja = true;
				break;
			}
		}
		// Si es positiva la y
		else
		{
			if ((newCamy > (CajayTanda9[i] - 0.5f)) && (newCamy < (CajayTanda9[i] + LADO_CAJA + 0.5f)))
			{
				colCaja = true;
				break;
			}
		}
	}

	return colCaja;
}

// Funcion para comprobar si hay alguna colision con las cajas o con las paredes
bool colisiones(GLfloat newCamx, GLfloat newCamy)
{
	// Si hay colision con las paredes no hace falta comprobar nada mas, hay colision 
	if (colision_paredes(newCamx, newCamy) == true)
	{
		return true;
	}

	// Si no hay colision con las paredes, comprobar que hay colision con alguna caja

	bool detectadaColision = false;

	// Si esta en el espacio de las x negativas, solo comprueba ese espacio
	if (newCamx < 0.0f)
	{
		// Compruebo las tandas de 10 y 9 en cada iteracion
		for (int i = 0; i < 2; i++)
		{
			// Comprobar x de las tandas de 10
			if ((newCamx < (Cajax[i] + 0.5f)) && (newCamx > (Cajax[i] - (LADO_CAJA + 0.5f))))
			{
				// Si detectamos y tambien, hay colision
				if (colision_y_Tanda10(newCamy))
				{
					detectadaColision = true;
				}
				break; // Si cumple un rango no puede estar en otro, dejamos de comprobar
			}
			// Comprobar x de las tandas de 9
			else if ((newCamx < (Cajax[i + 2] + 0.5f)) && (newCamx > (Cajax[i + 2] - (LADO_CAJA + 0.5f))))
			{
				// Si detectamos y tambien, hay colision
				if (colision_y_Tanda9(newCamy))
				{
					detectadaColision = true;
				}
				break; // Si cumple un rango no puede estar en otro, dejamos de comprobar
			}
		}
	}
	// Si esta en el espacio de las x positivas, solo comprueba ese espacio
	else
	{
		// Compruebo las tandas de 10 y 9 en cada iteracion
		for (int i = 0; i < 2; i++)
		{
			// Comprobar x de las tandas de 10
			if ((newCamx > (Cajax[i + 4] - 0.5f)) && (newCamx < (Cajax[i + 4] + LADO_CAJA + 0.5f)))
			{
				// Si detectamos y tambien, hay colision
				if (colision_y_Tanda10(newCamy))
				{
					detectadaColision = true;
				}
				break; // Si cumple un rango no puede estar en otro, dejamos de comprobar
			}
			// Comprobar x de las tandas de 9
			else if ((newCamx > (Cajax[i + 6] - 0.5f)) && (newCamx < (Cajax[i + 6] + LADO_CAJA + 0.5f)))
			{
				// Si detectamos y tambien, hay colision
				if (colision_y_Tanda9(newCamy))
				{
					detectadaColision = true;
				}
				break; // Si cumple un rango no puede estar en otro, dejamos de comprobar
			}
		}
	}		

	return detectadaColision;
}

// Inicializar y generar objetos textura
void generar_objetos_textura()
{
	// Generar un nuevo objeto textura 
	glGenTextures(1, &controles_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, controles_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"controles.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &suelo);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, suelo);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"floor.jpg");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &pared_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, pared_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"wall.jpg");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &caja_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, caja_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"box.jpg");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &puerta_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, puerta_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"door.jpg");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &puertanubes_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, puertanubes_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"door_clouds.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &linterna_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, linterna_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"flashlight.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &ventana_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, ventana_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"window.jpg");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &estanteria_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, estanteria_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"shelf_boxes.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &espada_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, espada_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"sword.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &baston_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, baston_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"magic_staff.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &minimapa_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, minimapa_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"miniMapa.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &gameover_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, gameover_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"gameover.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &hielo_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, hielo_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"ice_block.jpg");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &paredHielo_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, paredHielo_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"block_wall.jpg");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &estanteriaHielo_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, estanteriaHielo_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"shelf_ice.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &minimapahielo_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, minimapahielo_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"miniMapaHielo.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &batteryFull_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, batteryFull_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"batteryFull.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &batteryAlmostFull_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, batteryAlmostFull_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"batteryAlmostFull.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &batteryHalf_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, batteryHalf_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"batteryHalf.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &batteryLessHalf_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, batteryLessHalf_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"batteryLessHalf.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &batteryAlmostOff_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, batteryAlmostOff_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"batteryAlmostOff.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &batteryOff_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, batteryOff_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"batteryOff.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &llave_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, llave_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"key.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &llaveEnMano_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, llaveEnMano_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"key_hand.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &gamefinished_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, gamefinished_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"game_finished.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &persona_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, persona_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"man.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &cajaSangre_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, cajaSangre_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"box_blood.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Generar un nuevo objeto textura 
	glGenTextures(1, &estanteriaCajaSangre_textura);
	// Seleccionar el objeto textura como actual
	glBindTexture(GL_TEXTURE_2D, estanteriaCajaSangre_textura);
	// Cargar la imagen en el objeto textura
	loadImageFile((char*)"shelf_boxes_blood.png");
	// Definir los filtros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

// Funcion para dibujar la llave
void dibujar_llave()
{
	// Si la llave no se ha cogido aun, dibujarla en el entorno
	if (!llave)
	{
		glBindTexture(GL_TEXTURE_2D, llave_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, BRONCE);
		glMaterialfv(GL_FRONT, GL_SPECULAR, BRONCE);

		// Material llave
		glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
		glMaterialf(GL_FRONT, GL_SHININESS, 40);

		// Si es de dia
		if (!noche)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, BRONCE);
		}
		// Si es de noche
		else
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, Em);
		}

		// Medidas de la llave
		GLfloat resLlave = ladoLLave * 4.0f;
		GLfloat v0Llave[3] = { -ladoLLave, -ladoLLave, 0.0f }, v1Llave[3] = { ladoLLave, -ladoLLave, 0.0f },
			v2Llave[3] = { ladoLLave, ladoLLave, 0.0f }, v3Llave[3] = { -ladoLLave, ladoLLave, 0.0f };

		glPushMatrix();
		glTranslatef(-0.02f, -6.0f, 0.01f); // Posicion de la llave
		quadtex(v0Llave, v1Llave, v2Llave, v3Llave, 0, 1, 0, 1, resLlave, resLlave);
		glPopMatrix();

	}
}

// Funcion para dibujar las estanterias
void dibujar_estanterias()
{
	if (modo == HIELO)
	{
		glBindTexture(GL_TEXTURE_2D, estanteriaHielo_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISCLARO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISCLARO);
	}
	else if (primeraPersona == ESPADA)
	{
		glBindTexture(GL_TEXTURE_2D, estanteriaCajaSangre_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISOSCURO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISOSCURO);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, estanteria_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISOSCURO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISOSCURO);
	}


	// Material de la estanteria
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialf(GL_FRONT, GL_SHININESS, 40);

	// Si es de dia
	if (!noche)
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, GRISOSCURO);
	}
	// Si es de noche
	else
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, Em);
	}

	// Medidas de la estanteria
	GLfloat alturaEst = TAMANYO_PLANO / 14.0f, anchoEst = TAMANYO_PLANO / 28.0f;
	GLfloat resEstx = anchoEst * 4.0f, resEsty = alturaEst * 4.0f;
	GLfloat v0Est[3] = { -anchoEst, -alturaEst, 0.0f }, v1Est[3] = { anchoEst, -alturaEst, 0.0f },
		v2Est[3] = { anchoEst, alturaEst, 0.0f }, v3Est[3] = { -anchoEst, alturaEst, 0.0f };

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, alturaEst);

	// Estanterias pared frontal
	glPushMatrix();
	glTranslatef(TAMANYO_PLANO / 4.0f, 0.0f, 0);
	glTranslatef(-(TAMANYO_PLANO + TAMANYO_PLANO / 2.0f), -TAMANYO_PLANO + 0.01f, 0.0f);
	for (int i = 0; i < 4; i++)
	{
		glTranslatef(TAMANYO_PLANO / 2.0f - 2.0f * anchoEst, 0.0f, 0);
		glPushMatrix();
		glRotatef(180, 0, 1, 0);
		glRotatef(-90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPushMatrix();

		glTranslatef(2.0f * anchoEst, 0.0f, 0);

		glPushMatrix();
		glRotatef(180, 0, 1, 0);
		glRotatef(-90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glTranslatef(2.0f * anchoEst, 0.0f, 0);

		glPushMatrix();
		glRotatef(180, 0, 1, 0);
		glRotatef(-90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPopMatrix();

		glTranslatef(2.0f * anchoEst, 0.0f, 0);
	}
	glPopMatrix();

	// Estanterias pared trasera
	glPushMatrix();
	glTranslatef(TAMANYO_PLANO / 4.0f, 0.0f, 0);
	glTranslatef(-(TAMANYO_PLANO + TAMANYO_PLANO / 2.0f), TAMANYO_PLANO - 0.01f, 0.0f);
	for (int i = 0; i < 4; i++)
	{
		glTranslatef(TAMANYO_PLANO / 2.0f - 2.0f * anchoEst, 0.0f, 0);
		glPushMatrix();
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPushMatrix();

		glTranslatef(2.0f * anchoEst, 0.0f, 0);

		glPushMatrix();
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glTranslatef(2.0f * anchoEst, 0.0f, 0);

		glPushMatrix();
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPopMatrix();

		glTranslatef(2.0f * anchoEst, 0.0f, 0);
	}
	glPopMatrix();

	// Estanterias pared izquierda
	glPushMatrix();
	glTranslatef(0.0f, TAMANYO_PLANO / 4.0f, 0);
	glTranslatef(TAMANYO_PLANO - 0.01f, -(TAMANYO_PLANO + TAMANYO_PLANO / 2.0f), 0.0f);
	for (int i = 0; i < 4; i++)
	{
		glTranslatef(0.0f, TAMANYO_PLANO / 2.0f - 2.0f * anchoEst, 0);
		glPushMatrix();
		glRotatef(-90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPushMatrix();

		glTranslatef(0.0f, 2.0f * anchoEst, 0);

		glPushMatrix();
		glRotatef(-90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glTranslatef(0.0f, 2.0f * anchoEst, 0);

		glPushMatrix();
		glRotatef(-90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPopMatrix();

		glTranslatef(0.0f, 2.0f * anchoEst, 0);

	}
	glPopMatrix();

	// Estanterias pared derecha
	glPushMatrix();
	glTranslatef(0.0f, TAMANYO_PLANO / 4.0f, 0);
	glTranslatef(-TAMANYO_PLANO + 0.01f, -(TAMANYO_PLANO + TAMANYO_PLANO / 2.0f), 0.0f);
	for (int i = 0; i < 4; i++)
	{
		glTranslatef(0.0f, TAMANYO_PLANO / 2.0f - 2.0f * anchoEst, 0);
		glPushMatrix();
		glRotatef(90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPushMatrix();

		glTranslatef(0.0f, 2.0f * anchoEst, 0);

		glPushMatrix();
		glRotatef(90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glTranslatef(0.0f, 2.0f * anchoEst, 0);

		glPushMatrix();
		glRotatef(90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Est, v1Est, v2Est, v3Est, 0, 1, 0, 1, resEstx, resEsty);
		glPopMatrix();

		glPopMatrix();

		glTranslatef(0.0f, 2.0f * anchoEst, 0);
	}
	glPopMatrix();

	glPopMatrix();
}

// Funcion para dibujar las ventanas
void dibujar_ventanas()
{
	glBindTexture(GL_TEXTURE_2D, ventana_textura);

	// Material de la ventana
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISOSCURO);
	glMaterialfv(GL_FRONT, GL_SPECULAR, GRISOSCURO);
	glMaterialf(GL_FRONT, GL_SHININESS, 40);

	// Si es de dia
	if (!noche)
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, GRISOSCURO);
	}
	// Si es de noche
	else
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, Em);
	}

	// Medidas de la ventana
	GLfloat anchoVentana = 1.0f;
	GLfloat resVentana = anchoVentana * 4.0f;
	GLfloat v0Ventana[3] = { -anchoVentana, -anchoVentana, 0.0f }, v1Ventana[3] = { anchoVentana, -anchoVentana, 0.0f },
		v2Ventana[3] = { anchoVentana, anchoVentana, 0.0f }, v3Ventana[3] = { -anchoVentana, anchoVentana, 0.0f };

	glPushMatrix();
	glTranslatef(0, 0, 3.5f * anchoVentana);

	// Ventanas pared frontal
	glPushMatrix();
	glTranslatef(-TAMANYO_PLANO, -TAMANYO_PLANO + 0.01f, 0.0f);
	for (int i = 0; i < 3; i++)
	{
		glTranslatef(TAMANYO_PLANO / 2.0f, 0.0f, 0);
		glPushMatrix();
		glRotatef(-90, 1, 0, 0);
		quadtex(v0Ventana, v1Ventana, v2Ventana, v3Ventana, 0, 1, 0, 1, resVentana, resVentana);
		glPopMatrix();

	}
	glPopMatrix();

	// Ventanas pared izquierda
	glPushMatrix();
	glTranslatef(TAMANYO_PLANO - 0.01f, -TAMANYO_PLANO, 0.0f);
	for (int i = 0; i < 3; i++)
	{
		glTranslatef(0.0f, TAMANYO_PLANO / 2.0f, 0);
		glPushMatrix();
		glRotatef(-90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Ventana, v1Ventana, v2Ventana, v3Ventana, 0, 1, 0, 1, resVentana, resVentana);
		glPopMatrix();

	}
	glPopMatrix();

	// Ventanas pared derecha
	glPushMatrix();
	glTranslatef(-TAMANYO_PLANO + 0.01f, -TAMANYO_PLANO, 0.0f);
	for (int i = 0; i < 3; i++)
	{
		glTranslatef(0.0f, TAMANYO_PLANO / 2.0f, 0);
		glPushMatrix();
		glRotatef(90, 0, 0, 1);
		glRotatef(90, 1, 0, 0);
		quadtex(v0Ventana, v1Ventana, v2Ventana, v3Ventana, 0, 1, 0, 1, resVentana, resVentana);
		glPopMatrix();

	}
	glPopMatrix();

	glPopMatrix();
}

// Funcion para dibujar la puerta
void dibujar_puerta()
{
	// Textura de la puerta
	// Si no se ha clickado con la llave, dibujar puerta normal
	if (!puerta)
	{
		glBindTexture(GL_TEXTURE_2D, puerta_textura);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, puertanubes_textura);
	}

	// Material de la puerta
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, BRONCE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, BRONCE);
	glMaterialf(GL_FRONT, GL_SHININESS, 40);

	// Si es de dia
	if (!noche)
	{
		//glMaterialfv(GL_FRONT, GL_EMISSION, GRISOSCURO);
		glMaterialfv(GL_FRONT, GL_EMISSION, GRISOSCURO);

	}
	// Si es de noche
	else
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, Em);
	}

	// Medidas de la puerta
	GLfloat v0Puerta[3] = { -anchoPuerta, -alturaPuerta, 0.0f }, v1Puerta[3] = { anchoPuerta, -alturaPuerta, 0.0f },
		v2Puerta[3] = { anchoPuerta, alturaPuerta, 0.0f }, v3Puerta[3] = { -anchoPuerta, alturaPuerta, 0.0f };

	glPushMatrix();
	glTranslatef(posActPuerta[0], posActPuerta[1], alturaPuerta);
	glRotatef(90, 1, 0, 0);
	quadtex(v0Puerta, v1Puerta, v2Puerta, v3Puerta, 0, 1, 0, 1, resPuertax, resPuertay);
	glPopMatrix();
}

// Funcion que se encarga de dibujar el fondos
void dibujar_fondo()
{
	// Cambiar materiales segun este el almacen encantado o de hielo
	if (modo == HIELO)
	{
		glBindTexture(GL_TEXTURE_2D, paredHielo_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISCLARO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISCLARO);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, pared_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, BRONCE);
		glMaterialfv(GL_FRONT, GL_SPECULAR, BRONCE);
	}

	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialf(GL_FRONT, GL_SHININESS, 40);

	// Si es de dia
	if (!noche)
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, GRISOSCURO);
	}
	// Si es de noche
	else
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, Em);
	}

	// Vertices techo
	GLfloat v0Techo[3] = { -TAMANYO_PLANO, -TAMANYO_PLANO, 0.0f }, v1Techo[3] = { -TAMANYO_PLANO, TAMANYO_PLANO, 0.0f },
		v2Techo[3] = { TAMANYO_PLANO, TAMANYO_PLANO , 0.0f }, v3Techo[3] = { TAMANYO_PLANO, -TAMANYO_PLANO, 0.0f };

	// Vertices paredes
	GLfloat v0[3] = { -TAMANYO_PLANO, -TAMANYO_PLANO * 0.15f / 2.0f, 0.0f }, v1[3] = { -TAMANYO_PLANO, TAMANYO_PLANO * 0.15f / 2.0f, 0.0f },
		v2[3] = { TAMANYO_PLANO, TAMANYO_PLANO * 0.15f / 2.0f , 0.0f }, v3[3] = { TAMANYO_PLANO, -TAMANYO_PLANO * 0.15f / 2.0f, 0.0f };

	glPushMatrix();

	// Segunda cara de la habitacion
	glPushMatrix();
	glTranslatef(TAMANYO_PLANO, 0, 0);
	glTranslatef(0, 0, TAMANYO_PLANO * 0.15f / 2.0f);
	glRotatef(90, 1, 0, 0);
	glRotatef(90, 0, 1, 0);
	quadtex(v0, v1, v2, v3, 0, 1, 0, 1, RESOLUCION_PLANO * 0.15f, RESOLUCION_PLANO);
	glPopMatrix();

	// Tercera cara de la habitacion
	glPushMatrix();
	glTranslatef(-TAMANYO_PLANO, 0, 0);
	glTranslatef(0, 0, TAMANYO_PLANO * 0.15f / 2.0f);
	glRotatef(90, 1, 0, 0);
	glRotatef(-90, 0, 1, 0);
	quadtex(v0, v1, v2, v3, 0, 1, 0, 1, RESOLUCION_PLANO * 0.15f, RESOLUCION_PLANO);
	glPopMatrix();

	// Cuarta cara de la habitacion
	glPushMatrix();
	glTranslatef(0, 0, TAMANYO_PLANO * 0.15f / 2.0f);
	glTranslatef(0, TAMANYO_PLANO, 0);
	glRotatef(90, 0, 1, 0);
	glRotatef(270, 0, 0, 1);
	glRotatef(-90, 0, 1, 0);
	quadtex(v0, v1, v2, v3, 0, 1, 0, 1, RESOLUCION_PLANO * 0.15f, RESOLUCION_PLANO);
	glPopMatrix();

	// Quinta cara de la habitacion
	glPushMatrix();
	glTranslatef(0, 0, TAMANYO_PLANO * 0.15f / 2.0f);
	glTranslatef(0, -TAMANYO_PLANO, 0);
	glRotatef(90, 0, 1, 0);
	glRotatef(90, 0, 0, 1);
	glRotatef(-90, 0, 1, 0);
	quadtex(v0, v1, v2, v3, 0, 1, 0, 1, RESOLUCION_PLANO * 0.15f, RESOLUCION_PLANO);
	glPopMatrix();

	glPopMatrix();


	// Techo de la habitacion
	if (modo == HIELO)
	{
		glBindTexture(GL_TEXTURE_2D, hielo_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISCLARO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISCLARO);
	}

	glPushMatrix();
	// Subimos el techo TAMANYO_PLANO * 0.15 para que este a la misma altura que las paredes y las lamparas
	glTranslatef(0, 0, TAMANYO_PLANO * 0.15f); 
	quadtex(v0Techo, v1Techo, v2Techo, v3Techo, 0, 1, 0, 1, RESOLUCION_PLANO, RESOLUCION_PLANO);
	glPopMatrix();

}

// Funcion que se encarga de dibujar las cajas
void dibujar_cajas()
{
	// Textura cajas
	if (modo == HIELO)
	{
		glBindTexture(GL_TEXTURE_2D, hielo_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISCLARO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISCLARO);
	}
	else if (primeraPersona == ESPADA)
	{
		glBindTexture(GL_TEXTURE_2D, cajaSangre_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, BRONCE);
		glMaterialfv(GL_FRONT, GL_SPECULAR, BRONCE);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, caja_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, BRONCE);
		glMaterialfv(GL_FRONT, GL_SPECULAR, BRONCE);
	}

	// Material cajas
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialf(GL_FRONT, GL_SHININESS, 40);

	// Si es de dia
	if (!noche)
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, GRISOSCURO);
	}
	// Si es de noche
	else
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, Em);
	}

	// Instanciamos caja
	caja c1(LADO_CAJA);

	for (int j = 0; j < 2; j++)
	{
		// Cajas de la derecha
		glPushMatrix();
		glTranslatef(Cajax[j], 0.0f, 0.0f);
		for (int i = 0; i < 10; i++)
		{
			glPushMatrix();
			glTranslatef(0.0f, CajayTanda10[i], 0.0f);
			c1.dibujar();
			// Apilamos cajas en posiciones pares
			if (i % 2 == 0)
			{
				glPushMatrix();
				glTranslatef(0.0f, 0.0f, LADO_CAJA);
				// Si se mueven las cajas 
				if (cajaMov == 1)
				{
					glTranslatef(0.0f, 0.0f, posActCaja);
				}
				glRotatef(rotActCaja[1], 0, 0, 1);
				glRotatef(45, 0, 0, 1);
				c1.dibujar();
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		glPushMatrix();
		glTranslatef(Cajax[j + 2], 0.0f, 0.0f);
		for (int i = 0; i < 9; i++)
		{
			glPushMatrix();
			glTranslatef(0, CajayTanda9[i], 0);
			c1.dibujar();
			// Apilamos cajas en posiciones pares
			if (i % 2 == 0)
			{
				glPushMatrix();
				glTranslatef(0.0f, 0.0f, LADO_CAJA);
				// Si se mueven las cajas 
				if (cajaMov == 2)
				{
					glTranslatef(0.0f, 0.0f, posActCaja);
				}
				glRotatef(rotActCaja[2], 0, 0, 1);
				glRotatef(45, 0, 0, 1);
				c1.dibujar();
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		// Cajas de la izquierda
		glPushMatrix();
		glTranslatef(Cajax[j + 4], 0.0f, 0.0f);
		for (int i = 0; i < 10; i++)
		{
			glPushMatrix();
			glTranslatef(0.0f, CajayTanda10[i], 0.0f);
			c1.dibujar();
			// Apilamos cajas en posiciones pares
			if (i % 2 == 0)
			{
				glPushMatrix();
				glTranslatef(0.0f, 0.0f, LADO_CAJA);
				// Si se mueven las cajas 
				if (cajaMov == 3)
				{
					glTranslatef(0.0f, 0.0f, posActCaja);
				}
				glRotatef(rotActCaja[3], 0, 0, 1);
				glRotatef(45, 0, 0, 1);
				c1.dibujar();
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		glPushMatrix();
		glTranslatef(Cajax[j + 6], 0.0f, 0.0f);
		for (int i = 0; i < 9; i++)
		{
			glPushMatrix();
			glTranslatef(0.0f, CajayTanda9[i], 0.0f);
			c1.dibujar();
			// Apilamos cajas en posiciones pares
			if (i % 2 == 0)
			{
				glPushMatrix();
				glTranslatef(0.0f, 0.0f, LADO_CAJA);
				// Si se mueven las cajas 
				if (cajaMov == 4)
				{
					glTranslatef(0.0f, 0.0f, posActCaja);
				}
				glRotatef(rotActCaja[4], 0, 0, 1);
				glRotatef(45, 0, 0, 1);
				c1.dibujar();
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();
	}
}

// Funcion para construir el plano XY
void dibujar_planoxy()
{
	GLfloat v0[3] = { -TAMANYO_PLANO, -TAMANYO_PLANO, 0.0f }, v1[3] = { TAMANYO_PLANO, -TAMANYO_PLANO, 0.0f },
		v2[3] = { TAMANYO_PLANO, TAMANYO_PLANO, 0.0f }, v3[3] = { -TAMANYO_PLANO, TAMANYO_PLANO, 0.0f };

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Textura del suelo
	if (modo == HIELO)
	{
		glBindTexture(GL_TEXTURE_2D, hielo_textura);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISCLARO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISCLARO);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, suelo);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, GRISOSCURO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GRISOSCURO);
	}

	// Material del suelo
	glMaterialfv(GL_FRONT, GL_AMBIENT, GRISOSCURO);
	glMaterialf(GL_FRONT, GL_SHININESS, 40);

	// Si es de dia
	if (!noche)
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, GRISOSCURO);
	}
	// Si es de noche
	else
	{
		glMaterialfv(GL_FRONT, GL_EMISSION, Em);
	}

	quadtex(v0, v1, v2, v3, 0, 1, 0, 1, RESOLUCION_PLANO, RESOLUCION_PLANO);

	glPopAttrib();
	glPopMatrix();
}

// Funcion para implementar la luz solidaria a la camara
void luz_camara()
{
	// Situar el foco de luz de la linterna para que se mueva solidario a la camara
	GLfloat posicion[] = { 0.0, 0.0, 0.0, 1.0 }; // Posicion de la luz de la linterna

	// Si no hemos cogido la llave
	if (!llave)
	{
		// Cambiar color de la luz en funcion de la cabina que este activada
		if (cabina == true)
		{
			// Baston magico activado
			if (primeraPersona == BASTON)
			{
				glLightfv(GL_LIGHT0, GL_DIFFUSE, AZUL);
				glLightfv(GL_LIGHT0, GL_SPECULAR, AZUL);
			}
			// Espada activada
			else if (primeraPersona == ESPADA)
			{
				glLightfv(GL_LIGHT0, GL_DIFFUSE, ROJO);
				glLightfv(GL_LIGHT0, GL_SPECULAR, ROJO);
			}
			// Linterna activada
			else
			{
				glLightfv(GL_LIGHT0, GL_DIFFUSE, GRISCLARO);
				glLightfv(GL_LIGHT0, GL_SPECULAR, GRISCLARO);
			}
		}
		// Si no esta la cabina activada, simulamos que tenemos la linterna para poder ver si es de noche
		else
		{
			glLightfv(GL_LIGHT0, GL_DIFFUSE, GRISCLARO);
			glLightfv(GL_LIGHT0, GL_SPECULAR, GRISCLARO);
		}
	}
	// Si hemos cogido la llave, simulamos que tenemos la linterna en la otra mano para poder iluminar
	else
	{
		glLightfv(GL_LIGHT0, GL_DIFFUSE, GRISCLARO);
		glLightfv(GL_LIGHT0, GL_SPECULAR, GRISCLARO);
	}

	// Si es de noche, encender segun bateria de la linterna 
	if (noche)
	{
		// Si la bateria de la linterna se ha agotado, apagar linterna
		if ((linterna == false) || (linternaBateria == 6))
		{
			glDisable(GL_LIGHT0);
		}
		else
		{
			glEnable(GL_LIGHT0);
		}
	}

	glLightfv(GL_LIGHT0, GL_POSITION, posicion);
}

// Funcion que define la posicion de las luces fijas y las dibuja
void luces_fijas()
{
	// Definir posiciones de las luces fijas en funcion de la altura del techo
	GLfloat L1P[] = { 0.0, 20.0, TAMANYO_PLANO * 0.15 - -0.41, 1.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, L1P);
	GLfloat L2P[] = { 0.0, 10.0, TAMANYO_PLANO * 0.15 - 0.41, 1.0 };
	glLightfv(GL_LIGHT2, GL_POSITION, L2P);
	GLfloat L3P[] = { 0.0, 0.0, TAMANYO_PLANO * 0.15 - 0.41, 1.0 };
	glLightfv(GL_LIGHT3, GL_POSITION, L3P);
	GLfloat L4P[] = { 0.0, -10.0, TAMANYO_PLANO * 0.15 - 0.41, 1.0 };
	glLightfv(GL_LIGHT4, GL_POSITION, L4P);
	GLfloat L5P[] = { 0.0, -20.0, TAMANYO_PLANO * 0.15 - 0.41, 1.0 };
	glLightfv(GL_LIGHT5, GL_POSITION, L5P);
	GLfloat L6P[] = { 0.0, -30.0, TAMANYO_PLANO * 0.15 - 0.41, 1.0 };
	glLightfv(GL_LIGHT6, GL_POSITION, L6P);

	// Dibujar luces fijas en funcion de la altura del techo
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();

	// Trasladar en z
	glTranslatef(0.0, 0.0, TAMANYO_PLANO * 0.15 - 0.41);
	glTranslatef(0.0, 30.0, 0.0);

	for (int i = 6; i > 0; i--)
	{
		// Trasladar en y
		glTranslatef(0.0, -10.0, 0.0);

		// Material del cono
		glMaterialfv(GL_FRONT, GL_AMBIENT, NEGRO);
		// Si es de dia
		if (!noche)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, NEGRO);
		}
		// Si es de noche
		else
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, Em);
		}
		glMaterialfv(GL_FRONT, GL_DIFFUSE, NEGRO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, NEGRO);
		glMaterialf(GL_FRONT, GL_SHININESS, 40);

		// Dibujar un cono
		glutSolidCone(0.3, 0.3, 20, 20); // radio de la base, altura, slices, stacks

		// Material de la esfera
		glMaterialfv(GL_FRONT, GL_AMBIENT, AMARILLO);
		// Si es de dia
		if (!noche)
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, Em);
		}
		// Si es de noche
		else
		{
			glMaterialfv(GL_FRONT, GL_EMISSION, AMARILLO);
		}
		glMaterialfv(GL_FRONT, GL_DIFFUSE, AMARILLO);
		glMaterialfv(GL_FRONT, GL_SPECULAR, AMARILLO);
		glMaterialf(GL_FRONT, GL_SHININESS, 10);

		// Dibujar una esfera
		glutSolidSphere(0.1, 20, 20); // radio, slices, stacks
	}

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}

// Funcion para implementar el movimiento de cabeza
void movimiento_cabeza()
{
	static float giroxAnterior, giroyAnterior; // Variables para guardar giros validos

	// Si esta dentro del rango posible de rotacion de una cabeza humana, actualizar rotacion
	if ((girox > -90.0f) && (girox < 90.0f))
	{
		glRotatef(girox, 1, 0, 0);
		giroxAnterior = girox;
	}
	else
	{
		glRotatef(giroxAnterior, 1, 0, 0);
	}
	if ((giroy > -90.0f) && (giroy < 90.0f))
	{
		glRotatef(giroy, 0, 1, 0);
		giroyAnterior = giroy;
	}
	else
	{
		glRotatef(giroyAnterior, 0, 1, 0);
	}
}

// Funcion para dibujar las texturas que esten activadas en la screen/cabina
void texturas_screen()
{
	// Si cabina esta activada, dibujar cabina
	if (salir || cabina || miniMapa || puerta || llave || (vista == TERCERA_PERSONA))
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		/// Nueva camara ortografica
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);

		/// Display de la textura pegada a la camara
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glEnable(GL_TEXTURE_2D);

		glDisable(GL_LIGHTING);

		// Vertices de game over
		static  Vec3 v0g = { -1,-1,0 };
		static  Vec3 v1g = { 1,-1,0 };
		static  Vec3 v2g = { 1,1,0 };
		static  Vec3 v3g = { -1,1,0 };

		// Vertices de la cabina
		static  Vec3 v0c = { -1,-1,0 };
		static  Vec3 v1c = { 1,-1,0 };
		static  Vec3 v2c = { 1,0,0 };
		static  Vec3 v3c = { -1,0,0 };

		// Vertices de la cabina de tercera persona
		static  Vec3 v0t = { -0.50f,-1.0f,0.0f };
		static  Vec3 v1t = { 0.50f,-1.0f,0.0f };
		static  Vec3 v2t = { 0.5f,-0.0f,0.0f };
		static  Vec3 v3t = { -0.5f,-0.0f,0.0f };

		// Vertices del minimapa
		static  Vec3 v0m = { -0.85f,0.35f,0.0f };
		static  Vec3 v1m = { -0.35f,0.35f,0.0f };
		static  Vec3 v2m = { -0.35f,0.85f,0.0f };
		static  Vec3 v3m = { -0.85,0.85f,0.0f };

		// Vertices de la bateria de la linterna
		static  Vec3 v0l = { 0.35f,0.35f,0.0f };
		static  Vec3 v1l = { 0.85f,0.35f,0.0f };
		static  Vec3 v2l = { 0.85f,0.85f,0.0f };
		static  Vec3 v3l = { 0.35,0.85f,0.0f };

		// Mostrar pantalla de controles al iniciar el juego
		if (pantallaControles)
		{
			glBindTexture(GL_TEXTURE_2D, controles_textura);
			quadtex(v0g, v1g, v2g, v3g, 0, 1, 0, 1);
		}
		// Si se ha pulsado salir en el menu o esc en el teclado, mostrar game over
	    else if (salir)
		{
			glBindTexture(GL_TEXTURE_2D, gameover_textura);
			quadtex(v0g, v1g, v2g, v3g, 0, 1, 0, 1);
		}
		// Si se ha pulsado la puerta teniendo la llave, mostrar juego terminado
		else if (puerta)
		{
			glBindTexture(GL_TEXTURE_2D, gamefinished_textura);
			quadtex(v0g, v1g, v2g, v3g, 0, 1, 0, 1);
		}
		else
		{
			if (miniMapa)
			{
				if (modo == HIELO)
				{
					glBindTexture(GL_TEXTURE_2D, minimapahielo_textura);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, minimapa_textura);
				}
				quadtex(v0m, v1m, v2m, v3m, 0, 1, 0, 1);
			}

			// Si he cogido la llave, dibujar llave 
			if (llave)
			{
				glBindTexture(GL_TEXTURE_2D, llaveEnMano_textura);
				quadtex(v0c, v1c, v2c, v3c, 0, 1, 0, 1);
			}
			// Si no ha cogido llave, dibujar cabina segun este activada
			else
			{
				// Si esta activada la tercera persona, no dibujar cabinas de primera persona
				if (vista == TERCERA_PERSONA)
				{
					// Dibujar cabina de tercera persona
					glBindTexture(GL_TEXTURE_2D, persona_textura);
					quadtex(v0t, v1t, v2t, v3t, 0, 1, 0, 1);
				}
				else
				{
					if (cabina)
					{
						// Si esta activada la linterna, dibujar linterna
						if (primeraPersona == LINTERNA)
						{
							glBindTexture(GL_TEXTURE_2D, linterna_textura);
						}
						// Si esta activada la espada, dibujar espada
						else if (primeraPersona == ESPADA)
						{
							glBindTexture(GL_TEXTURE_2D, espada_textura);
						}
						// Si esta activada la espada, dibujar espada
						else if (primeraPersona == BASTON)
						{
							glBindTexture(GL_TEXTURE_2D, baston_textura);
						}
						quadtex(v0c, v1c, v2c, v3c, 0, 1, 0, 1);
					}
				}
				// Si es de noche y la cabina y la linterna estan activadas, dibujar bateria de la linterna
				if (linternaBateria && cabina)
				{
					if (linternaBateria == 1)
					{
						glBindTexture(GL_TEXTURE_2D, batteryFull_textura);
					}
					else if (linternaBateria == 2)
					{
						glBindTexture(GL_TEXTURE_2D, batteryAlmostFull_textura);

					}
					else if (linternaBateria == 3)
					{
						glBindTexture(GL_TEXTURE_2D, batteryHalf_textura);

					}
					else if (linternaBateria == 4)
					{
						glBindTexture(GL_TEXTURE_2D, batteryLessHalf_textura);

					}
					else if (linternaBateria == 5)
					{
						glBindTexture(GL_TEXTURE_2D, batteryAlmostOff_textura);

					}
					else if (linternaBateria == 6)
					{
						glBindTexture(GL_TEXTURE_2D, batteryOff_textura);
					}
					quadtex(v0l, v1l, v2l, v3l, 0, 1, 0, 1);
				}
			}

		}
		glPopMatrix();

		/// Restaurar la camara
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPopAttrib();
	}
}

// Funcion para calcular la direccion y rotacion de la/el camara/personaje
void calcular_direccion_rotacion()
{
	if (direccion == UP)
	{
		if (vel < 4.0f)
		{
			vel += 0.3f;
		}
	}
	else if (direccion == DOWN)
	{
		if (vel > -4.0f)
		{
			vel -= 0.3f;
		}
	}

	if (rotacion == RIGHT)
	{
		if (vel < 0.0f)
		{
			alfav = -0.08f;
		}
		else
		{
			alfav = 0.08f;
		}
	}
	else if (rotacion == LEFT)
	{
		if (vel < 0.0f)
		{
			alfav = 0.08f;
		}
		else
		{
			alfav = -0.08f;
		}
	}
	else
	{
		// Hemos dejado de rotar
		alfav = 0.0f;
	}

	// Reseteamos direccion y rotacion para el siguiente display
	direccion = NOMOV;
	rotacion = NOROT;
}

// Callback de atencion al menu de popup
void onMenu(int valor)
{
	if (valor == 0)
	{
		modo = ALAMBRICO;
	}
	else if (valor == 1)
	{
		modo = ALMACEN;
	}
	else if (valor == 2)
	{
		modo = HIELO;
	}
	else if (valor == 3)
	{
		primeraPersona = LINTERNA;
	}
	else if (valor == 4)
	{
		primeraPersona = ESPADA;
		linternaBateria = 0; // Restauro la bateria de la linterna
	}
	else if (valor == 5)
	{
		primeraPersona = BASTON;
		linternaBateria = 0; // Restauro la bateria de la linterna
	}
	else if (valor == 6)
	{
		miniMapa = !miniMapa;
	}
	else if (valor == 7)
	{
		salir = true;
	}

	glutPostRedisplay();
}

void update()
{
	// Incialmente la hora de arranque
	static int antes = glutGet(GLUT_ELAPSED_TIME);

	// Hora actual
	int ahora = glutGet(GLUT_ELAPSED_TIME);

	// Tiempo transcurrido
	float tiempo_transcurrido = (ahora - antes) / 1000.0f;

	// Variable contador para empezar a contar tiempo desde es de noche para mover las cajas
	static GLuint contadorCaja = 0;
	
	// Variable contador para empezar a contar tiempo desde que se ha pulsado salir en el menu o se ha acabado el juego 
	static float contadorExit = 0.0f;

	// Variable contador para empezar a contar tiempo desde que es de noche y esta activa la linterna para cambiar su bateria
	static float contadorLinterna = 0.0f;

	// Calcular desplazamientoz
	posAct[0] = vel * tiempo_transcurrido;

	// Calcular rotación
	posAct[1] += (alfav * 360.0F) * tiempo_transcurrido;

	// Asegurarse de que el ángulo esté en el rango -180° a 180°
	while (posAct[1] > 180.0f) {
		posAct[1] -= 360.0f;
	}
	while (posAct[1] < -180.0f) {
		posAct[1] += 360.0f;
	}

	GLfloat newCamx = camPosition.x + posAct[0] * cosf(rad(posAct[1])), newCamy = camPosition.y - posAct[0] * sinf(rad(posAct[1]));

	// Actualizo vector de posicion de la camara si no hay colision, no esta la pantalla de controles, no se ha pulsado salir y no se ha acabado el juego
	if (!salir && !puerta)
	{
		if (!pantallaControles && colisiones(newCamx, newCamy) == false)
		{
			camPosition.x = newCamx;
			camPosition.y = newCamy;
		}
		// Si hay colision, la velocidad es nula
		else
		{
			vel = 0.0f;
		}
	}
	else
	{
		// Actualizo contador desde que se pulso salir
		contadorExit += tiempo_transcurrido;
		// Si ha pasado mas de 1 segundo, acabar el programa 
		if (contadorExit > 1.0f)
		{
			exit(0);
		}
	}

	// Si es de noche
	if (noche)
	{
		// Bateria de la linterna -------------------------------------
		
		// Si esta la linterna seleccionada
		if (primeraPersona == LINTERNA)
		{
			// Actualizo contador desde que se encendio la linterna
			contadorLinterna += tiempo_transcurrido;

			// Intervalos para cambiar el indicador de la bateria y apagar/encender la linterna
			if (contadorLinterna < 5.0f) 
			{
				linternaBateria = 1;

				// Parpadeo de la linterna
				if (contadorLinterna < 0.25f)
				{
					linterna = true;
				}
				else if (contadorLinterna < 0.45f)
				{
					linterna = false;
				}
				else if (contadorLinterna < 0.65f)
				{
					linterna = true;
				}
				else if (contadorLinterna < 0.85f)
				{
					linterna = false;
				}
				else
				{
					linterna = true;
				}
			}
			else if (contadorLinterna < 10.0f)
			{
				linternaBateria = 2;
			}
			else if (contadorLinterna < 15.0f)
			{
				linternaBateria = 3;
			}
			else if (contadorLinterna < 20.0f)
			{
				linternaBateria = 4;
			}
			else if (contadorLinterna < 25.0f)
			{
				linternaBateria = 5;
			}
			else if (contadorLinterna < 30.0f)
			{
				linternaBateria = 6;
			}
		}
		// Reiniciamos contador y no dibujamos la bateria de la linterna
		else
		{
			contadorLinterna = 0.0f;
			linternaBateria = 0;
		}

		// Movimiento de las cajas -------------------------------------

		// Calcular desplazamiento en el aire de la caja
		posActCaja += velCaja * tiempo_transcurrido; 
		// Si ha llegado al tope subiendo, bajar, y viceversa
		if (posActCaja > 1.0f)
		{
			velCaja = -0.2f;
		}
		else if (posActCaja < 0.0f)
		{
			//subir = 1;
			velCaja = 0.2f;
			contadorCaja += 1; // 1 subida y bajada
		}

		// Calcular rotación en el aire de la caja
		rotActCaja[cajaMov] += (angCaja * 360.0F) * tiempo_transcurrido;
		// Si ha llegado al tope subiendo, bajar, y viceversa
		if (rotActCaja[cajaMov] > 90.0f)
		{
			angCaja = -0.02f;
		}
		else if (rotActCaja[cajaMov] < -90.0f)
		{
			angCaja = 0.02f;
		}

		// Si han pasado mas de 4 subidas y bajadas, pasar a la siguiente tanda de cajas 
		if (contadorCaja < 4)
		{
			cajaMov = 1;
		}
		else if (contadorCaja < 8)
		{
			cajaMov = 2;
		}
		else if (contadorCaja < 12)
		{
			cajaMov = 3;
		}
		else if (contadorCaja < 16)
		{
			cajaMov = 4;
		}
		// Si han pasado mas de 40 segundos, reiniciar secuencia
		else
		{
			cajaMov = 0;
			contadorCaja = 0;
		}
	}
	// Si no es de noche, reiniciar contadores, parar cajas y recargar bateria
	else
	{
		cajaMov = 0;
		contadorLinterna = 0.0f;
		contadorCaja = 0;
		linternaBateria = 0;
	}

	// Actualizo vector del punto central al que se mira 
	camLookAt.x = cosf(rad(posAct[1])) + camPosition.x;
	camLookAt.y =  -sinf(rad(posAct[1])) + camPosition.y;

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

// Inicializaciones (solo una vez)
void init()
{
	// Mensaje indicando nombre de proyecto, version y autor en consola -------------------------------------

	cout << "Iniciando " << PROYECTO << endl;
	cout << "GL version " << glGetString(GL_VERSION) << endl;
	cout << "Autora: " << AUTOR << endl;

	// Descripcion de la interfaz y ampliaciones como salida en consola -------------------------------------

	cout << endl << "La interfaz desarrollada consiste en una simulacion de un almacen de cajas encantado, ";
	cout << "en el que por la noche las cajas empiezan a levitar y rotar sobre su eje con una secuencia definida, ";
	cout << "mientras que el personaje enciende una linterna a la que se le agotara la bateria gradualmente hasta apagarse (indicador de cabina). ";
	cout << "Para volver a tener luz, se puede desactivar la noche con la tecla l o desactivar la cabina pulsando la tecla c, o ";
	cout << "en el menu haciendo click derecho seleccionando cualquiera de las otras dos implementadas: una espada sangrienta (activada tambien con la tecla 2) ";
	cout << "con luz roja, que cambia el aspecto de las cajas por unas con manchas de sangre, o un baston magico con luz azul (activado con la tecla 3). ";
	cout << "El medio participante activado con la tecla n simula ser polvo, acorde al ambiente de almacen abandonado." << endl; 

	cout << endl << "Mas Ampliaciones: " << endl << endl;
	cout << "* Pantalla inicial explicando controles y objetivo de la simulación interactiva." << endl;
	cout << "* Cambiar apariencia del entorno con las teclas: e (almacen encantado), a (alambrico) y h (hielo). Tambien se puede hacer desde el menu." << endl;
	cout << "* Se implementan colisiones con las paredes y las cajas." << endl;
	cout << "* Se implementa movimiento de cabeza de 90 grados." << endl;
	cout << "* Vista en tercera persona pulsando la tecla t y vista en primera persona con la tecla p." << endl;
	cout << "* Se implementa minimapa para una mejor orientacion del espectador. Se activa con la tecla m o con el menu." << endl;
	cout << "* Parpadeo inicial de la linterna al activarla siendo de noche." << endl;
	cout << "* Se dibujan ventanas, estanterias de cajas y una puerta, ademas de los objetos y las paredes y suelo del entorno." << endl;
	cout << "* El almacen se puede convertir en una habitacion/almacen de cubitos de hielo presionando la tecla h, cambiando las texturas ";
	cout << "y el color del medio participante para que parezca vapor de agua, al mismo tiempo que se cambian los colores ";
	cout << "del minimapa para adecuarse a los del nuevo entorno." << endl;
	cout << "* Se implementa interaccion con el objetivo principal de la simulacion: encontrar una llave, cogerla clickando en ella, ";
	cout << "y dirigirse hacia la puerta mientras sostienes la llave para, finalmente, abrir la puerta (haciendo click en ella) ";
	cout << "y terminar la ejecucion, cambiando la textura de la puerta con una imagen de nubes dentro del marco simulando la libertad, ";
	cout << "y con un mensaje en pantalla." << endl;
	cout << "* Si se pulsa esc o la opcion salir en el menu, se imprime un mensaje distinto por pantalla." << endl;

	// Menu de popup -------------------------------------
	glutCreateMenu(onMenu);
	glutAddMenuEntry("ALAMBRICO", 0);
	glutAddMenuEntry("ALMACEN", 1);
	glutAddMenuEntry("HIELO", 2);
	glutAddMenuEntry("LINTERNA", 3);
	glutAddMenuEntry("ESPADA", 4);
	glutAddMenuEntry("BASTON", 5);
	glutAddMenuEntry("MINIMAPA", 6);
	glutAddMenuEntry("SALIR", 7);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Luces -------------------------------------

	/************ Luz Ambiente ************/

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GRISOSCURO);

	/************ Foco de la linterna ************/
	GLfloat dir_central_linterna[] = { 0.0, 0.0, -1.0 }; // Apunta delante como si fuera una linterna
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir_central_linterna);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, GRISCLARO);
	glLightfv(GL_LIGHT0, GL_SPECULAR, GRISCLARO);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 10.0); // Cono con apertura de 20º para simular una linterna
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 128.0); // Atenuacion grande de la luz

	/************ Luces fijas ************/
	GLfloat dir_central_fijas[] = { 0.0, -1.0, 0.0 }; // Apuntan hacia el suelo

	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dir_central_fijas);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT1, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0); // Cono con apertura de 60º
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 10.0); // Atenuacion media de la luz

	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, dir_central_fijas);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT2, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0); // Cono con apertura de 60º
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 10.0); // Atenuacion media de la luz

	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, dir_central_fijas);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT3, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 30.0); // Cono con apertura de 60º
	glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 10.0); // Atenuacion media de la luz
	 
	glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, dir_central_fijas);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT4, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 30.0); // Cono con apertura de 60º
	glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, 10.0); // Atenuacion media de la luz

	glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, dir_central_fijas);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT5, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 30.0); // Cono con apertura de 60º
	glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 10.0); // Atenuacion media de la luz

	glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, dir_central_fijas);
	glLightfv(GL_LIGHT6, GL_DIFFUSE, AMARILLO);
	glLightfv(GL_LIGHT6, GL_SPECULAR, AMARILLO);
	glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, 30.0); // Cono con apertura de 60º
	glLightf(GL_LIGHT6, GL_SPOT_EXPONENT, 10.0); // Atenuacion media de la luz
	 

	// Texturas -------------------------------------	

	generar_objetos_textura();

	// Configuracion del motor de render -------------------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
}

// Render en backbuffer 
void displaySinMostrar()
{
	// Se hace lo mismo que en display() pero sin hacer swapbuffers
	// Cada objeto seleccionable se pinta de un color solido diferente

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Seleccionar la MODELVIEW
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Deshabilito para poder usar glcolor
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	// Situar la camara ----------------------------------------------

	// Posicionar y direccionar la camara segun vista 
	// Camara de Primera Persona
	if (vista == PRIMERA_PERSONA)
	{
		// Movimiento de cabeza al arrastrar raton con click izquierdo
		movimiento_cabeza();
		gluLookAt(camPosition.x, camPosition.y, 2.0f + camPosition.z, camLookAt.x, camLookAt.y, 2.0f + camLookAt.z, 0, 0, 1);
	}
	// Camara de Tercera persona
	else
	{
		gluLookAt(camPosition.x, camPosition.y + 2.0f, 3.0f + camPosition.z, camLookAt.x, camLookAt.y, 2.8f + camLookAt.z, 0, 0, 1);
	}


	// Geometria ----------------------------------------------

	/***** Dibujar la llave en rojo *****/
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Medidas de la llave
	GLfloat resLlave = ladoLLave * 4.0f;
	GLfloat v0Llave[3] = { -ladoLLave, -ladoLLave, 0.0f }, v1Llave[3] = { ladoLLave, -ladoLLave, 0.0f },
		v2Llave[3] = { ladoLLave, ladoLLave, 0.0f }, v3Llave[3] = { -ladoLLave, ladoLLave, 0.0f };

	glTranslatef(-0.02f, -6.0f, 0.01f); // Posicion de la llave
	glColor3fv(ROJO); // La pintamos en color rojo
	quad(v0Llave, v1Llave, v2Llave, v3Llave, resLlave, resLlave);

	glPopAttrib();
	glPopMatrix();

	/***** Dibujar la puerta en azul *****/
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Medidas de la puerta
	GLfloat v0Puerta[3] = { -anchoPuerta, -alturaPuerta, 0.0f }, v1Puerta[3] = { anchoPuerta, -alturaPuerta, 0.0f },
		v2Puerta[3] = { anchoPuerta, alturaPuerta, 0.0f }, v3Puerta[3] = { -anchoPuerta, alturaPuerta, 0.0f };

	glTranslatef(posActPuerta[0], posActPuerta[1], alturaPuerta);
	glRotatef(90, 1, 0, 0);
	glColor3fv(AZUL); // La pintamos en color azul
	quad(v0Puerta, v1Puerta, v2Puerta, v3Puerta, resPuertax, resPuertay);
	glPopAttrib();
	glPopMatrix();

}

// Callback de atencion al redibujado
void display()
{
	// Limpiar Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Seleccionar la MODELVIEW
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Definir la forma de combinar las texturas
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Seleccionar modo de dibujar el entorno -------------------------------------

	// Si se ha pulsado alambrico, dibujar alambrico y deshabilitar luces
	if(modo == ALAMBRICO)
	{
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	// Si esta activado el almacen encantado o la habitacion de hielo, dibujar solido y habilitar luces 
	else 
	{
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Luz solidaria a la camara -------------------------------------

	luz_camara();

	// Situar la camara -------------------------------------
	
	// Posicionar y direccionar la camara segun vista 
	// Camara de Primera Persona
	if (vista == PRIMERA_PERSONA)
	{
		// Movimiento de cabeza al arrastrar raton con click izquierdo
		movimiento_cabeza();
		gluLookAt(camPosition.x, camPosition.y, 2.0f + camPosition.z, camLookAt.x, camLookAt.y, 2.0f + camLookAt.z, 0, 0, 1);
	}
	// Camara de Tercera persona
	else
	{
		gluLookAt(camPosition.x, camPosition.y + 2.0f, 3.0f + camPosition.z, camLookAt.x, camLookAt.y, 2.8f + camLookAt.z, 0, 0, 1);
	}

	// Dibujar ejes -------------------------------------
	ejes();

	// Luces fijas en la escena -------------------------------------

	luces_fijas();

	// Construccion Plano XY -------------------------------------

	dibujar_planoxy();

	// Dibujar paredes -------------------------------------

	dibujar_fondo();

	// Dibujar puerta -------------------------------------

	dibujar_puerta();

	// Dibujar ventanas -------------------------------------

	dibujar_ventanas();

	// Dibujar estanterias -------------------------------------

	dibujar_estanterias();

	// Dibujar Cajas -------------------------------------
	
	dibujar_cajas();

	// Dibujar llave -------------------------------------

	dibujar_llave();

	// Calcular direccion y rotacion -------------------------------------
	
	calcular_direccion_rotacion();

	// Pegar una textura solidaria a la camara -------------------------------------

	texturas_screen();

	glutSwapBuffers();
}

// Callback de atencion al redimensionado
void reshape(GLint w, GLint h)
{
	float relacionAspecto = float(w) / h;
	glViewport(0, 0, w, h);

	// Definir la camara
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/*
	// Para mantener la relacion de aspecto en Ortografica
	if(relacionAspecto>1)
		glOrtho(-2*relacionAspecto, 2*relacionAspecto, -2, 2, -20, 20);
	else
		glOrtho(-2, 2, -2/relacionAspecto, 2/relacionAspecto, -20, 20);
	*/

	gluPerspective(30, relacionAspecto, 0.1, 100);
}

// Callback de atencion a los eventos de teclas especiales (flechas)
void onSpecialKey(int specialKey, int x, int y)
{
	switch (specialKey) {

	// Flecha_izquierda: gira a la izquierda
	case GLUT_KEY_LEFT:
		rotacion = LEFT;
		break;

	// Flecha_derecha: gira a la derecha
	case GLUT_KEY_RIGHT:
		rotacion = RIGHT;
		break;

	// Flecha_arriba: acelera en la dirección del movimiento
	case GLUT_KEY_UP:
		direccion = UP;
		break;

	// Flecha_abajo: desacelera (acelera marcha atrás)
	case GLUT_KEY_DOWN:
		direccion = DOWN;
		break;
	}

	glutPostRedisplay();
}

// Callback de atencion a los eventos de teclas alfanumericas
void onKey(unsigned char tecla, int x, int y)
{
	// polvo indica si esta activado el medio participante
	static bool polvo = false;

	// Si se pulsa space, quitar pantalla de controles
	if (tecla == 32)
	{
		pantallaControles = false;
	}
	// Si se pulsa esc, salir 
	else if (tecla == 27)
	{
		salir = true;
	}
	// Si se pulsa l, cambia de modo noche/dia
	else if (tecla == 108) 
	{
		// Cambiamos de modo
		noche = !noche;

		// Modo noche activado
		if (noche)
		{
			glEnable(GL_LIGHT0);
			glEnable(GL_LIGHT1);
			glEnable(GL_LIGHT2);
			glEnable(GL_LIGHT3);
			glEnable(GL_LIGHT4);
			glEnable(GL_LIGHT5);
			glEnable(GL_LIGHT6);
		}
		// Modo dia activado
		else
		{
			glDisable(GL_LIGHT0);
			glDisable(GL_LIGHT1);
			glDisable(GL_LIGHT2);
			glDisable(GL_LIGHT3);
			glDisable(GL_LIGHT4);
			glDisable(GL_LIGHT5);
			glDisable(GL_LIGHT6);
		}
	}
	// Si se pulsa n, activar/desactivar  medio participante
	else if (tecla == 110)
	{
		// Polvo cambia al valor contrario
		polvo = !polvo;

		// Polvo activado
		if (polvo)
		{
			glEnable(GL_FOG);
			// Si esta activado el hielo, es vapor de agua
			if (modo == HIELO)
			{
				glFogfv(GL_FOG_COLOR, GRISCLARO);
			}
			else
			{
				glFogfv(GL_FOG_COLOR, GRISOSCURO);
			}
			glFogf(GL_FOG_DENSITY, 0.01);
		}
		// Polvo desactivado
		else
		{
			glDisable(GL_FOG);
		}
	}
	// Si se pulsa c, activar/desactivar cabina
	else if (tecla == 99)
	{
		// Cabina cambia al valor contrario
		cabina = !cabina;
	}
	// Si se pulsa h, cambia de texturas el almacen y pasa a ser una habitacion de hielo
	else if (tecla == 104)
	{
		modo = HIELO;
	}
	// Si se pulsa m, activar/desactivar minimapa
	else if (tecla == 109)
	{
		miniMapa = !miniMapa;
	}
	// Si se pulsa a, activar alambrico
	else if (tecla == 97)
	{
		// Variable para alternar modo alambrico/solido
		static bool alam = false;
		alam = !alam;
		if (alam)
		{
			modo = ALAMBRICO;
		}
		else
		{
			modo = ALMACEN;
		}
	}
	// Si se pulsa e, activar/desactivar alambrico
	else if (tecla == 101)
	{
		modo = ALMACEN;
	}
	// Si se pulsa 1, activar linterna
	else if (tecla == 49)
	{
		primeraPersona = LINTERNA;
	}
	// Si se pulsa 2, activar espada
	else if (tecla == 50)
	{
		primeraPersona = ESPADA;
		linternaBateria = 0; // Restauro la bateria de la linterna
	}
	// Si se pulsa 3, activar baston magico
	else if (tecla == 51)
	{
		primeraPersona = BASTON;
		linternaBateria = 0; // Restauro la bateria de la linterna
	}
	// Si se pulsa t, se activa la tercera persona
	else if (tecla == 116)
	{
		vista = TERCERA_PERSONA;
	}
	// Si se pulsa p, se activa la primera persona
	else if (tecla == 112)
	{
		vista = PRIMERA_PERSONA;
	}

	glutPostRedisplay();
}

// Callback de atencion al pulsado de un boton del raton
void onClick(int boton, int estado, int x, int y)
{
	// Callback de atencion al pulsado de un boton del raton

	// Almacenar donde se hizo el click
	if (boton == GLUT_LEFT_BUTTON && estado == GLUT_DOWN) {
		xanterior = x;
		yanterior = y;
	}

	// Para preguntar por el pixel tengo que cambiar la Y de sentido
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLint vy = viewport[3] - y;
	// Render en backbuffer para detectar el objeto
	displaySinMostrar();
	// Leer el pixel del backbuffer en la posicion del click para ver si coincide con rojo (la llave)
	glReadPixels(x, vy, 1, 1, GL_RED, GL_UNSIGNED_BYTE, objeto);

	// Si se ha clickado en la lLave 
	if (objeto[0] == 0xFF) { 
		// Si esta en una posicion cercana, coge la llave
		if ((camPosition.x > (posActLlave[0] - 2.0f)) && (camPosition.x < (posActLlave[0] + 2.0f)) && (camPosition.y > (posActLlave[1] - 2.0f)) && (camPosition.y < (posActLlave[1] + 2.0f)))
		{
			llave = true;
		}
	}
	else
	{
		// Leer el pixel del backbuffer en la posicion del click  para ver si coincide con azul (la puerta)
		glReadPixels(x, vy, 1, 1, GL_BLUE, GL_UNSIGNED_BYTE, objeto);

		// Si se ha clickado en la puerta
		if (objeto[0] == 0xFF) { 
			// Si esta en una posicion cercana y tiene la llave, abrir la puerta
			if (llave)
			{
				if ((camPosition.x > (posActPuerta[0] - 6.0f)) && (camPosition.x < (posActPuerta[0] + 6.0f)) && (camPosition.y > (posActPuerta[1] - 6.0f)) && (camPosition.y < (posActPuerta[1] + 6.0f)))
				{
					puerta = true;
				}
			}		
		}
	}

	


}

// Callback de atencion al arrastrado del raton
void onDrag(int x, int y)
{
	static const float pixel2grados = 1;

	giroy += (x - xanterior) * pixel2grados;
	girox += (y - yanterior) * pixel2grados;

	xanterior = x;
	yanterior = y;

	glutPostRedisplay();
}

// Programa principal
int main(int argc, char** argv)
{
	// Inicializaciones
	FreeImage_Initialise();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow(PROYECTO);
	init();

	// Registro de callbacks	
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, onTimer, 1000 / tasaFPS);		// Se llama al arrancar
	glutKeyboardFunc(onKey);
	glutSpecialFunc(onSpecialKey);
	glutMouseFunc(onClick);
	glutMotionFunc(onDrag);

	// Bucle de atencion a eventos
	glutMainLoop();

	FreeImage_DeInitialise();
}