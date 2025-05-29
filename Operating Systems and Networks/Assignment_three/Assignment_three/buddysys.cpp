//Denys Pedan
//ID: 23011350
//Date: 29/05/2025

#include "buddysys.h"
#include <cmath>
#include <vector>
#include <cstdint>

static int maxLevel = 0;
static std::vector<Node*> freeLists;
static bool initialized = false;
static uintptr_t baseAddr = 0;

static long long nextPowerOfTwo(long long x) {
    if (x <= 1) return 1;
    return 1LL << (long long)std::ceil(std::log2(x));
}

static void initBuddy() {
    if (initialized) return;
    if (!wholememory || MEMORYSIZE <= (long long)sizeof(Node)) return;
    maxLevel = (int)std::ceil(std::log2((double)MEMORYSIZE));
    freeLists.assign(maxLevel + 1, nullptr);
    Node* node = wholememory;
    node->size = MEMORYSIZE - sizeof(Node);
    node->alloc = 0;
    node->next = nullptr;
    node->previous = nullptr;
    baseAddr = reinterpret_cast<uintptr_t>(wholememory);
    freeLists[maxLevel] = node;
    initialized = true;
}

static void removeFromFreeList(Node* node, int level) {
    if (!node || level < 0 || level > maxLevel) return;
    if (node->previous) node->previous->next = node->next;
    if (node->next) node->next->previous = node->previous;
    if (freeLists[level] == node) freeLists[level] = node->next;
    node->next = node->previous = nullptr;
}

static void addToFreeList(Node* node, int level) {
    if (!node || level < 0 || level > maxLevel) return;
    node->alloc = 0;
    node->next = freeLists[level];
    node->previous = nullptr;
    if (freeLists[level]) freeLists[level]->previous = node;
    freeLists[level] = node;
}

void *buddyMalloc(int request_memory) {
    if (request_memory <= 0) return nullptr;
    initBuddy();
    if (!initialized) return nullptr;
    if ((long long)request_memory > MEMORYSIZE - (long long)sizeof(Node)) return nullptr;
    long long totalSize = request_memory + sizeof(Node);
    int targetLevel = (int)std::ceil(std::log2((double)totalSize));
    if (targetLevel > maxLevel) return nullptr;
    int level = targetLevel;
    while (level <= maxLevel && freeLists[level] == nullptr) level++;
    if (level > maxLevel) return nullptr;
    Node* node = freeLists[level];
    removeFromFreeList(node, level);
    while (level > targetLevel) {
        level--;
        long long sz = 1LL << level;
        Node* buddy1 = node;
        buddy1->size = sz - sizeof(Node);
        buddy1->alloc = 0;
        buddy1->next = buddy1->previous = nullptr;
        uintptr_t addr = reinterpret_cast<uintptr_t>(node);
        Node* buddy2 = reinterpret_cast<Node*>(addr + sz);
        buddy2->size = sz - sizeof(Node);
        buddy2->alloc = 0;
        buddy2->next = buddy2->previous = nullptr;
        addToFreeList(buddy2, level);
        node = buddy1;
    }
    node->alloc = 1;
    unsigned char* dataPtr = reinterpret_cast<unsigned char*>(node) + sizeof(Node);
    return reinterpret_cast<void*>(dataPtr);
}

int buddyFree(void *p) {
    if (!p || !initialized) return 0;
    uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
    if (ptr < baseAddr + sizeof(Node) || ptr >= baseAddr + MEMORYSIZE) return 0;
    unsigned char* rawPtr = reinterpret_cast<unsigned char*>(p) - sizeof(Node);
    Node* node = reinterpret_cast<Node*>(rawPtr);
    if (node->alloc == 0) return 0;
    long long blockSize = node->size + sizeof(Node);
    int level = (int)std::log2((double)blockSize);
    if (level < 0 || level > maxLevel) return 0;
    node->alloc = 0;
    uintptr_t addr = reinterpret_cast<uintptr_t>(node);
    while (level < maxLevel) {
        uintptr_t offset = addr - baseAddr;
        uintptr_t buddyOffset = offset ^ (1LL << level);
        Node* buddy = reinterpret_cast<Node*>(baseAddr + buddyOffset);
        if (!buddy || buddy->alloc != 0 || buddy->size + sizeof(Node) != (1LL << level)) break;
        removeFromFreeList(buddy, level);
        addr = std::min(addr, reinterpret_cast<uintptr_t>(buddy));
        node = reinterpret_cast<Node*>(addr);
        level++;
        node->size = (1LL << level) - sizeof(Node);
        node->alloc = 0;
    }
    addToFreeList(node, level);
    return 1;
}
