#include "buscaRyP.hh"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <stdlib.h>
#include <algorithm>


using namespace std;

void abrir_fichero_lectura(string nombre_fichero, ifstream& f_in){
    f_in.open(nombre_fichero);
    if (!f_in.is_open()){
        cout << "Error al abrir el fichero \"" << nombre_fichero << "\" para su lectura" << endl;
        exit(1);
    }
}

void abrir_fichero_escritura(string nombre_fichero, ofstream& f_out){
    f_out.open(nombre_fichero);
    if (!f_out.is_open()){
        cout << "Error al abrir el fichero \"" << nombre_fichero << "\" para su escritura" << endl;
        exit(1);
    }
}

void leer_pagina(ifstream& f_in, Pagina& pagina){

    string s_num_articulos, s_ancho_pag, s_alto_pag;
    string x_articulo, y_articulo, ancho_articulo, alto_articulo;

    getline(f_in, s_num_articulos, ' ');
    getline(f_in, s_ancho_pag, ' ');
    getline(f_in, s_alto_pag);

    int num_articulos = stoi(s_num_articulos);
    int ancho_pag = stoi(s_ancho_pag);
    int alto_pag = stoi(s_alto_pag);

    vector<Articulo> v_articulos;

    for (int i = 0; i < num_articulos; i++){
        
        getline(f_in, ancho_articulo, ' ');
        getline(f_in, alto_articulo, ' ');
        getline(f_in, x_articulo, ' ');
        getline(f_in, y_articulo);

        v_articulos.push_back(Articulo(i, stoi(ancho_articulo), stoi(alto_articulo), stoi(x_articulo), stoi(y_articulo)));

    }

    pagina.num_articulos = num_articulos;
    pagina.ancho = ancho_pag;
    pagina.alto = alto_pag;
    pagina.actualizar_area();
    pagina.articulos = v_articulos;
}

void imprimir_solucion(ofstream& f_out, int num_pag, int area_solucion, vector<Articulo> art_solucion, long double tiempo_ejecucion){
    f_out << endl;

    f_out << "Pagina " << num_pag << endl << "\tTiempo: " << tiempo_ejecucion << " ms" << endl << "\tArea: " << area_solucion << " mm" << endl << "\tArticulos:";
    for(Articulo art : art_solucion){
        f_out << endl << "\t\t";
        f_out << art.ancho << " " << art.alto << " " << art.x << " " << art.y << " (Area: " << art.area << " mm)";
    }

    f_out << endl;
}

void imprimir_solucion_por_pantalla(int num_pag, int area_solucion, vector<Articulo> art_solucion, long double tiempo_ejecucion){
    cout << endl;

    cout << "Pagina " << num_pag << endl << "\tTiempo: " << tiempo_ejecucion << " ms" << endl << "\tArea: " << area_solucion << " mm" << endl << "\tArticulos:";
    for(Articulo art : art_solucion){
        cout << endl << "\t\t";
        cout << art.ancho << " " << art.alto << " " << art.x << " " << art.y << " (Area: " << art.area << " mm)";
    }

    cout << endl;
}



int obtener_composicion_optima(Pagina& pagina, vector<Articulo>& articulos_optimos){
    priority_queue<Node*, vector<Node*>, CompareNodes> cola_nodos;
    vector<Articulo> articulos_insertados;
    int area_minima = pagina.area;

    Node* raiz = new Node(articulos_insertados, "", pagina.area);
    raiz->nivel = 0;
    raiz->f_estim = calcular_func_estimacion(raiz, pagina); 
    cola_nodos.push(raiz);

    //bool terminar = false;

    while(!cola_nodos.empty()){
        // Extraemos el nodo mínimo de la cola 
        Node* nodo_a_expandir = cola_nodos.top();
        cola_nodos.pop();

        // Expandimos el nodo mínimo
        // calcula hijos (con su f_estim y nivel_padre+1), los mete en la cola si <= U y actualiza la solución si <= U && ultimo nivel
        if(nodo_a_expandir->nivel < pagina.num_articulos){
            expandir_nodos_hijos(nodo_a_expandir, pagina, nodo_a_expandir->articulos, cola_nodos, area_minima, articulos_optimos); 
        }

        /*
        bool todos_mayores_que_sol = true;
        for (auto& nodo : cola_nodos) {
            if(nodo->f_estim < area_minima){
                todos_mayores_que_sol = false;
                break;
            }
        }
        
        if(cola_nodos.empty() || todos_mayores_que_sol) {
            terminar = true;
        }
        */
    }
    cout << "COLA NODOS EMPTY" << endl;
    return area_minima;
}

