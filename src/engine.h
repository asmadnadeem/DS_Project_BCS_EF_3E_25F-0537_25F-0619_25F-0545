#ifndef ENGINE_H
#define ENGINE_H
#define _CRT_SECURE_NO_WARNINGS

#include <ctime>
#include "linear.h"
#include "trees.h"
#include "graphs.h"

// ---------------- MODULE 11: LEADERBOARD PERSISTENCE ----------------
struct LeaderEntry {
    string cro;
    string result;
    int cred;
    int ticks;
    int crash;
    int hijack;
    int breach;
    string date;
};

class LeaderBoard {
private:
    string getTodayDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        char buf[20];
        sprintf(buf, "%02d-%02d-%02d", ltm->tm_mday, 1 + ltm->tm_mon, (1900 + ltm->tm_year) % 100);
        return string(buf);
    }

    bool comesBefore(LeaderEntry a, LeaderEntry b) {
        if (a.result != b.result) return a.result == "WIN";
        if (a.cred != b.cred) return a.cred > b.cred;
        if (a.ticks != b.ticks) return a.ticks > b.ticks;
        return a.date > b.date;
    }

public:
    void saveResult(string cro, string result, int cred, int ticks, int crash, int hijack, int breach) {
        LeaderEntry list[100];
        int count = 0;

        ifstream in("leaderboard.txt");
        string line;
        if (in.is_open()) {
            getline(in, line); // skip header
            while (getline(in, line) && count < 99) {
                if (trimStr(line) == "") continue;
                string p[10];
                if (splitPipe(line, p, 10) >= 9) {
                    list[count].cro = p[1];
                    list[count].result = p[2];
                    list[count].cred = toInt(p[3]);
                    list[count].ticks = toInt(p[4]);
                    list[count].crash = toInt(p[5]);
                    list[count].hijack = toInt(p[6]);
                    list[count].breach = toInt(p[7]);
                    list[count].date = p[8];
                    count++;
                }
            }
            in.close();
        }

        list[count].cro = cro;
        list[count].result = result;
        list[count].cred = cred;
        list[count].ticks = ticks;
        list[count].crash = crash;
        list[count].hijack = hijack;
        list[count].breach = breach;
        list[count].date = getTodayDate();
        count++;

        // sort entries
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (!comesBefore(list[j], list[j + 1])) {
                    LeaderEntry tmp = list[j];
                    list[j] = list[j + 1];
                    list[j + 1] = tmp;
                }
            }
        }

        ofstream out("leaderboard.txt");
        out << "#|CRO|Result|Cred|Ticks|Crash|Hijack|Breach|Date\n";
        for (int i = 0; i < count; i++) {
            out << (i + 1) << "|" << list[i].cro << "|" << list[i].result << "|"
                << list[i].cred << "|" << list[i].ticks << "|" << list[i].crash << "|"
                << list[i].hijack << "|" << list[i].breach << "|" << list[i].date << "\n";
        }
        out.close();
    }

    void displayLeaderboard() {
        cout << "\n================ MAYDAY-404 LEADERBOARD ================\n";
        ifstream in("leaderboard.txt");
        if (!in.is_open()) {
            cout << "leaderboard.txt not found.\n";
            return;
        }
        string line;
        while (getline(in, line)) {
            cout << line << endl;
        }
        in.close();
    }
};

// ---------------- MODULE 10: GAME ENGINE & SIMULATION LOOP ----------------
class GameEngine {
private:
    Registration reg;
    EmergencyLanding landing;
    CommandHistory history;
    BSTflight bst;
    AVLflight avl;
    SpaceGraph graph;
    RadarSweep* radar;
    RouteFinder* router;
    RadarMST* mst;
    LeaderBoard board;

    MissingFlight* missingHead;
    ScenarioEvent events[30];
    int eventCount;

    int tick;
    int credibility;
    int actionsLeft;
    bool cyberActive;
    string cyberSector;
    int cyberTicks;
    bool towerDegraded;

