// Node superclass
#ifndef ENGINE_NODE_H
#define ENGINE_NODE_H

#include <list>
#include <string>

/// Represents a functional element within a scene
class Node {
    public:
        /// Unique ID for every Node
        long long unsigned int UUID;

        /// Node Name
        std::string name;

        /// Node's children
        std::set<Node*> children;

        /// Parent node
        Node *parent;

        Node() : UUID(0), name(std::format("Node {}", UUID)), parent(nullptr) {}

        /// Called by the Engine when the node is added to the scene
        virtual void onEnter() {}

        /// Called by the Engine when the node is removed from the scene
        virtual void onExit() {}

        /**
         * Adds a node to this node's children.
         * If the node is already a child of this node, does nothing.
         * @throws std::runtime_error If a null pointer is passed.
         */
        void adopt(Node* child)
        {
            if (!child) throw std::runtime_error("Node::adopt(): child == NULL");
            if (children.contains(child)) return;

            children.insert(child);
            child->parent = this;
        }

        /**
         * Removes a node from this node's children and adds it to the root's children.
         * If the node is not a child of this node, does nothing
         * @throws std::runtime_error If a null pointer is passed.
         */
        void disown(Node* child)
        {
            if (!child) throw std::runtime_error("Node::disown(): child == NULL");
            if (!children.contains(child)) return;

            children.erase(child);
            getRoot()->adopt(child);
        }

        /// Returns a pointer to the root node
        Node* getRoot()
        {
            Node* cur = this;
            while (cur->parent)
            {
                cur = cur->parent;
            }
            return cur;
        }
};

#endif
