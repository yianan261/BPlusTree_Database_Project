#include "BPlusTree.h"
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

const int ORDER = 3;
const int MIN_ALLOWED = (ORDER - 1) / 2;

template<typename K, typename P>
BPlusTree<K, P>::BPlusTree() : root(new LeafNode<K, P>()), treeSize(0) {
    root->setParent(nullptr);  // Initialize root's parent
}

template<typename K, typename P>
void BPlusTree<K, P>::update(K& key, P& attrs){
    if(!keySet.count(key)){
        throw runtime_error("Key does not exist. Use create instead.");
    }
    remove(key);
    BPlusNode<K, P>* tempChild = nullptr;
    K tempKey;
    insertInternal(key, attrs, root, tempChild, tempKey);
}

template<typename K, typename P>
void BPlusTree<K, P>::insert(K &key, const P& attrs) { 
    if (keySet.count(key)){
        throw runtime_error("Key already exists. Use update instead.");
    }
    keySet.insert(key);
    treeSize++;  

    BPlusNode<K, P>* newChild = nullptr;
    K newKey;
    insertInternal(key, const_cast<P&>(attrs), root, newChild, newKey);  // 临时解决方案
    if (newChild != nullptr) {
        InternalNode<K,P>* newRoot = new InternalNode<K, P>();
        newRoot->setParent(nullptr);  // Root has no parent
        newRoot->getKeys().push_back(newKey);
        newRoot->getChildren().push_back(root);
        newRoot->getChildren().push_back(newChild);
        
        // Update parent pointers
        root->setParent(newRoot);
        newChild->setParent(newRoot);
        
        root = newRoot;
    }
}

template<typename K, typename P>
void BPlusTree<K, P>::insertInternal(K& key, P& attrs, BPlusNode<K, P>* node, BPlusNode<K, P>*& newChild, K& newKey){
    if (node->isLeafNode()){
        auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
        //lower_bound(start, end, value, comparator)
        auto insert_pos = lower_bound(entries.begin(), entries.end(), key, [](const Entry<K, P>& a, const K& b){
            return a.key < b;  
        });
        entries.insert(insert_pos, {key, attrs});
        if(entries.size() >= ORDER){
            splitLeaf(node, newChild, newKey);
        }
        return;
    }
    auto& keys = node->getKeys();
    auto& children = node->getChildren();
    size_t insert_pos = static_cast<size_t>(upper_bound(keys.begin(), keys.end(), key) - keys.begin());
    BPlusNode<K, P>* child = children[insert_pos];
    BPlusNode<K, P>* tempChild = nullptr;
    K tempKey;
    insertInternal(key, attrs, child, tempChild, tempKey);
    if(tempChild){
        auto* internal = static_cast<InternalNode<K, P>*>(node);
        internal->getKeys().insert(keys.begin() + insert_pos, tempKey);
        internal->getChildren().insert(children.begin()+insert_pos+1, tempChild);
        tempChild->setParent(node);  // Set parent for new child
        if (internal->getKeys().size() >= ORDER){
            splitInternal(node, newChild, newKey);
        }
    }
}

template<typename K, typename P>
void BPlusTree<K, P>::splitLeaf(BPlusNode<K, P>* node, BPlusNode<K, P>*& newChild, K& newKey) {
    // Split a full leaf node into two (keep LL connections)
    // Assign newChild and newKey for parent node to handle.
    LeafNode<K, P>* newLeaf = new LeafNode<K, P>();
    newLeaf->setParent(node->getParent());  // Set parent for new leaf
    
    auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
    auto& newEntries = newLeaf->getEntries();

    size_t mid = static_cast<size_t>(ORDER/2);
    newEntries.assign(entries.begin()+mid, entries.end());
    entries.resize(mid);

    newKey = newEntries.front().key; // promote to parent

    newLeaf->setNext(node->getNext());
    if(node->getNext()) node->getNext()->setPrev(newLeaf);
    newLeaf->setPrev(node);
    node->setNext(newLeaf);

    newChild = newLeaf;
}

template<typename K, typename P>
void BPlusTree<K, P>::splitInternal(BPlusNode<K, P>* node, BPlusNode<K, P>*& newChild, K& newKey) {
    // TODO: Split a full internal node and promote middle key to the parent
    // Assign newChild and newKey
    InternalNode<K, P>* newInternal = new InternalNode<K, P>();
    newInternal->setParent(node->getParent());  // Set parent for new internal node
    
    auto& keys = node->getKeys();
    auto& children = node->getChildren();
    auto& newKeys = newInternal->getKeys();
    auto& newChildren = newInternal->getChildren();

    size_t mid = static_cast<size_t>(ORDER/2);
    newKey = keys[mid];
    
    newKeys.assign(keys.begin() + mid + 1, keys.end());
    newChildren.assign(children.begin() + mid + 1, children.end());

    keys.resize(mid);
    children.resize(mid+1);

    // Update parent pointers for moved children
    for (auto child : newChildren) {
        child->setParent(newInternal);
    }

    newChild = newInternal;
}

