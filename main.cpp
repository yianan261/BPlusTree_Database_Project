#include "BPlusTree.h"
#include <iostream>

int main() {
    BPlusTree tree;
    tree.insert(10, "ten");
    tree.insert(20, "twenty");
    tree.insert(5, "five");

    tree.print();
    std::cout << "Search 10: " << tree.search(10) << std::endl;

    tree.remove(10);
    tree.printLeaves();

    return 0;
}
