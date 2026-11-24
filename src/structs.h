#ifndef STRUCTS_H
#define STRUCTS_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const int MAX_SECTORS = 20;
const int HASH_SIZE = 101;

// trim leading and trailing spaces
inline string trimStr(string s) {
    int start = 0;
    while (start < (int)s.length() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r')) {
        start++;
    }
    int end = (int)s.length() - 1;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r')) {
        end--;
    }
    if (start > end) return "";
    return s.substr(start, end - start + 1);
}

// split line by '|' into parts array
inline int splitPipe(string line, string parts[], int maxParts) {
    int count = 0;
    string cur = "";
    for (int i = 0; i < (int)line.length(); i++) {
        if (line[i] == '|') {
            if (count < maxParts) {
                parts[count++] = trimStr(cur);
            }
            cur = "";
        } else {
            cur += line[i];
        }
    }
    if (count < maxParts) {
        parts[count++] = trimStr(cur);
    }
    return count;
}

// simple string to integer converter
inline int toInt(string s) {
    s = trimStr(s);
    if (s == "") return 0;
    int sign = 1, i = 0, val = 0;
    if (s[0] == '-') {
        sign = -1;
        i = 1;
    }
    for (; i < (int)s.length(); i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            val = val * 10 + (s[i] - '0');
        }
    }
    return val * sign;
}

struct Flight {
    string callsign;
    string aircraftType;
    string airline;
    string origin;
    string destination;
    int passengerCount;
    int fuelRemaining;
    string currentSector;
    string assignedRunway;
    int arrivalTick;
    int priorityLevel;
    string status;
    int incidentTimer;
    Flight* next;

    Flight() {
        passengerCount = 0;
        fuelRemaining = 10;
        arrivalTick = 0;
        priorityLevel = 9;
        status = "ACTIVE";
        incidentTimer = 0;
        next = NULL;
    }
};

struct Crew {
    string employeeID;
    string name;
    string role;
    string assignedCallsign;
    Crew* next;

    Crew() {
        next = NULL;
    }
};

struct MissingFlight {
    string callsign;
    string lastKnownSector;
    int tickWindow;
    int credThreshold;
    int spawnTick;
    bool found;
    MissingFlight* next;

    MissingFlight() {
        tickWindow = 9;
        credThreshold = 50;
        spawnTick = 0;
        found = false;
        next = NULL;
    }
};

struct ScenarioEvent {
    int tick;
    string eventType;
    string target;
    bool triggered;
};

struct Route {
    string sectors[MAX_SECTORS];
    int count;
    int totalCost;

    Route() {
        count = 0;
        totalCost = 99999;
    }
};

#endif