template<typename K, typename P>
void BPlusTree<K, P>::remove(K &key) {
    // Zirui
    if (deleteEntry(root, key)){
        keySet.erase(key);
        treeSize--; 
        if(!root->isLeafNode() && root->getKeys().empty()) {
            BPlusNode<K,P>* newRoot = root->getChildren().front();
            newRoot->setParent(nullptr);
            delete root;
            root = newRoot;
        } 
    }
}

template<typename K, typename P>
bool BPlusTree<K, P>::deleteEntry(BPlusNode<K, P>* node, const K &key) {
    // TODO: Delete key from leaf or recurse into internal node
    // Handle underflow in leaf by borrow or merge
    // Return true if delete happened, else false
    //Zirui
    if (node->isLeafNode()) {
        auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
        auto it = find_if(entries.begin(), entries.end(), [key](const Entry<K, P>& e) { return e.key == key; });
        if (it != entries.end()) {
            entries.erase(it);
            if (entries.size() < MIN_ALLOWED) {
                borrowLeaf(node);
            }
            return true;
        }
        return false;
    }

    auto& keys = node->getKeys();
    auto& children = node->getChildren();
    size_t childIndex = static_cast<size_t>(upper_bound(keys.begin(), keys.end(), key) - keys.begin());
    return deleteEntry(children[childIndex], key);
}

template<typename K, typename P>
void BPlusTree<K, P>::borrowLeaf(BPlusNode<K, P>* node) {
    // Borrow entry from sibling leaf (either prev or next).
    // If cannot borrow, merge
    auto* leaf = static_cast<LeafNode<K, P>*>(node);
    auto* prev = leaf->getPrev(); //BPlusNode<K, P>* base pointer
    auto* next = leaf->getNext();
    
    // Try left sibling first
    if (prev && static_cast<LeafNode<K, P>*>(prev)->getEntries().size() > MIN_ALLOWED){
        auto& prevEntries = static_cast<LeafNode<K, P>*>(prev)->getEntries();
        auto& entries = leaf->getEntries();
        entries.insert(entries.begin(), prevEntries.back()); // insert to front
        prevEntries.pop_back();
        return;
    } // check right sibling to borrow
    else if(next && static_cast<LeafNode<K, P>*>(next)->getEntries().size() > MIN_ALLOWED){
        auto& nextEntries = static_cast<LeafNode<K, P>*>(next)->getEntries();
        auto& entries = leaf->getEntries();
        entries.push_back(nextEntries.front()); // insert to back
        nextEntries.erase(nextEntries.begin());
        return;
    }
    mergeLeaf(node); // if cannot borrow, merge current leaf with prev or next sibling
}

template<typename K, typename P>
void BPlusTree<K, P>::mergeLeaf(BPlusNode<K, P>* node) {
    // Merge curr leaf node with sibling.
    // Update LL pointers and delete unused node
    // use static_case because we've checked the node is leaf node
    auto* leaf = static_cast<LeafNode<K, P>*>(node);
    auto* prev = leaf->getPrev();
    auto* next = leaf->getNext();
    auto* parent = findParent(node);
    
    // Calculate merge capacities
    bool canMergeWithPrev = prev && (prev->getEntries().size() + leaf->getEntries().size() <= ORDER-1);
    bool canMergeWithNext = next && (next->getEntries().size() + leaf->getEntries().size() <= ORDER-1);
    
    if (canMergeWithPrev) {
        auto& prevEntries = static_cast<LeafNode<K, P>*>(prev)->getEntries();
        auto& entries = leaf->getEntries();
        prevEntries.insert(prevEntries.end(), entries.begin(), entries.end());
        
        prev->setNext(next);
        if (next) next->setPrev(prev);
        
        updateParentAfterMerge(parent, prev, leaf);
        delete leaf;
    } 
    else if (canMergeWithNext) {
        auto& nextEntries = static_cast<LeafNode<K, P>*>(next)->getEntries();
        auto& entries = leaf->getEntries();
        nextEntries.insert(nextEntries.begin(), entries.begin(), entries.end());
        
        if (prev) prev->setNext(next);
        next->setPrev(prev);
        
        updateParentAfterMerge(parent, next, leaf);
        delete leaf;
    }
}

