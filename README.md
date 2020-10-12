# File-Compression-C
(Class: Systems Programming) - Partner Project

This program takes in a file or a directory and creates a min heap with the words in those files. It then creates a Huffman Codebook, with the most used words assigned the least amount of bits. To compress the files, the given code book is stored as a linked list and the tokens from the to be compressed files are paired with their corresponding bits. To decode files, the given code book is read and a binary search tree is created from the bits with the node going left representing a zero and the node going right representing a one. Then, each bit in the compressed file is read one by one following the tree and once it hits a word, that chunk of bits is decompressed back to its corresponding word.

A more in detail readme.pdf is included in the repository.
