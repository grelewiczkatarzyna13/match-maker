#include <iostream>
using namespace std;

template <typename T>
class Vector {
    private:
        T *dataArray; //dynamiczna tablica w pamięci
        int currentCapacity;
        int currentSize;

        //powiększanie wielkości tablicy
        void reserve(int newCapacity) {
            T *newData = new T[newCapacity];

            for (int i = 0; i < currentSize; i++) {
                newData[i] = dataArray[i];
            }

            delete[] dataArray;
            dataArray = newData;
            currentCapacity = newCapacity;
        }
    
    public:
        //konstruktor, pusty wektor
        Vector() : dataArray(nullptr), currentCapacity(0), currentSize(0) {}

        Vector(int initialSize, const T &defaultValue = T()) : dataArray(nullptr), currentCapacity(0),currentSize(0) {
            resize(initialSize, defaultValue);
        }

        ~Vector() {
            delete[] dataArray;
        }
        //konstruktor kopiujący
        Vector(const Vector &other) {
            currentCapacity = other.currentCapacity;
            currentSize = other.currentSize;
            if (currentCapacity > 0) {
                dataArray = new T[currentCapacity];
                for (int i = 0; i < currentSize; i++) {
                    dataArray[i] = other.dataArray[i];
                }
            } else dataArray = nullptr;
        }
        //operator przypisania
        Vector &operator=(const Vector &other) {
            if (this != &other) {
                delete[] dataArray;
                currentCapacity = other.currentCapacity;
                currentSize = other.currentSize;
                if (currentCapacity > 0) {
                    dataArray = new T[currentCapacity];
                    for (int i = 0; i < currentSize; i++) {
                        dataArray[i] = other.dataArray[i];
                    }
                } else dataArray = nullptr;
            }
            return *this;
        }

        //dodawanie elementu na koniec wektora
        void push_back(const T &value) {
            if (currentSize == currentCapacity) {
                int newCapacity = (currentCapacity == 0) ? 1 : currentCapacity * 2;
                reserve(newCapacity);
            }
            dataArray[currentSize] = value;
            currentSize++;
        }

        int size() const {
            return currentSize;
        }

        void clear() {
            currentSize = 0;
        }

        void resize(int newSize, const T &defaultValue = T()) {
            if (newSize > currentCapacity) {
                reserve(newSize == 0 ? 1 : newSize * 2);
            }
            //wartość domyślna w reszcie powiekszonych komórek
            for (int i = currentSize; i < newSize; i++) {
                dataArray[i] = defaultValue;
            }
            currentSize = newSize;
        }
        //operator dostępu
        T &operator[](int index) {
            return dataArray[index];
        }
        //wersja do odczytu
        const T &operator[](int index) const {
            return dataArray[index];
        }
};

struct Edge {
    int to; //numer wierzcholka, do której prowadzi krawędź
    int weight; //waga relacji
};

//MCMF ALGORITHM (for bipartite graphs)
struct FlowEdge {
    int to;
    int capacity; //=1 bo każdy tworzy jedną relację
    int flow; //ile wody płynie przez rurę
    int cost; //waga relacji
    int revIdx; //indeks krawedzi zwrotnej
};

struct VertexOrder {
    int id;
    int maxEdgeWeight;
};

void sortEdgesDescending(Vector<Edge> &edges) {
    int size = edges.size();
    for (int i = 1; i < size; i++) {
        Edge key = edges[i];
        int j = i - 1;
        while (j >= 0 && edges[j].weight < key.weight) {
            edges[j + 1] = edges[j];
            j--;
        }
        edges[j + 1] = key;
    }
}

//BFS ALGORITHM
bool checkIsBipartite(int totalVertices, const Vector<Vector<Edge>> &adjList, Vector<int> &vertexColors) {
    //domyślnie osoba nie ma koloru -1
    vertexColors.resize(totalVertices + 1, -1);

    for (int startVertex = 1; startVertex <= totalVertices; startVertex++) {
        if (vertexColors[startVertex] == -1) {
            Vector<int> queue;
            queue.push_back(startVertex);

            vertexColors[startVertex] = 0; //kolor pierwszej osoby
            int head = 0; //który element z wektora jest aktualnie obsługiwany

            while (head < queue.size()) {
                int current = queue[head];
                head++;

                //przeglądamy wszytskich sąsiadów danej osoby
                for (int i = 0; i < adjList[current].size(); i++) {
                    int neighbor = adjList[current][i].to;

                    //sąsiad nie ma przypisanej grupy
                    if (vertexColors[neighbor] == -1) {
                        vertexColors[neighbor] = 1 - vertexColors[current]; //kolor przeciwny do sąsiada
                        queue.push_back(neighbor);
                    }
                    //ma grupę taka samą jak current
                    else if (vertexColors[neighbor] == vertexColors[current]) {
                        return false; //graf nie jest dwudzielny
                    }
                }
            }
        }
    }
    return true;
}

