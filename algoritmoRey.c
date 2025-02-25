#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define RONDAS_MAX 10

char mayoria(int size, char *planes)
{
	// Función que determina cuál fue el plan que tuvo la mayoría
	int ataque = 0;
	int retirar = 0;
	char plan = '0';

	for (int i = 0; i < size; i++)
	{
		if (planes[i] == 'A')
		{
			ataque++;
		}
		if (planes[i] == 'R')
		{
			retirar++;
		}
	}

	// Actualizamos el plan según la mayoría
	if (ataque > retirar)
	{
		plan = 'A';
	}
	if (retirar > ataque)
	{
		plan = 'R';
	}

	return plan;
}

int es_valida(int size, char *planes)
{
	// función que determina si una votación fue válida o no
	// para que una votación sea válida necesita haber de cierto plan de acuerdo con n/2 + t
	int ataques = 0;
	int retirar = 0;

	for (int i = 0; i < size; i++)
	{
		if (planes[i] == 'A')
		{
			ataques++;
		}
		if (planes[i] == 'R')
		{
			retirar++;
		}
	}

	int traidores = (size - 1) / 4;
	int minimo_votos = (size / 2) + traidores;

	if (ataques >= minimo_votos || retirar >= minimo_votos)
	{
		return 1;
	}

	return 0;
}

int *generarTraidores(int size)
{
	int numTraidores = (size - 1) / 4;					// Definimos el número máximo de traidores.
	int *traidores = (int *)malloc(size * sizeof(int)); // Definimos el arreglo donde guardamos si es traidor o no.

	// Inicializamos el arreglo con todos los nodos como no traidores.
	for (int i = 0; i < size; i++)
	{
		traidores[i] = 0;
	}

	srand(time(NULL)); // Inicializamos la semilla para la aleatoriedad

	// Seleccionamos aleatoriamente a los traidores
	for (int i = 0; i < numTraidores; i++)
	{
		int traidor;
		do
		{
			traidor = rand() % size; // Elegimos un traidor aleatorio
		} while (traidores[traidor] == 1); // Verificamos que no esté marcado como traidor
		traidores[traidor] = 1; // Marcamos este nodo como traidor
	}

	return traidores; // Devolvemos el arreglo de traidores
}

uint32_t pseudo_hash(int id)
{
	uint32_t hash = id ^ (uint32_t)time(NULL); // Mezclamos id con la semilla de tiempo.
	for (int i = 0; i < 7; i++)
	{
		hash = ((hash << 7) | (hash >> 25)) ^ (id * (i + 1));
		hash = hash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}
	return hash;
}

int elegir_rey(int size)
{
	int id_rey = 0;
	uint32_t min_hash = pseudo_hash(0);

	for (int i = 1; i < size; i++)
	{
		uint32_t hash_actual = pseudo_hash(i);
		if (hash_actual < min_hash)
		{
			min_hash = hash_actual;
			id_rey = i;
		}
	}
	return id_rey;
}

int main(int argc, char **argv)
{

	MPI_Init(&argc, &argv);

	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size); // Obtener el número de nodos
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtener el identificador del nodo

	// Cada nodo elige su estrategia aleatoriamente
	srand(time(NULL) + rank);						   // Semilla para la aleatoriedad
	char initial_plan = (rand() % 2 == 0) ? 'A' : 'R'; // A o R como planes
	printf("Plan de %i: %c\n", rank, initial_plan);

	// Generar los nodos traidores
	int *traidores = generarTraidores(size); // Ejemplo de lista de traidores [0,1,0,0,0]

	// Difundir los traidores a todos los nodos
	MPI_Bcast(traidores, size, MPI_INT, 0, MPI_COMM_WORLD);
	// Sincronización de nodos
	MPI_Barrier(MPI_COMM_WORLD);

	// Inicializar las variables y los arreglos para la comunicación
	int es_traidor = (traidores[rank] == 1) ? 1 : 0;
	char *planes = (char *)malloc(size * sizeof(char));
	int valida;
	char plan, result_plan;
	int consenso;
	char *mayority = (char *)malloc(size * sizeof(char));

	// Código adicional relacionado con la comunicación y consenso se agregaría aquí

	// necesitamos hacer un for, vamos a iterar hasta obtener consenso o el número máximo de rondas
	for (int rondas = 0; rondas < RONDAS_MAX && !consenso; rondas++)
	{
		for (int G = 0; G < size; G++)
		{
			char send_plan = (es_traidor == 1) ? (rand() % 2 == 0) ? 'A' : 'R' : initial_plan;
			MPI_Send(&send_plan, 1, MPI_CHAR, G, 0, MPI_COMM_WORLD);
		}

		for (int G = 0; G < size; G++)
		{
			MPI_Recv(&planes[G], 1, MPI_CHAR, G, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		printf("Ronda %d - Planes del nodo %i (traición = %i): ", rondas, rank, es_traidor);
		for (int i = 0; i < size; i++)
		{
			printf("%c", planes[i]);
		}
		printf("\n");

		// Revisamos el consenso.
		valida = es_valida(size, planes);
		if (valida == 1)
		{
			plan = mayoria(size, planes);
			mayority[rank] = plan;
		}
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Gather(&plan, 1, MPI_CHAR, mayority, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

		// MAYORITY está completa
		if (rank == 0)
		{
			for (int i = 1; i < size; i++)
			{
				if (mayority[i] == mayority[0] && mayority[0] != 'O')
				{
					consenso = 1;
				}
				else
				{
					consenso = 0;
					break;
				}

				if (consenso == 0)
				{
					printf("No se ha llegado a un consenso.\n");
				}
				else
				{
					printf("El plan que se llego por votacion es: %c\n", mayority[0]);
				}
			}
		}
		MPI_Bcast(&consenso, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);
		if (consenso == 1)
		{
			break;
		}

		int nodo_rey = elegir_rey(size);
		if (rank == 0)
		{
			printf("El nodo %d ha sido elegido como rey por el algoritmo de hash. \n", nodo_rey);
		}
		char kings_plan = planes[nodo_rey];

		for (int G = 0; G < size; G++)
		{
			char send_plan = (es_traidor == 1) ? (rand() % 2 == 0) ? 'A' : 'R' : kings_plan;
			MPI_Send(&send_plan, 1, MPI_CHAR, G, 0, MPI_COMM_WORLD);
		}

		for (int G = 0; G < size; G++)
		{
			MPI_Recv(&planes[G], 1, MPI_CHAR, G, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		printf("Ronda %d - Planes de nodo %i (traición = %i): ", rondas, rank, es_traidor);
		for (int i = 0; i < size; i++)
		{
			printf("%c ", planes[i]);
		}
		printf("\n");

		// Revisamos consenso
		valida = es_valida(size, planes);
		if (valida == 1)
		{
			plan = mayoria(size, planes);
			mayority[rank] = plan;
		}
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Gather(&plan, 1, MPI_CHAR, mayority, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

		// MAYORITY ya la tenemos completa
		if (rank == 0)
		{
			for (int i = 1; i < size; i++)
			{
				if (mayority[i] == mayority[0] && mayority[0] != 'O')
				{
					consenso = 1;
				}
				else
				{
					consenso = 0;
					break;
				}
			}
			if (consenso == 0)
			{
				printf("No se ha llegado a un consenso\n");
			}
			else
			{
				printf("El plan al que se llegó por votación es: %c\n", mayority[0]);
			}
		}

		MPI_Bcast(&consenso, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);
	}

	// Liberar memoria
	free(planes);
	free(mayority);
	free(traidores);

	MPI_Finalize();
	return 0;
}