template<typename K, typename P>
void BPlusTree<K,P>::borrowOrMergeInternal(BPlusNode<K,P>* node) {
    auto* parent = findParent(node);
    if (!parent) return;

    size_t nodeIndex = 0;
    bool found = false;
    auto& parentChildren = parent->getChildren();
    for (size_t i = 0; i < parentChildren.size(); ++i) {
        if (parentChildren[i] == node) {
            nodeIndex = i;
            found = true;
            break;
        }
    }
    if (!found) return;

    // Try left sibling first
    if (nodeIndex > 0) {
        auto* leftSibling = static_cast<InternalNode<K,P>*>(parentChildren[nodeIndex-1]);
        if (leftSibling->getKeys().size() > MIN_ALLOWED) {
            auto& nodeKeys = node->getKeys();
            auto& nodeChildren = node->getChildren();
            auto& leftKeys = leftSibling->getKeys();
            auto& leftChildren = leftSibling->getChildren();
            
            // Get separating key from parent
            K separatorKey = parent->getKeys()[nodeIndex-1];
            
            // Move rightmost key from left sibling to parent
            parent->getKeys()[nodeIndex-1] = leftKeys.back();
            
            // Move separator key to node's front
            nodeKeys.insert(nodeKeys.begin(), separatorKey);
            
            // Move rightmost child from left sibling to node
            if (!leftChildren.empty()) {
                nodeChildren.insert(nodeChildren.begin(), leftChildren.back());
                leftChildren.back()->setParent(node);
                leftChildren.pop_back();
            }
            
            leftKeys.pop_back();
            return;
        }
    }
    
    // Try right sibling
    if (nodeIndex < parentChildren.size()-1) {
        auto* rightSibling = static_cast<InternalNode<K,P>*>(parentChildren[nodeIndex+1]);
        if (rightSibling->getKeys().size() > MIN_ALLOWED) {
            auto& nodeKeys = node->getKeys();
            auto& nodeChildren = node->getChildren();
            auto& rightKeys = rightSibling->getKeys();
            auto& rightChildren = rightSibling->getChildren();
            
            // Get separating key from parent
            K separatorKey = parent->getKeys()[nodeIndex];
            
            // Move leftmost key from right sibling to parent
            parent->getKeys()[nodeIndex] = rightKeys.front();
            
            // Move separator key to node's back
            nodeKeys.push_back(separatorKey);
            
            // Move leftmost child from right sibling to node
            if (!rightChildren.empty()) {
                nodeChildren.push_back(rightChildren.front());
                rightChildren.front()->setParent(node);
                rightChildren.erase(rightChildren.begin());
            }
            
            rightKeys.erase(rightKeys.begin());
            return;
        }
    }
    
    // If can't borrow, merge with a sibling
    if (nodeIndex > 0) {
        // Merge with left sibling
        auto* leftSibling = static_cast<InternalNode<K,P>*>(parentChildren[nodeIndex-1]);
        auto& nodeKeys = node->getKeys();
        auto& nodeChildren = node->getChildren();
        auto& leftKeys = leftSibling->getKeys();
        auto& leftChildren = leftSibling->getChildren();
        
        // Get separating key from parent
        K separatorKey = parent->getKeys()[nodeIndex-1];
        
        // Move separator key to left sibling
        leftKeys.push_back(separatorKey);
        
        // Move all keys from node to left sibling
        leftKeys.insert(leftKeys.end(), nodeKeys.begin(), nodeKeys.end());
        
        // Move all children from node to left sibling
        for (auto child : nodeChildren) {
            leftChildren.push_back(child);
            child->setParent(leftSibling);
        }
        
        updateParentAfterMerge(parent, leftSibling, node);
        delete node;
    } 
    else if (nodeIndex < parentChildren.size()-1) {
        // Merge with right sibling
        auto* rightSibling = static_cast<InternalNode<K,P>*>(parentChildren[nodeIndex+1]);
        auto& nodeKeys = node->getKeys();
        auto& nodeChildren = node->getChildren();
        auto& rightKeys = rightSibling->getKeys();
        auto& rightChildren = rightSibling->getChildren();
        
        // Get separating key from parent
        K separatorKey = parent->getKeys()[nodeIndex];
        
        // Move separator key to node
        nodeKeys.push_back(separatorKey);
        
        // Move all keys from right sibling to node
        nodeKeys.insert(nodeKeys.end(), rightKeys.begin(), rightKeys.end());
        
        // Move all children from right sibling to node
        for (auto child : rightChildren) {
            nodeChildren.push_back(child);
            child->setParent(node);
        }
        
        updateParentAfterMerge(parent, node, rightSibling);
        delete rightSibling;
    }
}

template<typename K, typename P>
P BPlusTree<K, P>::search(K &key) {
    // Traverse the tree to find leaf node
    // search entries to find val
    BPlusNode<K, P>* node = root;
    while (!node->isLeafNode()) {
        auto& keys = node->getKeys();
        auto& children = node->getChildren();
        size_t idx = static_cast<size_t>(upper_bound(keys.begin(), keys.end(), key) - keys.begin());
        node = children[idx];
    }

    auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
    for (const auto& entry : entries) {
        if (entry.key == key) return entry.attrs;
    }
    return {}; // if no entry found
}

