# 🏰 Implementación del Algoritmo del Rey con MPI en C

## 📜 Descripción  

Este código implementa el **Algoritmo del Rey** utilizando **MPI (Message Passing Interface)** para simular un sistema distribuido en el que los nodos alcanzan un consenso a pesar de la presencia de nodos traidores.  

El algoritmo tiene un número máximo de rondas para intentar llegar a un acuerdo entre todos los nodos.  

---

## 🏗️ Estructura del Programa  

El programa comienza importando las bibliotecas necesarias como `stdio.h`, `mpi.h`, `time.h`, entre otras, que proporcionan funciones para:  
- Comunicación entre nodos  
- Manejo del tiempo  
- Generación de números aleatorios  
- Manipulación de estructuras de datos  

### 📌 Componentes Principales  

1. **Funciones auxiliares** → Manejan el consenso, la elección del rey y el rol de los traidores.  
2. **Generación de traidores** → Determina aleatoriamente qué nodos serán traidores.  
3. **Algoritmo del Rey** → Intenta alcanzar un consenso usando rondas de votación, con la intervención de un nodo rey en caso de desacuerdo.  

---

## ⚙️ Funciones Implementadas  

### 🏛️ `mayoria(size, planes)`  
Determina el plan (ataque o retirada) que obtuvo la mayoría de votos en una ronda.  

### 🏛️ `es_valida(size, planes)`  
Verifica si una votación es válida. Para que sea válida, un plan debe tener al menos `n/2 + t` votos, donde `t` es el número de nodos traidores.  

### 🏛️ `generarTraidores(size)`  
Genera un arreglo donde cada nodo se marca como **traidor (1) o no (0)**. El número de traidores está determinado por la fórmula:  
\[
\frac{(size - 1)}{4}
\]  
Usa números aleatorios para seleccionarlos de manera dinámica.  

### 🏛️ `pseudo_hash(id)`  
Calcula un **hash pseudoaleatorio** basado en el identificador del nodo y el tiempo actual. Se usa para elegir al **rey** de manera **determinista pero distribuida**.  

### 🏛️ `elegir_rey(size)`  
Elige el nodo rey utilizando la función `pseudo_hash`, seleccionando el nodo con el **valor hash más bajo**. Este nodo toma decisiones finales cuando no hay consenso.  

---

## 🎯 Main: Algoritmo del Rey  

El **main** implementa la lógica del algoritmo distribuido con **MPI**:  

### 🔹 **Inicialización**  
- Se inicializa **MPI** y se obtiene el **rango del nodo** (`rank`) y el **tamaño del grupo** (`size`).  
- Cada nodo elige un **plan inicial** (**A → ataque** o **R → retirada**) de manera aleatoria.  

### 🔹 **Generación de Traidores**  
- Se genera la lista de traidores aleatoriamente.  
- Se difunde a todos los nodos usando **MPI_Bcast**.  

### 🔹 **Rondas de Votación**  
El algoritmo itera hasta un máximo de `RONDAS_MAX`, intentando alcanzar un consenso. En cada ronda:  
1. Cada nodo **envía su plan** a los demás.  
2. Reciben los planes y verifican si hay consenso usando `mayoria` y `es_valida`.  
3. Si **no hay consenso**, se elige un **rey** que impone el plan en la siguiente ronda.  

### 🔹 **Elección del Rey**  
Si no se logra consenso, se elige un **nodo rey** mediante `elegir_rey`. Este nodo impone su plan a los nodos no traidores.  

### 🔹 **Finalización**  
El algoritmo termina cuando:  
- Se alcanza un consenso antes de agotar las rondas.  
- Se completan todas las rondas sin lograr consenso.  

---

## 📡 Comunicación MPI  

El programa usa las siguientes primitivas de **MPI**:  
- `MPI_Send` y `MPI_Recv` → Para enviar y recibir planes entre nodos.  
- `MPI_Bcast` → Para **difundir** el estado de los traidores y el consenso a todos los nodos.  
- `MPI_Gather` → Para **recolectar** los planes finales y verificar si se alcanzó un consenso.  

---

## 🚀 Instalación y Ejecución  

### 📦 Instalación de Bibliotecas  

En sistemas **Debian/Ubuntu**, instala **OpenMPI** con:  
```bash
sudo apt-get install openmpi-bin openmpi-common libopenmpi-dev