    int crashes, hijacks, breaches, landedCount, resolvedCount, missingFound, warCountered;
    string croName;
    bool gameOver;
    string loseReason;

    void printFile(string filename) {
        ifstream in(filename.c_str());
        if (!in.is_open()) {
            cout << "[Warning: " << filename << " missing]\n";
            return;
        }
        string line;
        while (getline(in, line)) cout << line << endl;
        in.close();
    }

    void clearMissing() {
        while (missingHead != NULL) {
            MissingFlight* t = missingHead;
            missingHead = missingHead->next;
            delete t;
        }
    }

    void loadAirspace() {
        graph.clearGraph();
        ifstream in("airspace.txt");
        string line;
        if (!in.is_open()) return;
        getline(in, line);
        while (getline(in, line)) {
            if (trimStr(line) == "") continue;
            string p[4];
            if (splitPipe(line, p, 4) >= 3) {
                graph.addCorridor(p[0], p[1], toInt(p[2]));
            }
        }
        in.close();
    }

    void loadFlightsAndCrew() {
        reg.clearAll();
        landing.reset();
        bst.destroyTree();
        avl.destroyTree();

        ifstream in("flights.txt");
        string line;
        if (in.is_open()) {
            getline(in, line);
            while (getline(in, line)) {
                if (trimStr(line) == "") continue;
                string p[12];
                if (splitPipe(line, p, 12) >= 11) {
                    Flight* f = new Flight();
                    f->callsign = p[0];
                    f->aircraftType = p[1];
                    f->airline = p[2];
                    f->origin = p[3];
                    f->destination = p[4];
                    f->passengerCount = toInt(p[5]);
                    f->fuelRemaining = toInt(p[6]);
                    f->currentSector = p[7];
                    f->assignedRunway = p[8];
                    f->priorityLevel = toInt(p[9]);
                    f->arrivalTick = toInt(p[10]);
                    reg.addFlight(f);
                    bst.insert(f);
                    avl.insert(f);
                    if (f->priorityLevel >= 9) landing.enqueueNormal(f);
                    else landing.insertEmergency(f);
                }
            }
            in.close();
        }

        ifstream cinf("crew.txt");
        if (cinf.is_open()) {
            getline(cinf, line);
            while (getline(cinf, line)) {
                if (trimStr(line) == "") continue;
                string p[5];
                if (splitPipe(line, p, 5) >= 4) {
                    Crew* c = new Crew();
                    c->employeeID = p[0];
                    c->name = p[1];
                    c->role = p[2];
                    c->assignedCallsign = p[3];
                    reg.addCrew(c);
                }
            }
            cinf.close();
        }
    }

    void loadScenario() {
        eventCount = 0;
        ifstream in("scenario.txt");
        string line;
        if (!in.is_open()) return;
        getline(in, line);
        while (getline(in, line) && eventCount < 30) {
            if (trimStr(line) == "") continue;
            string p[4];
            if (splitPipe(line, p, 4) >= 3) {
                events[eventCount].tick = toInt(p[0]);
                events[eventCount].eventType = p[1];
                events[eventCount].target = p[2];
                events[eventCount].triggered = false;
                eventCount++;
            }
        }
        in.close();
    }

    void addCred(int delta) {
        credibility += delta;
        if (credibility > 200) credibility = 200;
        if (credibility <= 0) {
            credibility = 0;
            gameOver = true;
            loseReason = "Credibility dropped to zero!";
        }
    }

    int wrongPenalty() {
        return towerDegraded ? -10 : -5;
    }