// Comprueba si hay intersección entre el artículo actual y los articulos colocados anteriormente
bool hay_interseccion_con_sig_articulo(const vector<Articulo>& articulos_actuales, const Articulo& sig_articulo) {
    const Articulo& articulo_actual = sig_articulo;

    for (int i = 0; i < (int)articulos_actuales.size(); ++i) {
        const Articulo& articulo_anterior = articulos_actuales[i];
        
        // Comprueba si hay intersección entre los artículos
        if (hay_interseccion_entre_pareja_articulos(articulo_actual, articulo_anterior)) {
            return true;
        }
    }
    return false;
}

void expandir_nodos_hijos(Node* nodo_a_expandir, Pagina pagina, vector<Articulo> art_insertados, priority_queue<Node*, 
                          vector<Node*>, CompareNodes> &cola_nodos, int& area_minima, vector<Articulo>& articulos_optimos){
    Node n = *nodo_a_expandir; 
    cout << "Nodo a expandir: " << n << endl;
    

    // si no interseca, EXPANDE EL HIJO IZQUIERDO

    if(!hay_interseccion_con_sig_articulo(art_insertados, pagina.articulos[nodo_a_expandir->nivel])){ 
        art_insertados.push_back(pagina.articulos[nodo_a_expandir->nivel]);

        // si es el primero, nos saltamos el primer "-" en el id
        if(nodo_a_expandir->nivel == 0){
            nodo_a_expandir->left = new Node(art_insertados, nodo_a_expandir->id + to_string(pagina.articulos[nodo_a_expandir->nivel].id), pagina.area);
        } else {
            nodo_a_expandir->left = new Node(art_insertados, nodo_a_expandir->id + "-" + to_string(pagina.articulos[nodo_a_expandir->nivel].id), pagina.area);
        }

        nodo_a_expandir->left->nivel = nodo_a_expandir->nivel + 1;
        nodo_a_expandir->left->f_estim = calcular_func_estimacion(nodo_a_expandir->left, pagina);

        cout << "Func_estim izq: " << nodo_a_expandir->left->f_estim << " vs area minima " << area_minima << endl;

        if(nodo_a_expandir->left->f_estim <= area_minima && nodo_a_expandir->left->f_estim >= 0){
            cout << "Metido izquierdo:";
            Node n2 = *nodo_a_expandir->left;
            cout << n2;
            cola_nodos.push(nodo_a_expandir->left);

            if(nodo_a_expandir->left->nivel == pagina.num_articulos){
                area_minima = nodo_a_expandir->left->area_sin_ocupar;
                articulos_optimos = nodo_a_expandir->left->articulos;
            }
        }

        art_insertados.pop_back();
    }
    
    // EXPANDE EL HIJO DERECHO

    // si es el primero, nos saltamos el primer "-" en el id
    if(nodo_a_expandir->nivel == 0){
        nodo_a_expandir->right = new Node(art_insertados, nodo_a_expandir->id + "*", pagina.area); // no metemos el articulo nuevo
    } else {
        nodo_a_expandir->right = new Node(art_insertados, nodo_a_expandir->id + "-*", pagina.area); // no metemos el articulo nuevo
    }

    nodo_a_expandir->right->nivel = nodo_a_expandir->nivel + 1;
    nodo_a_expandir->right->f_estim = calcular_func_estimacion(nodo_a_expandir->right, pagina);

    cout << "Func_estim dcha: " << nodo_a_expandir->right->f_estim << " vs area minima " << area_minima << endl;
    if(nodo_a_expandir->right->f_estim <= area_minima && nodo_a_expandir->right->f_estim >= 0){
        cout << "Metido derecho:";
        Node n1 = *nodo_a_expandir->right;
        cout << n1;
        cola_nodos.push(nodo_a_expandir->right);

        if(nodo_a_expandir->right->nivel == pagina.num_articulos){
            area_minima = nodo_a_expandir->right->area_sin_ocupar;
            articulos_optimos = nodo_a_expandir->right->articulos;
        }
    }
}