template<typename K, typename P>
vector<P> BPlusTree<K, P>::rangeQuery(K lowKey, K highKey) {
    vector<P> result;

    BPlusNode<K, P>* node = root;
    while (!node->isLeafNode()) {
        auto& keys = node->getKeys();
        auto& children = node->getChildren();
        size_t idx = static_cast<size_t>(upper_bound(keys.begin(), keys.end(), lowKey) - keys.begin());
        node = children[idx];
    }

    while (node) {
        auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
        for (const auto& entry : entries) {
            if (entry.key > highKey) return result; 
            if (entry.key >= lowKey) {
                result.push_back(entry.attrs);
            }
        }
        node = node->getNext(); // go to right sibling
    }

    return result;
}

// finds parent pointer
template<typename K, typename P>
BPlusNode<K,P>* BPlusTree<K,P>::findParent(BPlusNode<K,P>* node) {
    if (node == root) return nullptr;
    
    BPlusNode<K,P>* parent = nullptr;
    BPlusNode<K,P>* current = root;
    
    while (current && current != node) {
        parent = current;
        if (current->isLeafNode()) {
            current = nullptr;
        } else {
            auto& keys = current->getKeys();
            auto& children = current->getChildren();
            size_t idx = static_cast<size_t>(upper_bound(keys.begin(), keys.end(), node->getKeys().front()) - keys.begin());
            current = children[idx];
        }
    }
    
    return (current == node) ? parent : nullptr;
}

template<typename K, typename P>
void BPlusTree<K,P>::updateParentAfterMerge(BPlusNode<K,P>* parent, BPlusNode<K,P>* /*survivingNode*/, BPlusNode<K,P>* mergedNode) {                             
    if (!parent) return;

    auto& parentKeys = parent->getKeys();
    auto& parentChildren = parent->getChildren();

    auto it = find(parentChildren.begin(), parentChildren.end(), mergedNode);
    if (it != parentChildren.end()) {
        size_t index = static_cast<size_t>(it - parentChildren.begin());
        parentChildren.erase(it);
        
        if (index > 0) {
            parentKeys.erase(parentKeys.begin() + index - 1);
        } else if (!parentKeys.empty()) {
            parentKeys.erase(parentKeys.begin());
        }

        // Update parent pointers
        for (auto child : parentChildren) {
            child->setParent(parent);
        }
    }

    // Check if parent underflows
    if (parent != root && parentKeys.size() < MIN_ALLOWED) {
        borrowOrMergeInternal(parent);
    } 
    else if (parent == root && parentKeys.empty()) {
        // Handle root with no keys
        root = parentChildren[0];
        root->setParent(nullptr);
        // Update all children's parent pointers
        for (auto child : root->getChildren()) {
            child->setParent(root);
        }
        delete parent;
    }
}

template<typename K, typename P>
bool BPlusTree<K, P>::contains(K &key) const{
    return keySet.count(key) > 0;
}

template<typename K, typename P>
void BPlusTree<K, P>::print() {
    printTree(root, 0);
}

template<typename K, typename P>
void BPlusTree<K, P>::printTree(BPlusNode<K, P>* node, size_t level) {
    if (node == nullptr) {
        throw runtime_error("Node is null.");
    }
    cout << string(level * 4, ' ');
    if (node->isLeafNode()) {
        auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
        cout << "Leaf: ";
        for (const auto& e : entries) {
            cout << "(" << e.key << ", ";
            if (!e.attrs.empty()) {
                if (is_same<P, vector<string>>::value)
                    cout << e.attrs[0];
                else if (is_same<P, vector<int>>::value)
                    cout << e.attrs[0];
            }
            cout << ") ";
        }
    } else {
        cout << "Internal: ";
        for (const auto& k : node->getKeys()) {
            cout << k << " ";
        }
    }
    cout << endl;
    if (!node->isLeafNode()) {
        for (BPlusNode<K, P>* child : node->getChildren()) {
            printTree(child, level + 1);
        }
    }
}

template<typename K, typename P>
void BPlusTree<K, P>::printLeaves() {
    //go to the leftmost leaf and print all entries(left to right)
    BPlusNode<K, P>* node = root;
    while (!node->isLeafNode()) {
        node = node->getChildren().front();
    }
    while (node) {
        auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
        for (const auto& e : entries) {
            cout << "(" << e.key << ", ";
            if (!e.attrs.empty()) {
                if (is_same<P, vector<string>>::value)
                    cout << e.attrs[0];
                else if (is_same<P, vector<int>>::value)
                    cout << e.attrs[0];
            }
            cout << ") ";
        }
        node = node->getNext();
    }
    cout << endl;
}

template class BPlusTree<int, vector<string>>;
template class BPlusTree<string, vector<int>>;
