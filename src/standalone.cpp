#include "standalone.hpp"

void destroyNode(JSONNode* node)
{
    // Check for child first then for next node

    JSONNode* current_node = node;
    JSONNode* next_node; // Initialise a pointer variable for the next node to move onto

    // First check if the current node has a next node
    // And if so then make it the next node of the previous node
    if(current_node->next)
    {
        if(current_node->previous)
        {
            current_node->previous->next = current_node->next;
            current_node->next = nullptr;
            current_node->previous = nullptr;
        }
        else
        {
            // No need to check if there current node has a next node as root node will never have a next node
            current_node->parent->child = current_node->next;
            current_node->next = nullptr;
            current_node->parent = nullptr;
        }
    }

    while(true)
    {
        if(current_node->child)
        {
            current_node = current_node->child;
            continue;
        }
        if(current_node->next)
        {
            current_node = current_node->next;
            continue;
        }

        // The current node now has no children and no next nodes
        if(current_node == node)
        {
            // We have arrived back at the start point. Delete it and remove any references to it
            // Not sure if this is actually nevessary as the code above us should take care of it
            if(current_node->previous)
            {
                if(current_node->next)
                    current_node->previous->next = current_node->next;
                else
                    current_node->previous->next = nullptr;
            }

            if(node->parent && node->parent->child == node)
                node->parent->child = nullptr;

            delete current_node;
            node = nullptr;
            return;
        }
        else if(current_node->previous)  // If we have a previous node move onto that
        {
            next_node = current_node->previous;
            next_node->next = nullptr; // Remove pointer to next node from previous node as it will be deleted
        }
        else if(current_node->parent) // If the current node has a parent switch to the parent (remember: the root node does not have a parent)
        {
            next_node = current_node->parent;
            next_node->child = nullptr; // Remove pointer to child node from parent node as it will be deleted
        }
        else
        {
            // If we have no previous and no parent nodes we are at the root node which can now be safely deleted
            delete current_node;
            node = nullptr;
        }

        delete current_node;
        current_node = next_node;
    }
}