void calcular_partes(const vector<Articulo>& articulos_actuales, vector<vector<Articulo>>& partes){
    for (int i = 0; i < (int)articulos_actuales.size(); i++)
    {
        vector<Articulo> empty;
        partes.push_back( empty );

        vector< vector<Articulo> > subset_temp = partes;  //making a copy of given 2-d vector.
        
        for (int j = 0; j < (int)subset_temp.size(); j++)
            subset_temp[j].push_back( articulos_actuales[i] );   // adding set[i] element to each subset of subsetTemp. like adding {2}(in 2nd iteration  to {{},{1}} which gives {{2},{1,2}}.
        
        for (int j = 0; j < (int)subset_temp.size(); j++)
            partes.push_back( subset_temp[j] );  //now adding modified subsetTemp to original subset (before{{},{1}} , after{{},{1},{2},{1,2}})
    }

    sort(partes.begin(), partes.end());
    partes.erase( unique(partes.begin(), partes.end() ), partes.end());
}

bool hay_interseccion_entre_pareja_articulos(Articulo a, Articulo b){
    return  (a.x < b.x + b.ancho &&
            a.x + a.ancho > b.x &&
            a.y < b.y + b.alto &&
            a.y + a.alto > b.y);
}

bool intersecan_todos_articulos(const vector<Articulo>& articulos) {
    for(int i = 0; i < (int)articulos.size(); i++){
        for(int j = i+1; j < (int)articulos.size(); j++){
            if(!hay_interseccion_entre_pareja_articulos(articulos[i], articulos[j])){
                return false;
            }
        }
    }
    return true;
}

Articulo calcular_articulo_interseccion_pareja(Articulo a, Articulo b){

    // guardamos los puntos derecha-inferior de ambos articulos por comodidad
    int x_dcha_a = a.x + a.ancho;
    int y_inf_a = a.y + a.alto;

    int x_dcha_b = b.x + b.ancho;
    int y_inf_b = b.y + b.alto;

    int res_x = max(a.x, b.x);
    int res_y = max(a.y, b.y);
    int res_ancho = min(x_dcha_a, x_dcha_b) - res_x;
    int res_alto = min(y_inf_a, y_inf_b) - res_y;

    return Articulo(0, res_ancho, res_alto, res_x, res_y);
}

int calcular_area_articulos_solapados(const vector<Articulo>& articulos) {
    if(articulos.size() == 0){
        return 0;
    }
    if(articulos.size() == 1){
        return articulos[0].area;
    }

    if(!intersecan_todos_articulos(articulos)){
        return 0;
    }

    Articulo interseccion = articulos[0];
    for(int i = 1; i < (int)articulos.size(); i++){
        interseccion = calcular_articulo_interseccion_pareja(interseccion, articulos[i]);
    }

    return interseccion.area;
}

/**
 * @brief 
 * Calcula el área actual ocupada por los articulos pasados por parámetro
 * 
 * @param articulos 
 * @return int 
 */
int calcular_area(const vector<Articulo>& articulos) {
    int area_total = 0;

    vector<vector<Articulo>> partes;
    calcular_partes(articulos, partes);

    for(vector<Articulo> conjunto : partes){

        int area_interseccion = calcular_area_articulos_solapados(conjunto);
        if(conjunto.size() % 2 == 0){
            area_total -= area_interseccion;
        } else {
            area_total += area_interseccion;
        }
    }

    return area_total;
}

/**
 * @brief 
 * 
 * @param art_en_pagina: Articulos ya colocados en la pagina, representarían una solución parcial
 * @return int 
 */
int calcular_area_articulos_sin_solapar(vector<Articulo> arts_en_pagina){
    int area_ocupada = 0;
    for (auto& articulo : arts_en_pagina){
        area_ocupada = area_ocupada + articulo.area;
    }
    return area_ocupada;
}

/**
 * @brief 
 * 
 *  Función que se encarga de calcular el area compuesta por la suma de aquellos articulos 
 *  restantes por añadir, estos articulos no deben solaparse con los articulos actuales añadidos,
 *  sin embargo, pueden solaparse entre ellos.
 * 
 * @param pagina: Página que contiene los articulos
 * @param arts_actuales: Vector que contiene los articulos añadidos hasta el momento
 * @return int 
 */
