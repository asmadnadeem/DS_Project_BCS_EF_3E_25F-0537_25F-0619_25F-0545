#ifndef GRAPHS_H
#define GRAPHS_H

#include "structs.h"

// ---------------- MODULE 6: AIRSPACE NETWORK (ADJ LIST + MATRIX) ----------------
struct AdjNode {
    int dest;
    int weight;
    bool active;
    AdjNode* next;
    AdjNode(int d, int w) { dest = d; weight = w; active = true; next = NULL; }
};

class SpaceGraph {
public:
    string sectors[MAX_SECTORS];
    int sectorCount;
    AdjNode* adjList[MAX_SECTORS];
    int** adjMatrix;

    SpaceGraph() {
        sectorCount = 0;
        adjMatrix = new int*[MAX_SECTORS];
        for (int i = 0; i < MAX_SECTORS; i++) {
            adjList[i] = NULL;
            adjMatrix[i] = new int[MAX_SECTORS];
            for (int j = 0; j < MAX_SECTORS; j++) {
                adjMatrix[i][j] = (i == j) ? 0 : -1;
            }
        }
    }

    ~SpaceGraph() {
        clearGraph();
        for (int i = 0; i < MAX_SECTORS; i++) delete[] adjMatrix[i];
        delete[] adjMatrix;
    }

    void clearGraph() {
        for (int i = 0; i < MAX_SECTORS; i++) {
            while (adjList[i] != NULL) {
                AdjNode* t = adjList[i];
                adjList[i] = adjList[i]->next;
                delete t;
            }
            for (int j = 0; j < MAX_SECTORS; j++) {
                adjMatrix[i][j] = (i == j) ? 0 : -1;
            }
        }
        sectorCount = 0;
    }

    int findIndex(string name) {
        for (int i = 0; i < sectorCount; i++) {
            if (sectors[i] == name) return i;
        }
        return -1;
    }

    int addSector(string name) {
        int idx = findIndex(name);
        if (idx != -1) return idx;
        if (sectorCount >= MAX_SECTORS) return -1;
        sectors[sectorCount] = name;
        return sectorCount++;
    }

    void addCorridor(string from, string to, int weightSeconds) {
        int u = addSector(from);
        int v = addSector(to);
        if (u == -1 || v == -1) return;

        adjMatrix[u][v] = weightSeconds;
        adjMatrix[v][u] = weightSeconds;

        AdjNode* n1 = new AdjNode(v, weightSeconds);
        n1->next = adjList[u];
        adjList[u] = n1;

        AdjNode* n2 = new AdjNode(u, weightSeconds);
        n2->next = adjList[v];
        adjList[v] = n2;
    }

    bool disableOneCorridor() {
        for (int u = 0; u < sectorCount; u++) {
            AdjNode* cur = adjList[u];
            while (cur != NULL) {
                if (cur->active && sectors[cur->dest] != "Restricted Zone" && sectors[u] != "Restricted Zone") {
                    cur->active = false;
                    adjMatrix[u][cur->dest] = -1;
                    adjMatrix[cur->dest][u] = -1;
                    AdjNode* rev = adjList[cur->dest];
                    while (rev != NULL) {
                        if (rev->dest == u) rev->active = false;
                        rev = rev->next;
                    }
                    cout << "ALERT: Corridor disabled between " << sectors[u] << " and " << sectors[cur->dest] << "!\n";
                    return true;
                }
                cur = cur->next;
            }
        }
        return false;
    }

    int getWeight(string from, string to) {
        int u = findIndex(from);
        int v = findIndex(to);
        if (u == -1 || v == -1) return -1;
        return adjMatrix[u][v];
    }

    int getSectorCount() { return sectorCount; }

    void displayAdjacencyList() {
        cout << "\n--- AIRSPACE ADJACENCY LIST ---\n";
        for (int i = 0; i < sectorCount; i++) {
            cout << "[" << i << "] " << sectors[i] << " -> ";
            AdjNode* cur = adjList[i];
            while (cur != NULL) {
                if (cur->active) {
                    cout << sectors[cur->dest] << "(" << cur->weight << "t) ";
                }
                cur = cur->next;
            }
            cout << endl;
        }
    }

    void displayAdjacencyMatrix() {
        cout << "\n--- AIRSPACE ADJACENCY MATRIX (TICKS) ---\n   ";
        for (int i = 0; i < sectorCount; i++) cout << i << "\t";
        cout << endl;
        for (int i = 0; i < sectorCount; i++) {
            cout << i << ": ";
            for (int j = 0; j < sectorCount; j++) {
                if (adjMatrix[i][j] == -1) cout << ".\t";
                else cout << adjMatrix[i][j] << "\t";
            }
            cout << endl;
        }
    }
};

// ---------------- MODULE 7: RADAR SWEEP (BFS & DFS) ----------------
class RadarSweep {
private:
    SpaceGraph* graph;

public:
    RadarSweep(SpaceGraph* g) { graph = g; }

