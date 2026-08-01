// Node superclass
#ifndef ENGINE_NODE_H
#define ENGINE_NODE_H

#include "Debug.hpp"
#include <cstdint>
#include <format>
#include <set>
#include <string>


/// Represents a functional element within a scene
class Node {

    public:

        /// Unique ID for every Node
        uint64_t UUID;

        /// Node Name
        std::string name;

        /// Node's children
        std::set<Node*> children;

        /// Parent node
        Node *parent;



        /* Default constructor */
        Node() {
            UUID = random(); // TODO: use a better random function
            name = std::format("{} {}", typeid(this).name(), UUID);
            children = std::set<Node*>();
            parent = nullptr;
        }

        /// Called by the Engine when the node is added to the scene
        virtual void onEnter() {}

        /// Called by the Engine when the node is removed from the scene
        virtual void onExit() {}

        /**
         * Adds a node to this node's children and removes it from its previous parent.
         * If the node is already a child of this node, throws a warning and does nothing.
         * @throws std::runtime_error If a null pointer is passed.
         */
        void adopt(Node* child) {
            if (!child)
                error("Node::adopt(): child == NULL");

            if (this->children.contains(child)) {
                warning(std::format("Node {} already contains the child {}", this->UUID, child->UUID));
                return;
            }


            // remove from old parent. NOTE: maybe the logic of removing the child can be put somewhere else
            if (child->parent != nullptr)
                child->parent->disown(child);
            // add to new parent
            child->parent = this;
            this->children.insert(child);
        }

        /**
         * Removes a node from this node's children and adds it to the root's children.
         * If the node is not a child of this node, throws a warning and does nothing
         * @throws std::runtime_error If a null pointer is passed.
         */
        void disown(Node* child) {

            if (!child)
                error("Node::disown(): child == NULL");

            // FIXME: set compares pointers so it never finds the actual object
            if (!this->children.contains(child)) {
                warning(std::format("Node {} is not a child of {}", child->UUID, this->UUID));
                return;
            }


            this->children.erase(child);
            getRoot()->adopt(child);
        }

        /// Returns a pointer to the root node
        Node* getRoot() {

            Node* cur = this;
            while (cur->parent) {
                cur = cur->parent;
            }

            return cur;
        }
};

#endif