//tworzenie rur w przód i tył między wierzchołkami
void addFlowEdge(Vector<Vector<FlowEdge>> &flowAdj, int from, int to, int capacity, int cost) {
    FlowEdge forwardEdge = {to, capacity, 0, cost, flowAdj[to].size()};
    FlowEdge backwardEdge = {from, 0, 0, -cost, flowAdj[from].size()};

    //wrzucamy rury do sieci przepływowej
    flowAdj[from].push_back(forwardEdge);
    flowAdj[to].push_back(backwardEdge);
}

int solveBipartiteMCMF(int totalVertices, const Vector<Vector<Edge>> &adjList, const Vector<int> &vertexColors) {
    int sourceNode = 0;
    int targetNode = totalVertices + 1;
    int totalNodes = totalVertices + 2; //wierzchołki +2 bramki

    //nowa lista sąsiedztwa dla rur przepływowych
    Vector<Vector<FlowEdge>> flowAdj;
    flowAdj.resize(totalNodes);

    //budowa relacji i rur w pamięci
    for (int u = 1; u <= totalVertices; u++) {
        if (vertexColors[u] == 0) { //jeżeli to pan
            //bramka wejściowa z panem u
            addFlowEdge(flowAdj, sourceNode, u, 1, 0);

            //panie akceptowane przez pana
            for (int i = 0; i < adjList[u].size(); i++) {
                int v = adjList[u][i].to;
                int weight = adjList[u][i].weight;
                addFlowEdge(flowAdj, u, v, 1, weight);
            }
        } else { //to pani, łączymy ją z bramką wyjściową
            addFlowEdge(flowAdj, u, targetNode, 1, 0);
        }
    }

    //algorytm główny
    int maximumWeightResult = 0;

    Vector<int> distanceArray(totalNodes); //rekordowe punkty zdobyte po drodze
    Vector<int> parentVertex(totalNodes); 
    Vector<int> parentEdgeIdx(totalNodes); //numer rury przez którą przeszedł
    Vector<bool> inQueue(totalNodes); //kolejka do przejścia
    Vector<int> processQueue; //kolejka BFS

    while (true) {
        //reset 
        for (int i = 0; i < totalNodes; i++) {
            distanceArray[i] = -2000000000;
            inQueue[i] = false;
        }
        processQueue.clear();
        //start z 0 punktami
        distanceArray[sourceNode] = 0;
        processQueue.push_back(sourceNode);
        inQueue[sourceNode] = true;

        int head = 0;
        while (head < processQueue.size()) {
            int u = processQueue[head];
            head++;
            inQueue[u] = false;

            //sprawdzenie rur wychodzących z u
            for (int i = 0; i < flowAdj[u].size(); i++) {
                const FlowEdge &edge = flowAdj[u][i];

                //w rurze musi być miejsce oraz dawać więcej pkt niż rekord dotychczas
                if (edge.capacity - edge.flow > 0 && distanceArray[edge.to] < distanceArray[u] + edge.cost) {
                    //poprawa rekordu pkt sąsiada
                    distanceArray[edge.to] = distanceArray[u] + edge.cost;
                    parentVertex[edge.to] = u;
                    parentEdgeIdx[edge.to] = i;

                    //sąsiad do dalszego sprawdzania
                    if (!inQueue[edge.to]) {
                        processQueue.push_back(edge.to);
                        inQueue[edge.to] = true;
                    }
                }
            }
        }
        //jesli bramka końcowa to brak nowych opłacalnych par 
        if (distanceArray[targetNode] <= 0) break;

        //działanie wsteczne 
        int current = targetNode;
        while (current != sourceNode) {
            int parent = parentVertex[current];
            int edgeIdx = parentEdgeIdx[current];

            flowAdj[parent][edgeIdx].flow += 1; //dodanie biletu do rury głównej

            int reverseIdx = flowAdj[parent][edgeIdx].revIdx;

            flowAdj[current][reverseIdx].flow -= 1;

            current = parent; //cofnięcie o jeden krok
        }
        //dodane pkt za te parę do wyniku końcowego
        maximumWeightResult += distanceArray[targetNode];
    }
    return maximumWeightResult;
}