int area_restante_posible_maxima(const Pagina& pagina, vector<Articulo> arts_actuales, int nivel){

    // Debemos comprobar que los articulos que faltan por añadir no solapen con los que tenemos,
    // sin embargo, no hace falta comprobar que solapen entre ellos (entre los restantes).
    // Esto se consigue iterando y comprobando que los articulos restantes no solapen con los actuales.

    int arts_ya_anadidos = arts_actuales.size();
    //int area_restante_posible_max = 0;
    vector<Articulo> articulos_restantes;

    // Obtenemos los articulos restantes que no se solapan con los actuales
    for (int i = nivel; i < pagina.num_articulos; i++){     // Nodo de nivel 3 valora el artículo 3, que está en la pos. 2 del array pagina.articulos.
                                                        // Como queremos el siguiente artículo, dejamos i = nivel porque será el de la pos. 3 del array
        
        if(!hay_interseccion_con_sig_articulo(arts_actuales, pagina.articulos[i])){
            articulos_restantes.push_back(pagina.articulos[i]); 
        }
    }


    cout << "Hay " << arts_ya_anadidos << " articulos anadidos" << endl;
    cout << "Hay " << pagina.num_articulos - nivel << " articulos restantes" << endl;

    int cont = 0;
    for(Articulo a : arts_actuales){
        cout << "Art actual " << cont << ": " << a << endl;
        cont++;
    }

    return calcular_area_articulos_solapados(articulos_restantes);

    /*

    if(arts_actuales.size() != 0 && articulos_restantes.size() != 0){
        // Hacemos la comprobación con los articulos actuales 
        for(int i = 0; i < (int)articulos_restantes.size(); i++){
            for(int j = 0; j < (int)arts_actuales.size(); j++){
                //cout << "Comparo " << arts_actuales[j].id << " con " << articulos_restantes[i].id << endl;
                if(!hay_interseccion_entre_pareja_articulos(arts_actuales[j], articulos_restantes[i])){
                    area_restante_posible_max = area_restante_posible_max + articulos_restantes[i].area;
                }
            }
        }
    }
    return area_restante_posible_max;
    */
}

/**
 * @brief 
 * 
 * @param nodo: Nodo del que se va a calcular su función de estimación
 * @param pagina: Página que contiene los articulos
 * @return int 
 */
int calcular_func_estimacion(Node* nodo, Pagina pagina){
    // Area ocupada por los articulos ya colocados (solución parcial del nodo)
    int area_ocupada_actual = calcular_area_articulos_sin_solapar(nodo->articulos);
    // Area posible restante 
    cout << endl << "============ CALCULANDO FUNC ESTIMACION DE \"" << nodo->id << "\" ============" << endl;
    int area_heuristica = area_restante_posible_maxima(pagina, nodo->articulos, nodo->nivel);
    cout << "Area total: " << pagina.area << " , Area ocupada: " << area_ocupada_actual << ", Area heuristica: " << area_heuristica 
         <<  ", Valor de funcion de estimacion: " << pagina.area - area_ocupada_actual - area_heuristica << endl;
    cout << "=================================================================" << endl << endl;

    return pagina.area - area_ocupada_actual - area_heuristica;
}

int main(int argc, char *argv[]){
    double tiempo_ejecucion = 0.0;
    ifstream f_in;
    ofstream f_out;
    int num_pag = 1;
    int area_solucion = 0;
    vector<Articulo> articulos_solucion = {};

    if (argc < 3){
        cout << "ERROR: Numero de parametros invalido" << endl;
        exit(1);
    }

    abrir_fichero_lectura(argv[1], f_in);
    abrir_fichero_escritura(argv[2], f_out);

    while(!f_in.eof()){
        Pagina pagina;
        leer_pagina(f_in, pagina);

        auto start_time = chrono::high_resolution_clock::now();
        area_solucion = obtener_composicion_optima(pagina, articulos_solucion);
        auto end_time = chrono::high_resolution_clock::now();
        
        auto duracion = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time) / 1000000.0;
        tiempo_ejecucion = duracion.count();

        imprimir_solucion(f_out, num_pag, area_solucion, articulos_solucion, tiempo_ejecucion);
        imprimir_solucion_por_pantalla(num_pag, area_solucion, articulos_solucion, tiempo_ejecucion);
        num_pag++;
    }
    
    f_in.close();
    f_out.close();
    return 0;
}