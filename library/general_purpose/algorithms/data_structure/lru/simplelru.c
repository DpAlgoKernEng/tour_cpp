#include <stdio.h>
#include <stdlib.h>

// 定义双向链表节点结构
typedef struct DLinkedNode {
    int key;
    int value;
    struct DLinkedNode* prev;
    struct DLinkedNode* next;
} DLinkedNode;

// 定义LRU缓存结构
typedef struct {
    int size;               // 当前缓存大小
    int capacity;           // 缓存容量
    DLinkedNode* head;      // 虚拟头节点
    DLinkedNode* tail;      // 虚拟尾节点
    DLinkedNode** hashMap;  // 哈希表用于O(1)查找
} LRUCache;

// 创建新的双向链表节点
DLinkedNode* createNode(int key, int value) {
    DLinkedNode* node = (DLinkedNode*)malloc(sizeof(DLinkedNode));
    node->key = key;
    node->value = value;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// 初始化LRU缓存
LRUCache* lRUCacheCreate(int capacity) {
    LRUCache* cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->size = 0;
    cache->capacity = capacity;

    // 创建虚拟头尾节点
    cache->head = createNode(0, 0);
    cache->tail = createNode(0, 0);
    cache->head->next = cache->tail;
    cache->tail->prev = cache->head;

    // 初始化哈希表
    cache->hashMap = (DLinkedNode**)calloc(10001, sizeof(DLinkedNode*));
    return cache;
}

// 将节点移动到链表头部（最近使用）
void moveToHead(LRUCache* cache, DLinkedNode* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->prev = cache->head;
    node->next = cache->head->next;
    cache->head->next->prev = node;
    cache->head->next = node;
}

// 删除链表尾部节点（最久未使用）
DLinkedNode* removeTail(LRUCache* cache) {
    DLinkedNode* node = cache->tail->prev;
    node->prev->next = cache->tail;
    cache->tail->prev = node->prev;
    return node;
}

// 添加节点到链表头部
void addToHead(LRUCache* cache, DLinkedNode* node) {
    node->prev = cache->head;
    node->next = cache->head->next;
    cache->head->next->prev = node;
    cache->head->next = node;
}

// 获取缓存中的值
int lRUCacheGet(LRUCache* cache, int key) {
    DLinkedNode* node = cache->hashMap[key];
    if (node == NULL) {
        return -1;
    }
    moveToHead(cache, node);
    return node->value;
}

// 放入新的键值对
void lRUCachePut(LRUCache* cache, int key, int value) {
    DLinkedNode* node = cache->hashMap[key];

    if (node == NULL) {
        // 创建新节点
        DLinkedNode* newNode = createNode(key, value);
        cache->hashMap[key] = newNode;
        addToHead(cache, newNode);
        cache->size++;

        // 如果超出容量，删除最久未使用的节点
        if (cache->size > cache->capacity) {
            DLinkedNode* tail = removeTail(cache);
            cache->hashMap[tail->key] = NULL;
            free(tail);
            cache->size--;
        }
    } else {
        // 更新已存在节点的值
        node->value = value;
        moveToHead(cache, node);
    }
}

// 释放LRU缓存
void lRUCacheFree(LRUCache* cache) {
    DLinkedNode* curr = cache->head;
    while (curr != NULL) {
        DLinkedNode* next = curr->next;
        free(curr);
        curr = next;
    }
    free(cache->hashMap);
    free(cache);
}

// 测试主函数
int main() {
    // 创建容量为2的LRU缓存
    LRUCache* cache = lRUCacheCreate(2);

    // 测试用例
    lRUCachePut(cache, 1, 1);    // 缓存是 {1=1}
    lRUCachePut(cache, 2, 2);    // 缓存是 {1=1, 2=2}
    printf("%d\n", lRUCacheGet(cache, 1));    // 返回 1
    lRUCachePut(cache, 3, 3);    // 删除 key 2，缓存是 {1=1, 3=3}
    printf("%d\n", lRUCacheGet(cache, 2));    // 返回 -1 (未找到)
    lRUCachePut(cache, 4, 4);    // 删除 key 1，缓存是 {4=4, 3=3}
    printf("%d\n", lRUCacheGet(cache, 1));    // 返回 -1 (未找到)
    printf("%d\n", lRUCacheGet(cache, 3));    // 返回 3
    printf("%d\n", lRUCacheGet(cache, 4));    // 返回 4

    // 释放缓存
    lRUCacheFree(cache);
    return 0;
}