#ifndef LINEAR_H
#define LINEAR_H

#include "structs.h"

// ---------------- MODULE 1: AIRCRAFT & CREW REGISTRY + HASH TABLE ----------------
struct HashEntry {
    string key;
    Flight* flight;
    int state; // 0 = empty, 1 = occupied, 2 = deleted
    HashEntry() {
        key = "";
        flight = NULL;
        state = 0;
    }
};

class FlightHashTable {
private:
    HashEntry table[HASH_SIZE];
    int totalCollisions;
    int maxProbeChain;

    int hashFunc(string key) {
        int h = 0;
        for (int i = 0; i < (int)key.length(); i++) {
            h = (h * 31 + (int)key[i]) % HASH_SIZE;
        }
        return (h < 0) ? -h : h;
    }

public:
    FlightHashTable() {
        totalCollisions = 0;
        maxProbeChain = 0;
    }

    void clear() {
        for (int i = 0; i < HASH_SIZE; i++) {
            table[i].key = "";
            table[i].flight = NULL;
            table[i].state = 0;
        }
        totalCollisions = 0;
        maxProbeChain = 0;
    }

    void insert(string callsign, Flight* f) {
        int idx = hashFunc(callsign);
        int probes = 0;
        int firstDeleted = -1;

        while (probes < HASH_SIZE) {
            if (table[idx].state == 1 && table[idx].key == callsign) {
                table[idx].flight = f;
                return;
            }
            if (table[idx].state == 2 && firstDeleted == -1) {
                firstDeleted = idx;
            }
            if (table[idx].state == 0) {
                int target = (firstDeleted != -1) ? firstDeleted : idx;
                table[target].key = callsign;
                table[target].flight = f;
                table[target].state = 1;
                if (probes > maxProbeChain) maxProbeChain = probes;
                return;
            }
            totalCollisions++;
            probes++;
            idx = (idx + 1) % HASH_SIZE;
        }
    }

    Flight* find(string callsign) {
        int idx = hashFunc(callsign);
        int probes = 0;
        while (probes < HASH_SIZE && table[idx].state != 0) {
            if (table[idx].state == 1 && table[idx].key == callsign) {
                return table[idx].flight;
            }
            probes++;
            idx = (idx + 1) % HASH_SIZE;
        }
        return NULL;
    }

    void remove(string callsign) {
        int idx = hashFunc(callsign);
        int probes = 0;
        while (probes < HASH_SIZE && table[idx].state != 0) {
            if (table[idx].state == 1 && table[idx].key == callsign) {
                table[idx].state = 2;
                table[idx].flight = NULL;
                return;
            }
            probes++;
            idx = (idx + 1) % HASH_SIZE;
        }
    }

    void displayStats() {
        int used = 0;
        for (int i = 0; i < HASH_SIZE; i++) {
            if (table[i].state == 1) used++;
        }
        double lf = (double)used / HASH_SIZE;
        cout << "Hash Table Stats -> Entries: " << used << "/" << HASH_SIZE
             << " | Load Factor: " << lf
             << " | Collisions: " << totalCollisions
             << " | Longest Probe: " << maxProbeChain << endl;
    }
};

class Registration {
public:
    Flight* flightHead;
    Crew* crewHead;
    FlightHashTable hashTable;

    Registration() {
        flightHead = NULL;
        crewHead = NULL;
    }

    ~Registration() {
        clearAll();
    }

    void clearAll() {
        while (flightHead != NULL) {
            Flight* temp = flightHead;
            flightHead = flightHead->next;
            delete temp;
        }
        while (crewHead != NULL) {
            Crew* temp = crewHead;
            crewHead = crewHead->next;
            delete temp;
        }
        hashTable.clear();
    }

    void addFlight(Flight* f) {
        f->next = NULL;
        if (flightHead == NULL) {
            flightHead = f;
        } else {
            Flight* cur = flightHead;
            while (cur->next != NULL) cur = cur->next;
            cur->next = f;
        }
        hashTable.insert(f->callsign, f);
    }

    Flight* findFlight(string callsign) {
        return hashTable.find(callsign);
    }

