#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// estructuras
typedef struct Pedido {
    char destino[50];
    int cantidad;
    struct Pedido* sig;
} Pedido;

typedef struct NodoAVL {
    int fecha;              // AAAAMMDD
    int stock;
    char producto[30];
    int altura;
    Pedido* cola;
    struct NodoAVL* izq;
    struct NodoAVL* der;
} NodoAVL;

// utilidades de entrada segura
void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}
void leerLinea(char* buf, size_t n) {
    if (fgets(buf, (int)n, stdin)) {
        buf[strcspn(buf, "\n")] = 0;
    } else {
        buf[0] = 0;
    }
}
int leerEntero(const char* prompt) {
    int x;
    printf("%s", prompt);
    while (scanf("%d", &x) != 1) {
        printf("Entrada inválida. Intente de nuevo.\n");
        limpiarBuffer();
        printf("%s", prompt);
    }
    limpiarBuffer();
    return x;
}
int validarFechaAAAAMMDD(int f) {
    // Rango básico y despiece
    if (f < 19000101 || f > 29991231) return 0;
    int yyyy = f / 10000;
    int mm   = (f / 100) % 100;
    int dd   = f % 100;
    if (mm < 1 || mm > 12) return 0;
    int diasMes[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    // Ajuste por año bisiesto
    int bisiesto = (yyyy%4==0 && (yyyy%100!=0 || yyyy%400==0));
    if (bisiesto && mm == 2) diasMes[2] = 29;
    if (dd < 1 || dd > diasMes[mm]) return 0;
    return 1;
}

// utilidades AVL
int max(int a, int b) { return (a > b) ? a : b; }
int altura(NodoAVL* n) { return n ? n->altura : 0; }
int balance(NodoAVL* n) { return n ? altura(n->izq) - altura(n->der) : 0; }

// rotaciones
NodoAVL* rotDer(NodoAVL* y) {
    NodoAVL* x = y->izq;
    NodoAVL* t2 = x->der;
    x->der = y;
    y->izq = t2;
    y->altura = max(altura(y->izq), altura(y->der)) + 1;
    x->altura = max(altura(x->izq), altura(x->der)) + 1;
    return x;
}
NodoAVL* rotIzq(NodoAVL* x) {
    NodoAVL* y = x->der;
    NodoAVL* t2 = y->izq;
    y->izq = x;
    x->der = t2;
    x->altura = max(altura(x->izq), altura(x->der)) + 1;
    y->altura = max(altura(y->izq), altura(y->der)) + 1;
    return y;
}

// crear nodo
NodoAVL* nuevoNodo(int fecha, const char* prod, int stock) {
    NodoAVL* n = (NodoAVL*)malloc(sizeof(NodoAVL));
    if (!n) { printf("Error de memoria.\n"); exit(1); }
    n->fecha = fecha;
    n->stock = stock;
    strncpy(n->producto, prod, sizeof(n->producto)-1);
    n->producto[sizeof(n->producto)-1] = 0;
    n->altura = 1;
    n->cola = NULL;
    n->izq = n->der = NULL;
    return n;
}

// insertar AVL
NodoAVL* insertar(NodoAVL* raiz, int fecha, const char* prod, int stock) {
    if (!raiz) return nuevoNodo(fecha, prod, stock);
    if (fecha < raiz->fecha) raiz->izq = insertar(raiz->izq, fecha, prod, stock);
    else if (fecha > raiz->fecha) raiz->der = insertar(raiz->der, fecha, prod, stock);
    else {
        printf("Fecha ya existe. No se inserta el lote.\n");
        return raiz;
    }
    raiz->altura = 1 + max(altura(raiz->izq), altura(raiz->der));
    int bf = balance(raiz);
    if (bf > 1 && fecha < raiz->izq->fecha) return rotDer(raiz);           // LL
    if (bf < -1 && fecha > raiz->der->fecha) return rotIzq(raiz);          // RR
    if (bf > 1 && fecha > raiz->izq->fecha) {                              // LR
        raiz->izq = rotIzq(raiz->izq);
        return rotDer(raiz);
    }
    if (bf < -1 && fecha < raiz->der->fecha) {                             // RL
        raiz->der = rotDer(raiz->der);
        return rotIzq(raiz);
    }
    return raiz;
}

// cola FIFO
void encolar(Pedido** cabeza, const char* destino, int cant) {
    Pedido* nuevo = (Pedido*)malloc(sizeof(Pedido));
    if (!nuevo) { printf("Error de memoria.\n"); exit(1); }
    strncpy(nuevo->destino, destino, sizeof(nuevo->destino)-1);
    nuevo->destino[sizeof(nuevo->destino)-1] = 0;
    nuevo->cantidad = cant;
    nuevo->sig = NULL;
    if (*cabeza == NULL) *cabeza = nuevo;
    else {
        Pedido* p = *cabeza;
        while (p->sig) p = p->sig;
        p->sig = nuevo;
    }
}
int contarPedidos(Pedido* cabeza) {
    int c = 0;
    while (cabeza) { c++; cabeza = cabeza->sig; }
    return c;
}
void liberarCola(Pedido* cabeza) {
    while (cabeza) {
        Pedido* tmp = cabeza->sig;
        free(cabeza);
        cabeza = tmp;
    }
}

// mínimo nodo
NodoAVL* minimo(NodoAVL* raiz) {
    NodoAVL* cur = raiz;
    while (cur && cur->izq) cur = cur->izq;
    return cur;
}

// registrar pedido (siempre al lote con fecha mínima con stock)
void registrarPedido(NodoAVL* raiz) {
    if (!raiz) { printf("Inventario vacío.\n"); return; }
    // Buscar mínimo con stock > 0
    NodoAVL* cur = raiz;
    NodoAVL* lote = NULL;
    while (cur) {
        if (cur->izq) { cur = cur->izq; continue; }
        // visitar in-order sin recursión: más simple, usamos minimo directamente
        lote = minimo(raiz);
        break;
    }
    if (!lote || lote->stock <= 0) { printf("No hay stock disponible.\n"); return; }

    char destino[50];
    int cant = leerEntero("Cantidad a despachar: ");
    printf("Destino: "); leerLinea(destino, sizeof(destino));

    if (cant <= 0) { printf("Cantidad inválida.\n"); return; }
    if (cant > lote->stock) { printf("Stock insuficiente en el lote %d.\n", lote->fecha); return; }

    encolar(&lote->cola, destino, cant);
    lote->stock -= cant;
    printf("Pedido registrado en lote %d.\n", lote->fecha);
}

// cancelar pedido específico y devolver stock al lote mínimo
int cancelarPedido(Pedido** cabeza, const char* destino, int cant) {
    Pedido* ant = NULL;
    Pedido* act = *cabeza;
    while (act) {
        if (strcmp(act->destino, destino) == 0 && act->cantidad == cant) {
            if (ant) ant->sig = act->sig;
            else *cabeza = act->sig;
            free(act);
            return 1;
        }
        ant = act;
        act = act->sig;
    }
    return 0;
}

// eliminar nodo AVL (baja de producto)
NodoAVL* eliminar(NodoAVL* raiz, int fecha) {
    if (!raiz) return NULL;
    if (fecha < raiz->fecha) raiz->izq = eliminar(raiz->izq, fecha);
    else if (fecha > raiz->fecha) raiz->der = eliminar(raiz->der, fecha);
    else {
        liberarCola(raiz->cola);
        if (!raiz->izq || !raiz->der) {
            NodoAVL* tmp = raiz->izq ? raiz->izq : raiz->der;
            free(raiz);
            return tmp;
        } else {
            NodoAVL* succ = minimo(raiz->der);
            raiz->fecha = succ->fecha;
            raiz->stock = succ->stock;
            strncpy(raiz->producto, succ->producto, sizeof(raiz->producto)-1);
            raiz->producto[sizeof(raiz->producto)-1] = 0;
            raiz->cola = succ->cola;
            succ->cola = NULL;
            raiz->der = eliminar(raiz->der, succ->fecha);
        }
    }
    if (!raiz) return NULL;
    raiz->altura = 1 + max(altura(raiz->izq), altura(raiz->der));
    int bf = balance(raiz);
    if (bf > 1 && balance(raiz->izq) >= 0) return rotDer(raiz);
    if (bf > 1 && balance(raiz->izq) < 0) {
        raiz->izq = rotIzq(raiz->izq);
        return rotDer(raiz);
    }
    if (bf < -1 && balance(raiz->der) <= 0) return rotIzq(raiz);
    if (bf < -1 && balance(raiz->der) > 0) {
        raiz->der = rotDer(raiz->der);
        return rotIzq(raiz);
    }
    return raiz;
}

// reporte in-order
void reporte(NodoAVL* raiz) {
    if (!raiz) return;
    reporte(raiz->izq);
    printf("Fecha: %d | Prod: %s | Stock: %d | Pedidos: %d\n",
           raiz->fecha, raiz->producto, raiz->stock, contarPedidos(raiz->cola));
    Pedido* p = raiz->cola;
    while (p) {
        printf("   -> Destino: %s | Cantidad: %d\n", p->destino, p->cantidad);
        p = p->sig;
    }
    reporte(raiz->der);
}

// liberar árbol
void liberarArbol(NodoAVL* raiz) {
    if (!raiz) return;
    liberarArbol(raiz->izq);
    liberarArbol(raiz->der);
    liberarCola(raiz->cola);
    free(raiz);
}

// menú
void menu() {
    printf("\n--- MENU ---\n");
    printf("1. Recepción de Mercancía (Insertar en AVL)\n");
    printf("2. Registrar Pedido de Despacho (Encolar en FIFO)\n");
    printf("3. Cancelar Pedido y devolver stock\n");
    printf("4. Baja de Producto (Eliminar lote)\n");
    printf("5. Reporte de Estado (In-Order)\n");
    printf("0. Salir\n");
    printf("Opcion: ");
}

int main() {
    NodoAVL* raiz = NULL;
    int op;
    do {
        menu();
        op = leerEntero("");
        if (op == 1) {
            int fecha = leerEntero("Fecha (AAAAMMDD): ");
            if (!validarFechaAAAAMMDD(fecha)) {
                printf("Fecha inválida. Use formato AAAAMMDD válido.\n");
                continue;
            }
            int cant = leerEntero("Cantidad: ");
            if (cant <= 0) { printf("Cantidad inválida.\n"); continue; }
            char prod[30];
            printf("Producto: "); leerLinea(prod, sizeof(prod));
            raiz = insertar(raiz, fecha, prod, cant);
        } else if (op == 2) {
            registrarPedido(raiz);
        } else if (op == 3) {
            if (!raiz) { printf("Inventario vacío.\n"); continue; }
            char destino[50]; int cant;
            printf("Destino a cancelar: "); leerLinea(destino, sizeof(destino));
            cant = leerEntero("Cantidad: ");
            // Para simplicidad: cancelar en la cola del mínimo (mismo criterio de despacho)
            NodoAVL* lote = minimo(raiz);
            if (lote && cancelarPedido(&lote->cola, destino, cant)) {
                lote->stock += cant; // devolver stock
                printf("Pedido cancelado y stock restablecido en lote %d.\n", lote->fecha);
            } else {
                printf("No se encontró el pedido en el lote más antiguo.\n");
            }
        } else if (op == 4) {
            int fecha = leerEntero("Fecha a eliminar (AAAAMMDD): ");
            raiz = eliminar(raiz, fecha);
            printf("Si existía, el lote fue eliminado y el árbol re-balanceado.\n");
        } else if (op == 5) {
            reporte(raiz);
        } else if (op != 0) {
            printf("Opción inválida.\n");
        }
    } while (op != 0);

    liberarArbol(raiz);
    return 0;
}
