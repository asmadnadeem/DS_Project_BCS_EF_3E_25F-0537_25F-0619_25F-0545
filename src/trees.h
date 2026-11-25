#ifndef TREES_H
#define TREES_H

#include "structs.h"

// ---------------- MODULE 4: BST FLIGHT MANIFEST ----------------
struct BSTNode {
    Flight* data;
    BSTNode* left;
    BSTNode* right;
    BSTNode(Flight* f) { data = f; left = NULL; right = NULL; }
};

class BSTflight {
private:
    BSTNode* root;

    BSTNode* insertRec(BSTNode* node, Flight* f) {
        if (node == NULL) return new BSTNode(f);
        if (f->callsign < node->data->callsign)
            node->left = insertRec(node->left, f);
        else if (f->callsign > node->data->callsign)
            node->right = insertRec(node->right, f);
        return node;
    }

    Flight* searchRec(BSTNode* node, string key) {
        if (node == NULL) return NULL;
        if (key == node->data->callsign) return node->data;
        if (key < node->data->callsign) return searchRec(node->left, key);
        return searchRec(node->right, key);
    }

    BSTNode* findMin(BSTNode* node) {
        while (node != NULL && node->left != NULL) node = node->left;
        return node;
    }

    BSTNode* removeRec(BSTNode* node, string key) {
        if (node == NULL) return NULL;
        if (key < node->data->callsign) node->left = removeRec(node->left, key);
        else if (key > node->data->callsign) node->right = removeRec(node->right, key);
        else {
            if (node->left == NULL) {
                BSTNode* tmp = node->right;
                delete node;
                return tmp;
            } else if (node->right == NULL) {
                BSTNode* tmp = node->left;
                delete node;
                return tmp;
            }
            BSTNode* tmp = findMin(node->right);
            node->data = tmp->data;
            node->right = removeRec(node->right, tmp->data->callsign);
        }
        return node;
    }

    void inorderRec(BSTNode* node) {
        if (node == NULL) return;
        inorderRec(node->left);
        cout << node->data->callsign << " (" << node->data->airline
             << ", Sector: " << node->data->currentSector << ")\n";
        inorderRec(node->right);
    }

    int heightRec(BSTNode* node) {
        if (node == NULL) return 0;
        int lh = heightRec(node->left);
        int rh = heightRec(node->right);
        return 1 + (lh > rh ? lh : rh);
    }

    void destroyRec(BSTNode* node) {
        if (node == NULL) return;
        destroyRec(node->left);
        destroyRec(node->right);
        delete node;
    }

public:
    BSTflight() { root = NULL; }
    ~BSTflight() { destroyTree(); }

    void insert(Flight* f) { root = insertRec(root, f); }
    Flight* search(string callsign) { return searchRec(root, callsign); }
    void remove(string callsign) { root = removeRec(root, callsign); }
    void inorderTraversal() { inorderRec(root); }
    int height() { return heightRec(root); }
    void destroyTree() { destroyRec(root); root = NULL; }
};

// ---------------- MODULE 5: AVL BALANCED FLIGHT INDEX ----------------
struct AVLNode {
    Flight* data;
    AVLNode* left;
    AVLNode* right;
    int h;
    AVLNode(Flight* f) { data = f; left = NULL; right = NULL; h = 1; }
};

class AVLflight {
private:
    AVLNode* root;

    int getH(AVLNode* n) { return (n == NULL) ? 0 : n->h; }
    int maxVal(int a, int b) { return (a > b) ? a : b; }
    int getBal(AVLNode* n) { return (n == NULL) ? 0 : getH(n->left) - getH(n->right); }

    AVLNode* rotateRight(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* t2 = x->right;
        x->right = y;
        y->left = t2;
        y->h = maxVal(getH(y->left), getH(y->right)) + 1;
        x->h = maxVal(getH(x->left), getH(x->right)) + 1;
        return x;
    }

    AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* t2 = y->left;
        y->left = x;
        x->right = t2;
        x->h = maxVal(getH(x->left), getH(x->right)) + 1;
        y->h = maxVal(getH(y->left), getH(y->right)) + 1;
        return y;
    }

    AVLNode* rotateLeftRight(AVLNode* n) {
        n->left = rotateLeft(n->left);
        return rotateRight(n);
    }

    AVLNode* rotateRightLeft(AVLNode* n) {
        n->right = rotateRight(n->right);
        return rotateLeft(n);
    }

    AVLNode* insertRec(AVLNode* node, Flight* f) {
        if (node == NULL) return new AVLNode(f);
        if (f->callsign < node->data->callsign)
            node->left = insertRec(node->left, f);
        else if (f->callsign > node->data->callsign)
            node->right = insertRec(node->right, f);
        else
            return node;

        node->h = 1 + maxVal(getH(node->left), getH(node->right));
        int bal = getBal(node);

        if (bal > 1 && f->callsign < node->left->data->callsign)
            return rotateRight(node);
        if (bal < -1 && f->callsign > node->right->data->callsign)
            return rotateLeft(node);
        if (bal > 1 && f->callsign > node->left->data->callsign)
            return rotateLeftRight(node);
        if (bal < -1 && f->callsign < node->right->data->callsign)
            return rotateRightLeft(node);

        return node;
    }

    Flight* searchRec(AVLNode* node, string key) {
        if (node == NULL) return NULL;
        if (key == node->data->callsign) return node->data;
        if (key < node->data->callsign) return searchRec(node->left, key);
        return searchRec(node->right, key);
    }

    AVLNode* findMin(AVLNode* node) {
        while (node->left != NULL) node = node->left;
        return node;
    }

    AVLNode* removeRec(AVLNode* node, string key) {
        if (node == NULL) return NULL;
        if (key < node->data->callsign) node->left = removeRec(node->left, key);
        else if (key > node->data->callsign) node->right = removeRec(node->right, key);
        else {
            if (node->left == NULL || node->right == NULL) {
                AVLNode* tmp = (node->left != NULL) ? node->left : node->right;
                delete node;
                return tmp;
            }
            AVLNode* tmp = findMin(node->right);
            node->data = tmp->data;
            node->right = removeRec(node->right, tmp->data->callsign);
        }
        node->h = 1 + maxVal(getH(node->left), getH(node->right));
        int bal = getBal(node);
        if (bal > 1 && getBal(node->left) >= 0) return rotateRight(node);
        if (bal > 1 && getBal(node->left) < 0) return rotateLeftRight(node);
        if (bal < -1 && getBal(node->right) <= 0) return rotateLeft(node);
        if (bal < -1 && getBal(node->right) > 0) return rotateRightLeft(node);
        return node;
    }

    void inorderRec(AVLNode* node) {
        if (node == NULL) return;
        inorderRec(node->left);
        cout << node->data->callsign << " ";
        inorderRec(node->right);
    }

    void destroyRec(AVLNode* node) {
        if (node == NULL) return;
        destroyRec(node->left);
        destroyRec(node->right);
        delete node;
    }

public:
    AVLflight() { root = NULL; }
    ~AVLflight() { destroyTree(); }

    void insert(Flight* f) { root = insertRec(root, f); }
    Flight* search(string callsign) { return searchRec(root, callsign); }
    void remove(string callsign) { root = removeRec(root, callsign); }
    void inorderTraversal() { inorderRec(root); cout << endl; }
    int height() { return getH(root); }
    void destroyTree() { destroyRec(root); root = NULL; }

    // 15 alphabetical callsigns benchmark comparison
    void runHeightBenchmark() {
        BSTflight demoBST;
        AVLflight demoAVL;
        Flight dummy[15];
        string keys[15] = {
            "AA-101","AA-102","AA-103","AA-104","AA-105",
            "AA-106","AA-107","AA-108","AA-109","AA-110",
            "AA-111","AA-112","AA-113","AA-114","AA-115"
        };
        for (int i = 0; i < 15; i++) {
            dummy[i].callsign = keys[i];
            demoBST.insert(&dummy[i]);
            demoAVL.insert(&dummy[i]);
        }
        cout << "\n--- BST vs AVL HEIGHT COMPARISON ---\n";
        cout << "Active Flights -> BST Height: " << height() << " | AVL Height: " << height() << endl;
        cout << "15 Sorted Callsigns Demo -> Unbalanced BST Height: " << demoBST.height()
             << " | Self-Balancing AVL Height: " << demoAVL.height() << endl;
    }
};

#endif