    void triggerScenarioEvents() {
        for (int i = 0; i < eventCount; i++) {
            if (!events[i].triggered && events[i].tick == tick) {
                events[i].triggered = true;
                string type = events[i].eventType;
                string tgt = events[i].target;

                if (type == "Hijack") {
                    Flight* f = reg.findFlight(tgt);
                    if (f != NULL) {
                        f->priorityLevel = 2;
                        f->incidentTimer = 3;
                        landing.removeFlightPtr(f);
                        landing.insertEmergency(f);
                        cout << "\n*** MAYDAY MAYDAY MAYDAY ***\n"
                             << tgt << ": Squawking 7500! Suspected HIJACK! Window: 3 ticks.\n";
                    }
                } else if (type == "Cyber") {
                    cyberActive = true;
                    cyberSector = tgt;
                    cyberTicks = 0;
                    cout << "\n*** CYBER ALERT ***\nRadar Relay (" << tgt
                         << ") reports intrusion! Window: 2 ticks.\n";
                } else if (type == "Fuel") {
                    Flight* f = reg.findFlight(tgt);
                    if (f != NULL) {
                        f->priorityLevel = 5;
                        f->fuelRemaining = 5;
                        f->incidentTimer = 5;
                        landing.removeFlightPtr(f);
                        landing.insertEmergency(f);
                        cout << "\n*** PAN-PAN PAN-PAN ***\n"
                             << tgt << ": Bingo fuel! Emergency priority 5. Window: 5 ticks.\n";
                    }
                } else if (type == "Missing") {
                    MissingFlight* m = new MissingFlight();
                    m->callsign = tgt;
                    m->lastKnownSector = "Syntax Sector (West)";
                    m->tickWindow = 9;
                    m->credThreshold = 54;
                    m->spawnTick = tick;
                    m->next = missingHead;
                    missingHead = m;
                    cout << "\n*** MISSING FLIGHT ***\n" << tgt
                         << " lost radar contact over Syntax Sector (West)! Window: 9 ticks.\n";
                } else if (type == "War") {
                    Flight* f = new Flight();
                    f->callsign = tgt;
                    f->aircraftType = (tgt == "NA-640") ? "C-130" : "MiG-21";
                    f->airline = "NullAir";
                    f->origin = "Hostile";
                    f->destination = "Restricted Zone";
                    f->passengerCount = 1;
                    f->fuelRemaining = 20;
                    f->currentSector = "Alpha Tree (South)";
                    f->assignedRunway = "Restricted Zone";
                    f->priorityLevel = 1;
                    f->arrivalTick = tick;
                    f->incidentTimer = 2;
                    reg.addFlight(f);
                    bst.insert(f);
                    avl.insert(f);
                    landing.insertEmergency(f);
                    cout << "\n*** CHECK SIX WAR EVENT ***\nBandit " << tgt
                         << " entered Alpha Tree (South)! Window: 2 ticks.\n";
                }
            }
        }
    }

    void advanceSimulation() {
        tick++;
        cout << "\n--- TICK " << tick << "/30 SIMULATION ADVANCE ---\n";

        if (cyberActive) {
            cyberTicks++;
            if (cyberTicks == 2) {
                graph.disableOneCorridor();
            } else if (cyberTicks >= 3) {
                towerDegraded = true;
                breaches++;
                gameOver = true;
                loseReason = "Control Tower disabled by un-countered cyber attack!";
                return;
            }
        }

        // check missing flights window
        MissingFlight* m = missingHead;
        while (m != NULL) {
            if (!m->found && (tick - m->spawnTick) > m->tickWindow) {
                hijacks++;
                gameOver = true;
                loseReason = "Missing flight " + m->callsign + " breached Restricted Zone!";
                return;
            }
            m = m->next;
        }

        // advance active flights
        Flight* cur = reg.flightHead;
        while (cur != NULL) {
            Flight* nextF = cur->next;
            cur->fuelRemaining--;

            if (cur->incidentTimer > 0) {
                cur->incidentTimer--;
                if (cur->incidentTimer == 0) {
                    if (cur->priorityLevel == 1) {
                        gameOver = true;
                        loseReason = "Hostile War aircraft " + cur->callsign + " was not intercepted!";
                        return;
                    } else if (cur->priorityLevel == 2) {
                        hijacks++;
                        gameOver = true;
                        loseReason = "Hijacked flight " + cur->callsign + " disappeared!";
                        return;
                    }
                }
            }

            if (cur->status == "CLEARED") {
                cout << cur->callsign << " traversed " << cur->currentSector
                     << " -> " << cur->assignedRunway << ". LANDED.\n";
                if (cur->priorityLevel <= 8 &&
                    (cur->assignedRunway.find("04") != string::npos || cur->assignedRunway.find("17") != string::npos)) {
                    cout << "Penalty: Emergency landed on non-emergency runway (-10 Credibility).\n";
                    addCred(-10);
                } else {
                    addCred(2);
                    cout << cur->callsign << " landed safely. Credibility +2 -> " << credibility << endl;
                }
                landedCount++;
                string cs = cur->callsign;
                landing.removeFlightPtr(cur);
                bst.remove(cs);
                avl.remove(cs);
                reg.removeFlight(cs);
            } else if (cur->fuelRemaining <= 0) {
                crashes++;
                history.logCommand("CRASHED", cur->callsign);
                addCred(-50);
                gameOver = true;
                loseReason = "Flight " + cur->callsign + " ran out of fuel and CRASHED!";
                return;
            } else {
                cout << cur->callsign << " fuel -> " << cur->fuelRemaining << endl;
            }
            cur = nextF;
        }

        if (!gameOver && tick <= 30) {
            triggerScenarioEvents();
        }
    }