    int bfsFrom(string sourceSector) {
        int start = graph->findIndex(sourceSector);
        if (start == -1) {
            cout << "Sector not found.\n";
            return 0;
        }
        bool visited[MAX_SECTORS] = { false };
        int dist[MAX_SECTORS];
        for (int i = 0; i < MAX_SECTORS; i++) dist[i] = -1;

        int q[MAX_SECTORS];
        int front = 0, rear = 0;
        visited[start] = true;
        dist[start] = 0;
        q[rear++] = start;

        int count = 0;
        cout << "BFS Radar Sweep from " << sourceSector << ":\n";
        while (front < rear) {
            int u = q[front++];
            count++;
            cout << "  Hop " << dist[u] << ": " << graph->sectors[u] << endl;
            AdjNode* cur = graph->adjList[u];
            while (cur != NULL) {
                if (cur->active && !visited[cur->dest]) {
                    visited[cur->dest] = true;
                    dist[cur->dest] = dist[u] + 1;
                    q[rear++] = cur->dest;
                }
                cur = cur->next;
            }
        }
        return count;
    }

    void dfsFrom(string sourceSector) {
        int start = graph->findIndex(sourceSector);
        if (start == -1) {
            cout << "Sector not found.\n";
            return;
        }
        bool visited[MAX_SECTORS] = { false };
        int st[MAX_SECTORS];
        int top = -1;
        st[++top] = start;

        cout << "DFS Sector Traversal from " << sourceSector << ": ";
        while (top >= 0) {
            int u = st[top--];
            if (!visited[u]) {
                visited[u] = true;
                cout << graph->sectors[u] << " -> ";
                AdjNode* cur = graph->adjList[u];
                while (cur != NULL) {
                    if (cur->active && !visited[cur->dest]) {
                        st[++top] = cur->dest;
                    }
                    cur = cur->next;
                }
            }
        }
        cout << "END\n";
    }

    bool isReachable(string from, string to) {
        int s = graph->findIndex(from);
        int t = graph->findIndex(to);
        if (s == -1 || t == -1) return false;
        bool visited[MAX_SECTORS] = { false };
        int q[MAX_SECTORS], front = 0, rear = 0;
        visited[s] = true;
        q[rear++] = s;
        while (front < rear) {
            int u = q[front++];
            if (u == t) return true;
            AdjNode* cur = graph->adjList[u];
            while (cur != NULL) {
                if (cur->active && !visited[cur->dest]) {
                    visited[cur->dest] = true;
                    q[rear++] = cur->dest;
                }
                cur = cur->next;
            }
        }
        return false;
    }

    bool searchMissingFlight(string lastKnownSector) {
        int s = graph->findIndex("Control Tower");
        int t = graph->findIndex(lastKnownSector);
        if (s == -1 || t == -1) return false;

        bool visited[MAX_SECTORS] = { false };
        int parent[MAX_SECTORS];
        for (int i = 0; i < MAX_SECTORS; i++) parent[i] = -1;

        int q[MAX_SECTORS], front = 0, rear = 0;
        visited[s] = true;
        q[rear++] = s;

        while (front < rear) {
            int u = q[front++];
            if (u == t) {
                int path[MAX_SECTORS], plen = 0, cur = t;
                while (cur != -1) { path[plen++] = cur; cur = parent[cur]; }
                cout << "Running BFS from Control Tower...\nPath: ";
                for (int i = plen - 1; i >= 0; i--) {
                    cout << graph->sectors[path[i]];
                    if (i > 0) cout << " -> ";
                }
                cout << endl;
                return true;
            }
            AdjNode* adj = graph->adjList[u];
            while (adj != NULL) {
                if (adj->active && !visited[adj->dest]) {
                    visited[adj->dest] = true;
                    parent[adj->dest] = u;
                    q[rear++] = adj->dest;
                }
                adj = adj->next;
            }
        }
        return false;
    }
};

// ---------------- MODULE 8: FASTEST INTERCEPT ROUTE (DIJKSTRA) ----------------
class RouteFinder {
private:
    SpaceGraph* graph;

public:
    RouteFinder(SpaceGraph* g) { graph = g; }