    bool removeFlight(string callsign) {
        hashTable.remove(callsign);
        Flight* cur = flightHead;
        Flight* prev = NULL;
        while (cur != NULL) {
            if (cur->callsign == callsign) {
                if (prev == NULL) flightHead = cur->next;
                else prev->next = cur->next;
                delete cur;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }

    void displayAllFlights() {
        cout << "\n--- ACTIVE FLIGHT RECORDS ---\n";
        Flight* cur = flightHead;
        if (cur == NULL) {
            cout << "No active flights.\n";
            return;
        }
        while (cur != NULL) {
            cout << cur->callsign << " | " << cur->aircraftType
                 << " | Sector: " << cur->currentSector
                 << " | Runway: " << cur->assignedRunway
                 << " | Fuel: " << cur->fuelRemaining
                 << " | Prio: " << cur->priorityLevel
                 << " | Status: " << cur->status << endl;
            cur = cur->next;
        }
        hashTable.displayStats();
    }

    void addCrew(Crew* c) {
        c->next = NULL;
        if (crewHead == NULL) {
            crewHead = c;
        } else {
            Crew* cur = crewHead;
            while (cur->next != NULL) cur = cur->next;
            cur->next = c;
        }
    }

    void displayAllCrew() {
        cout << "\n--- CREW MANIFEST ---\n";
        Crew* cur = crewHead;
        while (cur != NULL) {
            cout << cur->employeeID << " | " << cur->name
                 << " | " << cur->role << " | Flight: " << cur->assignedCallsign << endl;
            cur = cur->next;
        }
    }
};

// ---------------- MODULE 2: LANDING QUEUE & EMERGENCY MIN-HEAP ----------------
class EmergencyLanding {
private:
    Flight* normQ[100];
    int qFront, qRear, qCount;

    Flight* heap[100];
    int heapSize;

    // lower priorityLevel comes first; tie-breaker is earlier arrivalTick
    bool isHigher(Flight* a, Flight* b) {
        if (a->priorityLevel != b->priorityLevel)
            return a->priorityLevel < b->priorityLevel;
        return a->arrivalTick < b->arrivalTick;
    }

    void heapifyUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (isHigher(heap[i], heap[p])) {
                Flight* tmp = heap[i];
                heap[i] = heap[p];
                heap[p] = tmp;
                i = p;
            } else break;
        }
    }

    void heapifyDown(int i) {
        while (true) {
            int smallest = i;
            int l = 2 * i + 1;
            int r = 2 * i + 2;
            if (l < heapSize && isHigher(heap[l], heap[smallest])) smallest = l;
            if (r < heapSize && isHigher(heap[r], heap[smallest])) smallest = r;
            if (smallest != i) {
                Flight* tmp = heap[i];
                heap[i] = heap[smallest];
                heap[smallest] = tmp;
                i = smallest;
            } else break;
        }
    }

public:
    EmergencyLanding() {
        qFront = 0;
        qRear = -1;
        qCount = 0;
        heapSize = 0;
    }

    void reset() {
        qFront = 0;
        qRear = -1;
        qCount = 0;
        heapSize = 0;
    }

    void enqueueNormal(Flight* f) {
        if (qCount >= 100) return;
        qRear = (qRear + 1) % 100;
        normQ[qRear] = f;
        qCount++;
    }

    Flight* dequeueNormal() {
        if (qCount == 0) return NULL;
        Flight* f = normQ[qFront];
        qFront = (qFront + 1) % 100;
        qCount--;
        return f;
    }

    Flight* frontNormal() {
        if (qCount == 0) return NULL;
        return normQ[qFront];
    }

    bool isEmptyNormal() { return qCount == 0; }

    void insertEmergency(Flight* f) {
        if (heapSize >= 100) return;
        heap[heapSize] = f;
        heapifyUp(heapSize);
        heapSize++;
    }

    Flight* extractMin() {
        if (heapSize == 0) return NULL;
        Flight* top = heap[0];
        heap[0] = heap[heapSize - 1];
        heapSize--;
        heapifyDown(0);
        return top;
    }