    void displayDashboard() {
        int activeCount = 0;
        Flight* cur = reg.flightHead;
        while (cur != NULL) { activeCount++; cur = cur->next; }

        cout << "\n==========================================================\n";
        cout << "TICK " << tick << "/30 | Actions: " << actionsLeft << "/2 | Credibility: " << credibility
             << " | Active Flights: " << activeCount
             << " | Emergencies: " << landing.getEmergencyCount() << endl;
        cout << "==========================================================\n";
        cout << "RADAR:\n";
        cur = reg.flightHead;
        while (cur != NULL) {
            cout << "  [" << cur->currentSector << "] " << cur->callsign
                 << " (" << cur->aircraftType << ") -> " << cur->assignedRunway
                 << " | Fuel: " << cur->fuelRemaining << " | P" << cur->priorityLevel << endl;
            cur = cur->next;
        }
    }

    void startNewGame() {
        while (true) {
            cout << "Enter CRO name (no spaces, use _ or .): ";
            cin >> croName;
            if (croName != "" && croName.find(' ') == string::npos) break;
            cout << "Invalid name! Try again.\n";
        }

        clearMissing();
        history.clearAll();
        loadAirspace();
        loadFlightsAndCrew();
        loadScenario();

        tick = 1;
        credibility = 100;
        actionsLeft = 2;
        cyberActive = false;
        cyberSector = "";
        cyberTicks = 0;
        towerDegraded = false;
        crashes = hijacks = breaches = landedCount = resolvedCount = missingFound = warCountered = 0;
        gameOver = false;
        loseReason = "";

        cout << "Loading scenario... 30 ticks to survive.\n";

        while (!gameOver && tick <= 30) {
            displayDashboard();
            printFile("gamemenu.txt");
            cout << "Choice: ";
            int choice;
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input! Credibility " << wrongPenalty() << endl;
                addCred(wrongPenalty());
                continue;
            }

            int cost = towerDegraded ? 2 : 1;

            if (choice == 1) { // Register Aircraft
                Flight* f = new Flight();
                cout << "Callsign: "; cin >> f->callsign;
                cout << "Type: "; cin >> f->aircraftType;
                cout << "Airline: "; cin >> f->airline;
                cout << "Fuel: "; cin >> f->fuelRemaining;
                f->currentSector = "Control Tower";
                f->assignedRunway = "Runway 12 (West)";
                f->priorityLevel = 9;
                f->arrivalTick = tick;
                reg.addFlight(f);
                bst.insert(f);
                avl.insert(f);
                landing.enqueueNormal(f);
                history.logCommand("REGISTER AIRCRAFT", f->callsign);
                actionsLeft -= cost;
            } else if (choice == 2) { // Register Crew
                Crew* c = new Crew();
                cout << "Employee ID: "; cin >> c->employeeID;
                cout << "Name (use _): "; cin >> c->name;
                cout << "Role: "; cin >> c->role;
                cout << "Assigned Callsign: "; cin >> c->assignedCallsign;
                reg.addCrew(c);
                history.logCommand("REGISTER CREW", c->employeeID);
                actionsLeft -= cost;
            } else if (choice == 3) { // Search Flight by Callsign
                string cs;
                cout << "Enter Callsign: "; cin >> cs;
                Flight* f = reg.findFlight(cs);
                if (f != NULL) {
                    cout << "Found in Hash Table: " << f->callsign << " | " << f->aircraftType
                         << " | Sector: " << f->currentSector << " | Fuel: " << f->fuelRemaining << endl;
                } else {
                    cout << "Flight not found! Penalty " << wrongPenalty() << endl;
                    addCred(wrongPenalty());
                }
                actionsLeft -= cost;
            } else if (choice == 4 || choice == 5) { // Add Normal / Emergency Arrival
                Flight* f = new Flight();
                cout << "Callsign: "; cin >> f->callsign;
                f->aircraftType = (choice == 4) ? "ATR 72" : "MedEvac-1";
                f->airline = "BlackBox";
                f->fuelRemaining = 10;
                f->currentSector = "Control Tower";
                f->assignedRunway = "Runway 12 (West)";
                f->arrivalTick = tick;
                if (choice == 4) {
                    f->priorityLevel = 9;
                    landing.enqueueNormal(f);
                } else {
                    cout << "Priority (1-8): "; cin >> f->priorityLevel;
                    landing.insertEmergency(f);
                }
                reg.addFlight(f);
                bst.insert(f);
                avl.insert(f);
                history.logCommand("SPAWN ARRIVAL", f->callsign);
                actionsLeft -= cost;
            } else if (choice == 6) { // Process Next Landing
                Flight* f = !landing.isEmptyNormal() ? landing.dequeueNormal() : landing.extractMin();
                if (f != NULL) {
                    f->status = "CLEARED";
                    cout << "Processing next landing: " << f->callsign << " cleared for " << f->assignedRunway << endl;
                    history.logCommand("PROCESS LANDING", f->callsign);
                } else {
                    cout << "No flights waiting in queues.\n";
                }
                actionsLeft -= cost;
            } else if (choice == 7) { // Clear Flight for Runway
                string cs, rw;
                cout << "Callsign: "; cin >> cs;
                cout << "Runway (04/09/12/17): "; cin >> rw;
                Flight* f = reg.findFlight(cs);
                if (f == NULL) {
                    cout << "Non-existent flight! Penalty " << wrongPenalty() << endl;
                    addCred(wrongPenalty());
                } else {
                    history.pushSnapshot(f);
                    f->assignedRunway = "Runway " + rw;
                    f->status = "CLEARED";
                    cout << "CRO: \"" << cs << ", cleared for landing Runway " << rw << ".\"\n";
                    cout << cs << ": \"Wilco, cleared Runway " << rw << ", " << cs << ".\"\n";
                    history.logCommand("CLEAR RUNWAY " + rw, cs);
                }
                actionsLeft -= cost;
            } else if (choice == 8) { // Reroute Flight
                string cs, rw;
                cout << "Callsign: "; cin >> cs;
                cout << "New Runway (Runway_12_(West) etc., use _): "; cin >> rw;
                for (int i = 0; i < (int)rw.length(); i++) if (rw[i] == '_') rw[i] = ' ';
                Flight* f = reg.findFlight(cs);
                if (f == NULL) {
                    cout << "Flight not found! Penalty " << wrongPenalty() << endl;
                    addCred(wrongPenalty());
                } else {
                    history.pushSnapshot(f);
                    f->assignedRunway = rw;
                    Route r = router->findFastestRoute(f->currentSector, rw);
                    router->printRoute(r);
                    history.logCommand("REROUTE", cs);
                }
                actionsLeft -= cost;
            } else if (choice == 9) { // Intercept Aircraft
                string target;
                cout << "Callsign or Sector (use _ for spaces): "; cin >> target;
                for (int i = 0; i < (int)target.length(); i++) if (target[i] == '_') target[i] = ' ';
                bool hit = false;
                Flight* cur = reg.flightHead;
                while (cur != NULL) {
                    Flight* nxt = cur->next;
                    if ((cur->callsign == target || cur->currentSector == target) && cur->priorityLevel <= 2) {
                        hit = true;
                        if (cur->priorityLevel == 1) {
                            cout << "Hostile " << cur->callsign << " intercepted! WAR countered (+30 Credibility).\n";
                            addCred(30);
                            warCountered++;
                            resolvedCount++;
                            string cs = cur->callsign;
                            landing.removeFlightPtr(cur);
                            bst.remove(cs);
                            avl.remove(cs);
                            reg.removeFlight(cs);
                        } else {
                            history.pushSnapshot(cur);
                            cur->priorityLevel = 9;
                            cur->incidentTimer = 0;
                            cout << "Intercept team dispatched. HIJACK on " << cur->callsign << " countered (+20 Credibility).\n";
                            addCred(20);
                            resolvedCount++;
                        }
                        history.logCommand("INTERCEPT", target);
                    }
                    cur = nxt;
                }
                if (!hit) {
                    cout << "Failed intercept! Penalty -20 Credibility.\n";
                    addCred(-20);
                }
                actionsLeft -= cost;
            } else if (choice == 10) { // Escalate Priority
                string cs;
                int p;
                cout << "Callsign: "; cin >> cs;
                cout << "New Priority (1-8): "; cin >> p;
                Flight* f = reg.findFlight(cs);
                if (f != NULL && p >= 1 && p <= 8) {
                    history.pushSnapshot(f);
                    f->priorityLevel = p;
                    landing.removeFlightPtr(f);
                    landing.insertEmergency(f);
                    history.logCommand("ESCALATE PRIORITY", cs);
                } else {
                    cout << "Invalid input! Penalty " << wrongPenalty() << endl;
                    addCred(wrongPenalty());
                }
                actionsLeft -= cost;
            } else if (choice == 11) { // Counter Cyber Attack
                string sec;
                cout << "Sector (use _ for spaces): "; cin >> sec;
                for (int i = 0; i < (int)sec.length(); i++) if (sec[i] == '_') sec[i] = ' ';
                if (cyberActive && sec == cyberSector) {
                    cyberActive = false;
                    towerDegraded = false;
                    addCred(15);
                    resolvedCount++;
                    cout << "Counter-cyber protocol engaged on " << sec << ". Credibility +15 -> " << credibility << endl;
                    history.logCommand("COUNTER CYBER", sec);
                } else {
                    cout << "No active cyber intrusion there! Penalty -15 Credibility.\n";
                    addCred(-15);
                }
                actionsLeft -= cost;
            } else if (choice == 12) { // Undo
                if (history.undo(reg)) cout << "Last command undone!\n";
                else cout << "Nothing to undo.\n";
                actionsLeft -= cost;
            } else if (choice == 13) { // Free: Pending Arrivals
                landing.displayNormalQueue();
                landing.displayHeap();
            } else if (choice == 14) { // Free: Crew Members
                reg.displayAllCrew();
            } else if (choice == 15) { // Free: Flight Records
                reg.displayAllFlights();
                cout << "\n--- BST ORDERED MANIFEST ---\n";
                bst.inorderTraversal();
            } else if (choice == 16) { // Free: Airspace Network
                graph.displayAdjacencyList();
                graph.displayAdjacencyMatrix();
            } else if (choice == 17) { // BFS Radar Sweep
                radar->bfsFrom("Control Tower");
                actionsLeft -= cost;
            } else if (choice == 18) { // DFS Sector Traversal
                radar->dfsFrom("Alpha Tree (South)");
                cout << "Reachable Alpha Tree -> Restricted Zone: "
                     << (radar->isReachable("Alpha Tree (South)", "Restricted Zone") ? "YES" : "NO") << endl;
                actionsLeft -= cost;
            } else if (choice == 19) { // Search Missing Flight
                MissingFlight* m = missingHead;
                while (m != NULL && m->found) m = m->next;
                if (m == NULL) {
                    cout << "No missing flights active.\n";
                } else if (credibility < m->credThreshold) {
                    cout << "Insufficient credibility (need " << m->credThreshold << ")! Penalty -5.\n";
                    addCred(-5);
                } else if (radar->searchMissingFlight(m->lastKnownSector)) {
                    m->found = true;
                    missingFound++;
                    resolvedCount++;
                    addCred(20);
                    cout << "Flight " << m->callsign << " located! Credibility +20 -> " << credibility << endl;
                    history.logCommand("FOUND MISSING", m->callsign);
                }
                actionsLeft -= cost;
            } else if (choice == 20) { // Dijkstra Fastest Route
                string src, dst;
                cout << "From Sector (use _): "; cin >> src;
                cout << "To Runway (use _): "; cin >> dst;
                for (int i = 0; i < (int)src.length(); i++) if (src[i] == '_') src[i] = ' ';
                for (int i = 0; i < (int)dst.length(); i++) if (dst[i] == '_') dst[i] = ' ';
                Route r = router->findFastestRoute(src, dst);
                router->printRoute(r);
                actionsLeft -= cost;
            } else if (choice == 21) { // Minimum Radar Coverage (MST)
                mst->computeMST();
                mst->displayMST();
                mst->verifyConnectivity();
                actionsLeft -= cost;
            } else if (choice == 22) { // Free: Compare BST and AVL Heights
                avl.runHeightBenchmark();
            } else if (choice == 23) { // End Turn
                actionsLeft = 0;
            } else if (choice == 24) { // Exit to Main Menu (counts as LOSE)
                gameOver = true;
                loseReason = "CRO abandoned post (Exited to Main Menu).";
            } else {
                cout << "Invalid choice! Credibility " << wrongPenalty() << endl;
                addCred(wrongPenalty());
            }

            if (!gameOver && actionsLeft <= 0) {
                advanceSimulation();
                actionsLeft = 2;
            }
        }