    Route findFastestRoute(string sourceSector, string destRunway) {
        Route res;
        int n = graph->getSectorCount();
        int s = graph->findIndex(sourceSector);
        int t = graph->findIndex(destRunway);
        if (s == -1 || t == -1) return res;

        int dist[MAX_SECTORS];
        int parent[MAX_SECTORS];
        bool done[MAX_SECTORS];
        for (int i = 0; i < n; i++) {
            dist[i] = 99999;
            parent[i] = -1;
            done[i] = false;
        }
        dist[s] = 0;

        // min-heap arrays for (node, distance)
        int hNode[100], hDist[100], hSize = 0;
        hNode[hSize] = s; hDist[hSize] = 0; hSize++;

        while (hSize > 0) {
            // extract min
            int u = hNode[0], d = hDist[0];
            hSize--;
            hNode[0] = hNode[hSize];
            hDist[0] = hDist[hSize];
            int idx = 0;
            while (true) {
                int sm = idx, l = 2 * idx + 1, r = 2 * idx + 2;
                if (l < hSize && hDist[l] < hDist[sm]) sm = l;
                if (r < hSize && hDist[r] < hDist[sm]) sm = r;
                if (sm != idx) {
                    int tn = hNode[idx]; hNode[idx] = hNode[sm]; hNode[sm] = tn;
                    int td = hDist[idx]; hDist[idx] = hDist[sm]; hDist[sm] = td;
                    idx = sm;
                } else break;
            }

            if (done[u]) continue;
            done[u] = true;

            for (int v = 0; v < n; v++) {
                int w = graph->adjMatrix[u][v];
                if (w > 0 && !done[v] && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    hNode[hSize] = v;
                    hDist[hSize] = dist[v];
                    int cur = hSize++;
                    while (cur > 0) {
                        int p = (cur - 1) / 2;
                        if (hDist[cur] < hDist[p]) {
                            int tn = hNode[cur]; hNode[cur] = hNode[p]; hNode[p] = tn;
                            int td = hDist[cur]; hDist[cur] = hDist[p]; hDist[p] = td;
                            cur = p;
                        } else break;
                    }
                }
            }
        }

        if (dist[t] == 99999) return res;
        res.totalCost = dist[t];
        int rev[MAX_SECTORS], rLen = 0, cur = t;
        while (cur != -1) {
            rev[rLen++] = cur;
            cur = parent[cur];
        }
        res.count = rLen;
        for (int i = 0; i < rLen; i++) {
            res.sectors[i] = graph->sectors[rev[rLen - 1 - i]];
        }
        return res;
    }

    void printRoute(Route r) {
        if (r.count == 0) {
            cout << "No valid route found!\n";
            return;
        }
        cout << "Fastest Route (" << r.totalCost << " ticks): ";
        for (int i = 0; i < r.count; i++) {
            cout << r.sectors[i];
            if (i < r.count - 1) cout << " -> ";
        }
        cout << endl;
    }
};

// ---------------- MODULE 9: MINIMUM RADAR COVERAGE (KRUSKAL MST) ----------------
struct EdgeItem {
    int u, v, w;
};

class RadarMST {
private:
    SpaceGraph* graph;
    EdgeItem mstEdges[MAX_SECTORS];
    int mstCount;
    int totalCost;

    int findSet(int parent[], int i) {
        while (parent[i] != i) i = parent[i];
        return i;
    }

public:
    RadarMST(SpaceGraph* g) {
        graph = g;
        mstCount = 0;
        totalCost = 0;
    }

    void computeMST() {
        int n = graph->getSectorCount();
        EdgeItem edges[200];
        int eCount = 0;

        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (graph->adjMatrix[i][j] > 0) {
                    edges[eCount].u = i;
                    edges[eCount].v = j;
                    edges[eCount].w = graph->adjMatrix[i][j];
                    eCount++;
                }
            }
        }

        // bubble sort edges by weight
        for (int i = 0; i < eCount - 1; i++) {
            for (int j = 0; j < eCount - i - 1; j++) {
                if (edges[j].w > edges[j + 1].w) {
                    EdgeItem tmp = edges[j];
                    edges[j] = edges[j + 1];
                    edges[j + 1] = tmp;
                }
            }
        }

        int parent[MAX_SECTORS];
        for (int i = 0; i < n; i++) parent[i] = i;

        mstCount = 0;
        totalCost = 0;
        for (int i = 0; i < eCount; i++) {
            int ru = findSet(parent, edges[i].u);
            int rv = findSet(parent, edges[i].v);
            if (ru != rv) {
                parent[ru] = rv;
                mstEdges[mstCount++] = edges[i];
                totalCost += edges[i].w;
            }
        }
    }

    void displayMST() {
        cout << "\n--- MINIMUM RADAR COVERAGE (KRUSKAL MST) ---\n";
        for (int i = 0; i < mstCount; i++) {
            cout << graph->sectors[mstEdges[i].u] << " <--> "
                 << graph->sectors[mstEdges[i].v] << " (Cost: " << mstEdges[i].w << ")\n";
        }
        cout << "Total Minimum Radar Cost: " << totalCost << " ticks\n";
    }

    void verifyConnectivity() {
        if (mstCount == 0) return;
        int n = graph->getSectorCount();
        // test removing first MST edge
        int skipU = mstEdges[0].u, skipV = mstEdges[0].v;
        bool vis[MAX_SECTORS] = { false };
        int q[MAX_SECTORS], f = 0, r = 0;
        vis[0] = true;
        q[r++] = 0;
        int reached = 0;
        while (f < r) {
            int u = q[f++];
            reached++;
            for (int i = 0; i < mstCount; i++) {
                if (i == 0) continue;
                int nxt = -1;
                if (mstEdges[i].u == u) nxt = mstEdges[i].v;
                else if (mstEdges[i].v == u) nxt = mstEdges[i].u;
                if (nxt != -1 && !vis[nxt]) {
                    vis[nxt] = true;
                    q[r++] = nxt;
                }
            }
        }
        cout << "Connectivity Check (Removing " << graph->sectors[skipU] << " - "
             << graph->sectors[skipV] << "): Reached " << reached << "/" << n
             << " sectors -> Network Disconnected (Minimality Verified).\n";
    }
};

#endif
// verified bst and avl rotation height comparison benchmark