void generalBacktracking(int orderIndex, int totalVertices, const Vector<Vector<Edge>> &adjList,
    const Vector<VertexOrder> &orderList, const Vector<int> &suffixMaxWeightSum, 
    Vector<bool> &visited, int currentWeight, int &maxWeightResult) {

    //pomijanie zajętych
    while (orderIndex < totalVertices && visited[orderList[orderIndex].id]) {
        orderIndex++;
    }
    //warunek końca
    if (orderIndex >= totalVertices) {
        if (currentWeight > maxWeightResult) {
            maxWeightResult = currentWeight; //nowy rekord
        }
        return;
    }
    //warunek odcięcia
    if (currentWeight + (suffixMaxWeightSum[orderIndex] + 1) / 2 <= maxWeightResult) {
        return;
    }

    int u = orderList[orderIndex].id;

    //parowanie u z wolnymi sasiadami
    for (int i = 0; i < adjList[u].size(); i++) {
        int v = adjList[u][i].to;
        int weight = adjList[u][i].weight;

        if (!visited[v]) {
            //idziemy dalej jeśli to ma sens
            int nextSuffix = (orderIndex + 1 < totalVertices) ? suffixMaxWeightSum[orderIndex + 1] : 0;
            if (currentWeight + weight + (nextSuffix + 1) / 2 > maxWeightResult) {
                visited[u] = true;
                visited[v] = true; //blokada pary

                generalBacktracking(orderIndex + 1, totalVertices, adjList, orderList, suffixMaxWeightSum, visited,
                    currentWeight + weight, maxWeightResult);
                
                visited[u] = false;
                visited[v] = false; //backtrack, cofnięcie blokady
            }
        }
    }

    //zostawinie osoby samotnej
    int nextSuffix = (orderIndex + 1 < totalVertices) ? suffixMaxWeightSum[orderIndex + 1] : 0;
    if (currentWeight + (nextSuffix + 1) / 2 > maxWeightResult) {
        visited[u] = true; //blokada jednego
        generalBacktracking(orderIndex + 1, totalVertices, adjList, orderList, suffixMaxWeightSum, visited,
            currentWeight, maxWeightResult);
        
        visited[u] = false;
    }
}

int solveGeneralGraph(int totalVertices, const Vector<Vector<Edge>> &adjList) {
    Vector<Vector<Edge>> sortedAdj = adjList; //kopia listy do modyfikacji

    //sortowanie malejące krawędzi każdego wierzchołka
    for (int i = 1; i <= totalVertices; i++) {
        sortEdgesDescending(sortedAdj[i]);
    }

    Vector<VertexOrder> orderList;
    for (int i = 1; i <= totalVertices; i++) {
        int maxWeight = (sortedAdj[i].size() > 0) ? sortedAdj[i][0].weight : 0;
        VertexOrder vertexInfo = {i, maxWeight}; //przypisanie każdej osoby do jej naj relacji
        orderList.push_back(vertexInfo);
    }

    //sortowanie ludzi malejąco według potencjału
    for (int i = 1; i < orderList.size(); i++) {
        VertexOrder key = orderList[i];
        int j = i - 1;
        while (j >= 0 && orderList[j].maxEdgeWeight < key.maxEdgeWeight) {
            orderList[j + 1] = orderList[j];
            j--;
        }
        orderList[j + 1] = key;
    }
    //liczenie idealnego zysku pkt z danej pary dwóch naj relacji
    Vector<int> suffixMaxWeightSum(totalVertices + 1, 0);
    for (int i = totalVertices - 1; i >= 0; i--) {
        suffixMaxWeightSum[i] = suffixMaxWeightSum[i + 1] + orderList[i].maxEdgeWeight;
    }

    Vector<bool> visited(totalVertices + 1, false); //nikt nie jest odwiedzony na start
    int maxWeightResult = 0;
    
    for (int i = 0; i < totalVertices; i++) {
        int u = orderList[i].id;
        if (!visited[u]) {
            for (int j = 0; j < sortedAdj[u].size(); j++) {
                int v = sortedAdj[u][j].to;
                if (!visited[v]) {
                    visited[u] = true;
                    visited[v] = true;
                    maxWeightResult += sortedAdj[u][j].weight;
                    break;
                }
            }
        }
    }

    for (int i = 1; i <= totalVertices; i++) visited[i] = false;

    generalBacktracking(0, totalVertices, sortedAdj, orderList, suffixMaxWeightSum, visited, 0, maxWeightResult);
    
    return maxWeightResult;
}

void readGraph(int &totalVertices, int&totalEdges, Vector<Vector<Edge>> &adjList) {
    cin >> totalVertices >> totalEdges;

    adjList.resize(totalVertices + 1);

    for (int i = 0; i < totalEdges; i++) {
        int u, v, w;
        cin >> u >> v >> w;

        //krawędź w dwie strony
        Edge edgeToV = {v, w};
        adjList[u].push_back(edgeToV);
        
        Edge edgeToU = {u, w};
        adjList[v].push_back(edgeToU);
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int testCases;
    if(!(cin >> testCases)) return 0;

    while (testCases--) {
        int totalVertices = 0, totalEdges = 0;
        Vector<Vector<Edge>> adjList;

        readGraph(totalVertices, totalEdges, adjList);

        if (totalVertices == 0) continue;

        Vector<int> vertexColors;

        if (checkIsBipartite(totalVertices, adjList, vertexColors)) {
            int result = solveBipartiteMCMF(totalVertices, adjList, vertexColors);
            cout << result << endl;
        } else {
            int result = solveGeneralGraph(totalVertices, adjList);
            cout << result << endl;
        }
    }
    return 0;
}