    Flight* peekMin() {
        if (heapSize == 0) return NULL;
        return heap[0];
    }

    bool isEmptyHeap() { return heapSize == 0; }
    int getEmergencyCount() { return heapSize; }

    void removeFlightPtr(Flight* f) {
        // remove from normal queue if present
        int n = qCount;
        for (int i = 0; i < n; i++) {
            Flight* cur = dequeueNormal();
            if (cur != f) enqueueNormal(cur);
        }
        // remove from emergency heap if present
        for (int i = 0; i < heapSize; i++) {
            if (heap[i] == f) {
                heap[i] = heap[heapSize - 1];
                heapSize--;
                heapifyDown(i);
                heapifyUp(i);
                break;
            }
        }
    }

    void displayNormalQueue() {
        cout << "Normal Landing Queue (" << qCount << "): ";
        if (qCount == 0) { cout << "Empty\n"; return; }
        for (int i = 0; i < qCount; i++) {
            int idx = (qFront + i) % 100;
            cout << normQ[idx]->callsign << " ";
        }
        cout << endl;
    }

    void displayHeap() {
        cout << "Emergency Priority Heap (" << heapSize << "): ";
        if (heapSize == 0) { cout << "Empty\n"; return; }
        for (int i = 0; i < heapSize; i++) {
            cout << heap[i]->callsign << "(P" << heap[i]->priorityLevel << ") ";
        }
        cout << endl;
    }
};

// ---------------- MODULE 3: COMMAND HISTORY (DLL) & UNDO (STACK) ----------------
struct HistoryNode {
    string logText;
    HistoryNode* prev;
    HistoryNode* next;
    HistoryNode(string t) { logText = t; prev = NULL; next = NULL; }
};

struct SnapshotNode {
    string callsign;
    string oldRunway;
    int oldPriority;
    string oldSector;
    string oldStatus;
    SnapshotNode* next;
};

class CommandHistory {
private:
    HistoryNode* head;
    HistoryNode* tail;
    SnapshotNode* stackTop;

public:
    CommandHistory() {
        head = NULL;
        tail = NULL;
        stackTop = NULL;
    }

    ~CommandHistory() {
        clearAll();
    }

    void clearAll() {
        while (head != NULL) {
            HistoryNode* t = head;
            head = head->next;
            delete t;
        }
        tail = NULL;
        while (stackTop != NULL) {
            SnapshotNode* s = stackTop;
            stackTop = stackTop->next;
            delete s;
        }
    }

    void logCommand(string action, string callsign) {
        HistoryNode* node = new HistoryNode(action + " -> " + callsign);
        if (head == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
    }

    void pushSnapshot(Flight* f) {
        if (f == NULL) return;
        SnapshotNode* s = new SnapshotNode();
        s->callsign = f->callsign;
        s->oldRunway = f->assignedRunway;
        s->oldPriority = f->priorityLevel;
        s->oldSector = f->currentSector;
        s->oldStatus = f->status;
        s->next = stackTop;
        stackTop = s;
    }

    bool undo(Registration& reg) {
        if (stackTop == NULL) return false;
        SnapshotNode* s = stackTop;
        stackTop = stackTop->next;

        Flight* f = reg.findFlight(s->callsign);
        if (f != NULL) {
            f->assignedRunway = s->oldRunway;
            f->priorityLevel = s->oldPriority;
            f->currentSector = s->oldSector;
            f->status = s->oldStatus;
            logCommand("UNDO RESTORED", s->callsign);
        }
        delete s;
        return true;
    }

    void displayHistory(int count) {
        cout << "\n--- RECENT COMMAND HISTORY ---\n";
        HistoryNode* cur = tail;
        int shown = 0;
        while (cur != NULL && shown < count) {
            cout << "- " << cur->logText << endl;
            cur = cur->prev;
            shown++;
        }
    }

    void displayLastCommand() {
        if (tail != NULL) cout << "Last Command: " << tail->logText << endl;
        else cout << "No commands logged yet.\n";
    }
};

#endif
