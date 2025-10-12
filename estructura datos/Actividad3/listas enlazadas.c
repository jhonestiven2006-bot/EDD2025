#include <stdio.h>
#include <stdlib.h>

typedef struct Pasajero {
    char tipoDocumento[3];
    char apellido[50];
    struct Pasajero* siguiente;
} Pasajero;

Pasajero* cabeza = 0;
int capacidad = 30;
int vendidos = 0;

void registrarPasajero() {
    int limite = capacidad + (capacidad / 30);
    if (vendidos >= limite) {
        printf("Ya se vendieron todos los tiquetes.\n");
        return;
    }

    Pasajero* nuevo = (Pasajero*)malloc(sizeof(struct Pasajero));
    if (nuevo == 0) {
        printf("No se pudo asignar memoria.\n");
        return;
    }

    printf("Documento (CC/TI/PA): ");
    scanf("%2s", nuevo->tipoDocumento);
    getchar();

    printf("Apellido: ");
    fgets(nuevo->apellido, 50, stdin);

    for (int i = 0; nuevo->apellido[i] != '\0'; i++) {
        if (nuevo->apellido[i] == '\n') {
            nuevo->apellido[i] = '\0';
            break;
        }
    }

    nuevo->siguiente = 0;

    if (cabeza == 0) {
        cabeza = nuevo;
    } else {
        Pasajero* actual = cabeza;
        while (actual->siguiente != 0) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevo;
    }

    vendidos++;
    printf("Registrado. Vendidos: %d/%d\n", vendidos, limite);
}

void mostrarPasajeros() {
    Pasajero* actual = cabeza;
    int i = 1;
    while (actual != 0) {
        printf("%d. %s - %s\n", i, actual->tipoDocumento, actual->apellido);
        actual = actual->siguiente;
        i++;
    }
}

void liberarMemoria() {
    Pasajero* actual = cabeza;
    while (actual != 0) {
        Pasajero* temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
}

int main() {
    int limite = capacidad + (capacidad / 30);
    printf("Capacidad del vuelo: %d pasajeros\n", capacidad);
    printf("Tiquetes disponibles (con 10%% de overbooking): %d\n", limite);

    int opcion;
    do {
        printf("\n1. Registrar\n2. Mostrar\n3. Salir\nOpción: ");
        scanf("%d", &opcion);

        if (opcion == 1) registrarPasajero();
        else if (opcion == 2) mostrarPasajeros();
        else if (opcion == 3) liberarMemoria();
        else printf("Opción inválida.\n");

    } while (opcion != 3);

    return 0;
}