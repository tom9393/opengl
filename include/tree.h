#ifndef SHADERNODE
#define SHADERNODE

#include "../pch.h"

#include "drawable.h"
#include "mappedlight.h"
#include "shader.h"
#include "drawable.h"
#include "batch.h"

template <typename T>
class Tree {
public:
    Tree() {};
    void addBefore(Tree<T>* node)
    {
        before = node;
    }

    void addAfter(Tree<T>* node)
    {
        after = node;
    }

    void addEntry(T &entry)
    {
        entries.push_back(entry);
    }

    void addEntry(std::vector<T> &vec)
    {
        entries.insert(this->entries.end(), vec.begin(), vec.end());
    }

    void addTop(T& entry) {
        Tree<T>* node = this;
        while (node->after != nullptr) {
            node = node->after;
        }
        node->after = new Tree<T>();
        node->after->addEntry(entry);
    }

    void addTop(std::vector<T>& vec) {
        Tree<T>* node = this;
        while (node->after != nullptr) {
            node = node->after;
        }
        node->after = new Tree<T>();
        node->after->addEntry(vec);
    }

    void addBottom(T& entry) {
        Tree<T>* node = this;
        while (node->before != nullptr) {
            node = node->before;
        }
        node->before = new Tree<T>();
        node->before->addEntry(entry);
    }

    void addBottom(std::vector<T>& vec) {
        Tree<T>* node = this;
        while (node->before != nullptr) {
            node = node->before;
        }
        node->before = new Tree<T>();
        node->before->addEntry(vec);
    }

    std::vector<T> traverse() {
        std::vector<T> result;
        if (before) 
        {
            std::vector<T> before = before->traverse();
            if (!before.empty()) result.insert(result.end(), before.begin(), before.end());
        }
        result.insert(result.end(), entries.begin(), entries.end());
        if (after)
        {
            std::vector<T> after = after->traverse();
            if (!after.empty()) result.insert(result.end(), after.begin(), after.end());
        }
        return result;
    }

    std::vector<T> entries;
    Tree* before = nullptr;
    Tree* after = nullptr;
};

#endif
