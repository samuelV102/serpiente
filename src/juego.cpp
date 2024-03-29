#include "juego.hpp"

Juego::Juego(Nivel *nivel, Comida *comida, Serpiente *serpiente)
{
	this->nivel = nivel;
	this->mapa = nivel->obtenerMapaActual();
	this->comida = comida;
	this->serpiente = serpiente;
	this->continua = true;

	this->puntos = 0;

	this->conf = this->global.configuracion("juego");
	this->vidas = (this->conf["vidas"].isInt()) ? this->conf["vidas"].asInt() : 3;
	this->puntos_proximo_nivel = (this->conf["puntos_proximo_nivel"].isInt()) ? this->conf["puntos_proximo_nivel"].asInt() : 2;

	this->fichero = new ifstream("mensaje.txt");
	char c;

	if (fichero->good())
	{
		while (fichero->get(c))
		{
			this->mensaje += c;
		}
	}
	else
	{
		this->mensaje = "No hay mensaje";
	}

	fichero->close();
}

Juego::~Juego()
{
	delwin(this->local_panel->win);
	del_panel(this->local_panel);
}

void Juego::iniciar(void)
{
	bool si_nivel_avanzado;
	bool serpiente_entro_tunel;

	this->actualizarTablero();
	this->mapa->dibujarTunel();

	while (this->continua)
	{
		this->comida->dibujar();

		this->continua = this->global.capturarComando();

		this->serpiente->limite_map(this->global.comando_nuevo);

		this->serpiente->cambiar_direccion(this->global.comando_actual, this->global.comando_nuevo);

		//mover gusano
		this->serpiente->mover();

		//velocidad de desplazamiento del gusano
		this->retardo(UNIDAD_CUADROS / this->serpiente->obtenerVelocidad());

		//verificar si el gusano esta en la misma posicion x y del alimento
		if (this->serpiente->si_come(this->comida->x, this->comida->y, this->puntos))
		{
			this->comida->randomXY();
			this->puntos++;
		}

		if (this->serpiente->si_choca_con_el())
		{
			global.mensaje(MAP_ALTO + 1, 35, "CHOCASTE CON TU CUERPO.", 2);
			global.mensaje(MAP_ALTO + 1, 35, "PRESIONA UNA TECLA PARA CONTINUAR.", true);
			this->vidas--;
			this->serpiente->mover(37, 18, this->global.comando_nuevo);
			this->mapa->dibujarMapa();
			this->mapa->dibujarTunel();
		}

		if (this->serpiente->si_choca_con_muro(this->mapa->eje))
		{
			global.mensaje(MAP_ALTO + 1, 35, "CHOCASTE CON EL MURO.", 2);
			global.mensaje(MAP_ALTO + 1, 35, "PRESIONA UNA TECLA PARA CONTINUAR.", true);
			this->vidas--;
			this->serpiente->mover(37, 18, this->global.comando_nuevo);
			this->mapa->dibujarMapa();
			this->mapa->dibujarTunel();
		}

		if (this->vidas == 0)
		{
			this->final();
		}

		if ((this->puntos % this->puntos_proximo_nivel) == 0 && this->puntos != 0)
		{
			this->mapa->dibujarTunel();
			this->comida->ocultar();

			if (this->serpiente->si_choca_con_tunel())
			{
				global.mensaje(MAP_ALTO + 1, 35, "CHOCASTE CON EL TUNEL.", 2);
				global.mensaje(MAP_ALTO + 1, 35, "PRESIONA UNA TECLA PARA CONTINUAR.", true);
				this->vidas--;
				this->serpiente->mover(37, 18, this->global.comando_nuevo);
			}
			serpiente_entro_tunel = this->serpiente->entrar_tunel(this->global.comando_nuevo);
		}

		if (this->mapa->tunel_visto && this->serpiente->salir_tunel(this->global.comando_nuevo))
		{
			this->mapa->borrarTunel();
		}

		if (serpiente_entro_tunel)
		{
			global.mensaje(MAP_ALTO + 1, 35, "NIVEL COMPLETADO.", 4);
			global.mensaje(MAP_ALTO + 1, 35, "PRESIONA UNA TECLA PARA CONTINUAR.", true);
			this->comida->randomXY();
			this->comida->mostrar();
			this->serpiente->mover(37, 18, this->global.comando_nuevo);
			this->serpiente->reiniciarVelocidad();
			this->puntos = 0;
			si_nivel_avanzado = this->nivel->avanzarNivel();

			if (!si_nivel_avanzado)
			{
				this->final();
			}
			serpiente_entro_tunel = false;
		}

		this->serpiente->dibujar();

		this->actualizarTablero();
	}
}

void Juego::final()
{
	this->continua = false;
}

void Juego::terminarJuego()
{
	this->mensajefinal();
	global.mensaje(MAP_ALTO + 1, 35, "JUEGO TERMINADO.", 4);
	global.mensaje(MAP_ALTO + 1, 35, "PRESIONA Q PARA FINALIZAR.", true);
	while (getch() != 'q');
	erase();
	attroff(COLOR_PAIR(1));
	endwin();
}

void Juego::retardo(int n)
{
	usleep(n * 1000);
}

void Juego::actualizarTablero()
{
	attron(A_BOLD);
	mvprintw(MAP_INI_Y - 1, 2, "Mapa: %s", this->mapa->obtenerNombre().c_str());
	mvprintw(MAP_INI_Y - 1, 36, "Puntos: %i", this->puntos);
	mvprintw(MAP_INI_Y - 1, 70, "Vidas: %i", this->vidas);

	mvprintw(MAP_INI_Y + MAP_ALTO, 5, "Velocidad: %i cuadro/s", this->serpiente->obtenerVelocidad());
	attroff(A_BOLD);
	refresh();
}

void Juego::mensajefinal()
{
	WINDOW *local_win;

	local_win = newwin(22, 79, 0, 0);
	box(local_win, 0, 0);
	mvwprintw(local_win, 8, 20, this->mensaje.c_str());
	this->local_panel = new_panel(local_win);
	update_panels();
	doupdate();
}