        string resStr = gameOver ? "LOSE" : "WIN";
        int finalTicks = (tick > 30) ? 30 : tick;
        cout << "\n==================== " << (gameOver ? "GAME OVER" : "YOU WIN") << " ====================\n";
        if (gameOver) cout << "Reason: " << loseReason << endl;
        cout << "CRO: " << croName << " | Final Credibility: " << credibility
             << " | Ticks Used: " << finalTicks << " | Landed: " << landedCount
             << " | Emergencies Resolved: " << resolvedCount << endl;
        board.saveResult(croName, resStr, credibility, finalTicks, crashes, hijacks, breaches);
        cout << "Result saved to leaderboard.\n";
    }

public:
    GameEngine() {
        radar = new RadarSweep(&graph);
        router = new RouteFinder(&graph);
        mst = new RadarMST(&graph);
        missingHead = NULL;
    }

    ~GameEngine() {
        clearMissing();
        delete radar;
        delete router;
        delete mst;
    }

    void run() {
        while (true) {
            printFile("mainmenu.txt");
            cout << "Choice: ";
            int ch;
            if (!(cin >> ch)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Please enter a valid number (1-5).\n";
                continue;
            }
            if (ch == 1) startNewGame();
            else if (ch == 2) board.displayLeaderboard();
            else if (ch == 3) printFile("about.txt");
            else if (ch == 4) printFile("rules.txt");
            else if (ch == 5) {
                cout << "Thank you for playing MayDay-404!\n";
                break;
            } else {
                cout << "Invalid menu option!\n";
            }
        }
    }
};

#endif

