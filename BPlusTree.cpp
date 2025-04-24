#include "BPlusTree.h"
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

const int ORDER = 3;
const int MIN_ALLOWED = (ORDER - 1) / 2;

template<typename K, typename P>
BPlusTree<K, P>::BPlusTree() {
    root = new LeafNode<K, P>();
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
void BPlusTree<K, P>::insert(K &key, const P& attrs) {  // 修改为 const 引用
    if (keySet.count(key)){
        throw runtime_error("Key already exists. Use update instead.");
    }
    keySet.insert(key);

    BPlusNode<K, P>* newChild = nullptr;
    K newKey;
    insertInternal(key, const_cast<P&>(attrs), root, newChild, newKey);  // 临时解决方案
    if (newChild != nullptr) {
        InternalNode<K,P>* newRoot = new InternalNode<K, P>();
        newRoot->getKeys().push_back(newKey);
        newRoot->getChildren().push_back(root);
        newRoot->getChildren().push_back(newChild);
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
    int insert_pos = upper_bound(keys.begin(), keys.end(), key) - keys.begin();
    BPlusNode<K, P>* child = children[insert_pos];
    BPlusNode<K, P>* tempChild = nullptr;
    K tempKey;
    insertInternal(key, attrs, child, tempChild, tempKey);
    if(tempChild){
        auto* internal = static_cast<InternalNode<K, P>*>(node);
        internal->getKeys().insert(keys.begin() + insert_pos, tempKey);
        internal->getChildren().insert(children.begin()+insert_pos+1, tempChild);
        if (internal->getKeys().size() >= ORDER){
            splitInternal(node, newChild, newKey);
        }
    }

}

template<typename K, typename P>
void BPlusTree<K, P>::splitLeaf(BPlusNode<K, P>* node, BPlusNode<K, P>*& newChild, K& newKey) {
    // Split a full leaf node into two and maintain linked list connections.
    // Assign newChild and newKey for parent node to handle.
    LeafNode<K, P>* newLeaf = new LeafNode<K, P>();
    auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
    auto& newEntries = newLeaf->getEntries();

    int mid = ORDER/2;
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
    // TODO: Split a full internal node and promote the middle key to the parent.
    // Assign newChild and newKey accordingly.
    InternalNode<K, P>* newInternal = new InternalNode<K, P>();
    auto& keys = node->getKeys();
    auto& children = node->getChildren();
    auto& newKeys = newInternal->getKeys();
    auto& newChildren = newInternal->getChildren();

    int mid = ORDER/2;
    newKey = keys[mid];
    
    newKeys.assign(keys.begin() + mid + 1, keys.end());
    newChildren.assign(children.begin() + mid + 1, children.end());

    keys.resize(mid);
    children.resize(mid+1);

    newChild = newInternal;
}

template<typename K, typename P>
void BPlusTree<K, P>::remove(K &key) {
    // TODO: Call deleteEntry. If root becomes empty, demote it to its only child.
    // Zirui
    if (deleteEntry(root, key)){
        keySet.erase(key);
        if(!root->isLeafNode() && root->getKeys().empty()) {
            root = root->getChildren().front();
        } 
    }

}

template<typename K, typename P>
bool BPlusTree<K, P>::deleteEntry(BPlusNode<K, P>* node, const K &key) {
    // TODO: Delete key from leaf or recurse into internal node.
    // Handle underflow in leaf by borrowing or merging.
    // Return true if deletion happened, false otherwise.
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
    int childIndex = upper_bound(keys.begin(), keys.end(), key) - keys.begin();
    return deleteEntry(children[childIndex], key);
}

template<typename K, typename P>
void BPlusTree<K, P>::borrowLeaf(BPlusNode<K, P>* node) {
    // Borrow entry from sibling leaf (either prev or next).
    // If borrowing is not possible, fallback to merging.
    auto* leaf = static_cast<LeafNode<K, P>*>(node);
    auto* prev = node->getPrev(); //BPlusNode<K, P>* base pointer
    auto* next = node->getNext();
    // check left sibling to borrow, convert to LeafNode()
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
    // Merge current leaf node with its sibling.
    // Update linked list pointers and delete the node.
    auto* leaf = static_cast<LeafNode<K, P>*>(node);
    auto* prev = leaf->getPrev();
    auto* next = leaf->getNext();
    // check previous or next leaf to merge
    if (prev) {
        auto& prevEntries = static_cast<LeafNode<K, P>*>(prev)->getEntries();
        auto& entries = leaf->getEntries();
        prevEntries.insert(prevEntries.end(), entries.begin(), entries.end());
        prev->setNext(next);
        if (next) next->setPrev(prev);
        delete leaf;
    } 
    else if (next) {
        auto& nextEntries = static_cast<LeafNode<K, P>*>(next)->getEntries();
        auto& entries = leaf->getEntries();
        nextEntries.insert(nextEntries.begin(), entries.begin(), entries.end());
        next->setPrev(nullptr);
        delete leaf;
    }
    
}

template<typename K, typename P>
P BPlusTree<K, P>::search(K &key) {
    // Traverse the tree to find the leaf node.
    // Then search through entries to find the value.
    BPlusNode<K, P>* node = root;
    while (!node->isLeafNode()) {
        auto& keys = node->getKeys();
        auto& children = node->getChildren();
        int idx = upper_bound(keys.begin(), keys.end(), key) - keys.begin();
        node = children[idx];
    }

    auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
    for (const auto& entry : entries) {
        if (entry.key == key) return entry.attrs;
    }
    return {}; // if no entry found
}

template<typename K, typename P>
std::vector<P> BPlusTree<K, P>::rangeQuery(K lowKey, K highKey) {
    vector<P> result;

    BPlusNode<K, P>* node = root;
    while (!node->isLeafNode()) {
        auto& keys = node->getKeys();
        auto& children = node->getChildren();
        int idx = upper_bound(keys.begin(), keys.end(), lowKey) - keys.begin();
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


template<typename K, typename P>
bool BPlusTree<K, P>::contains(K &key) const{
    return keySet.count(key) > 0;
}

template<typename K, typename P>
void BPlusTree<K, P>::print() {
    // TODO: Print the entire B+ tree structure for debugging.
    printTree(root, 0);
}

template<typename K, typename P>
void BPlusTree<K, P>::printTree(BPlusNode<K, P>* node, int level) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null.");
    }
    cout << string(level * 4, ' ');
    if (node->isLeafNode()) {
        auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
        cout << "Leaf: ";
        for (const auto& e : entries) {
            cout << "(" << e.key << ", ";
            if (!e.attrs.empty()) {
                if (std::is_same<P, std::vector<std::string>>::value)
                    cout << e.attrs[0];
                else if (std::is_same<P, std::vector<int>>::value)
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
    //Traverse to the leftmost leaf and print all entries left-to-right.
    BPlusNode<K, P>* node = root;
    while (!node->isLeafNode()) {
        node = node->getChildren().front();
    }
    while (node) {
        auto& entries = static_cast<LeafNode<K, P>*>(node)->getEntries();
        for (const auto& e : entries) {
            cout << "(" << e.key << ", ";
            if (!e.attrs.empty()) {
                if (std::is_same<P, std::vector<std::string>>::value)
                    cout << e.attrs[0];
                else if (std::is_same<P, std::vector<int>>::value)
                    cout << e.attrs[0];
            }
            cout << ") ";
        }
        node = node->getNext();
    }
    cout << endl;
}

// template<typename K, typename P>
// template<typename Fn>
// void BPlusTree<K,P>::forEachLeaf(Fn&& f) const
// {
//     const BPlusNode<K,P>* n = root;
//     while (!n->isLeafNode()) n = n->getChildren().front();
//     while (n) {
//         for (const auto& e : static_cast<const LeafNode<K,P>*>(n)->getEntries()) f(e);
//         n = n->getNext();
//     }
// }


template class BPlusTree<int, vector<string>>;


template class BPlusTree<string, vector<int>>;