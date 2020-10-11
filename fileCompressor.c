#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

typedef struct heapNode
{ // structure for nodes in minHeap
    int frequency;
    char token[1000];
    int isHuffTree;
} hNode;

typedef struct Heap
{ // structure for minHeap
    hNode *heap;
    int count;
    int capacity;
} heap;

typedef struct HuffNode
{ // structure for nodes in Huffman Tree
    int value;
    char token[1000];
    struct HuffNode *left;
    struct HuffNode *right;
} Huffnode;

typedef struct HuffQueueNode
{
    Huffnode *treeNode;
    struct HuffQueueNode *next;
} HuffQueueNode;

typedef struct cbNode
{
    char value2[1000];
    char token2[1000];
    struct cbNode *next;
} cbNode;

cbNode *head;
cbNode *temp;
Huffnode *hhead;
char escapechar[100];
int readCodebook(const char *codebookName)
{

    int fd = open(codebookName, O_RDONLY);

    if (fd < 0)
    {
        printf("Failed to open %s.\n", codebookName);
        return 0;
    }
    int status = 1;
    int readIn = 0;
    char buffer[1000];
    int cbData = 0;

    char cbNodeData[1000];
    int nline = 0;
    int escchar = 1;
    int tabcount = 0;
    int lol = 0;
    int error = 0;

    do
    {
        status = read(fd, buffer + readIn, 1);
        cbNodeData[cbData] = buffer[readIn];
        cbData++;
        if (buffer[readIn] == '\n' && (nline == 0 && escchar == 1))
        {
            cbNodeData[cbData - 1] = '\0';
            cbNode *new = (cbNode *)malloc(sizeof(cbNode));
            strcpy(new->token2, cbNodeData);
            new->next = NULL;
            head = new;
            temp = head;
            nline = 1;
            cbData = 0;
            escchar = 1;
        }
        else if (buffer[readIn] == '\t' && status != 0)
        {

            if (escchar == 0 && tabcount == 1)
            {
                temp->token2[0] = '\t';
                tabcount = 0;
                lol = 1;
            }
            else
            {
                cbNodeData[cbData - 1] = '\0';
                int i;
                for (i = 0; i < cbData - 1; i++)
                {
                    if (cbNodeData[i] != '0' && cbNodeData[i] != '1')
                    {
                        printf("Error: Improper bit sequence %s in given Huffman Codebook\n", cbNodeData);
                        error = 1;
                        break;
                    }
                }
                if (error == 0)
                {
                    cbNode *newcbNode = (cbNode *)malloc(sizeof(cbNode));
                    strcpy(newcbNode->value2, cbNodeData);
                    if (nline == 0 && escchar == 1)
                    {
                        head = newcbNode;
                        temp = head;
                        escchar = 0;
                    }
                    temp->next = newcbNode;
                    temp = newcbNode;
                    tabcount = 1;
                }
            }
            cbData = 0;
        }
        else if (buffer[readIn] == '\n' && status != 0 && error == 1)
        {
            error = 0;
            cbData = 0;
        }
        else if (buffer[readIn] == '\n' && status != 0 && error == 0)
        {
            if (lol == 0)
            {
                cbNodeData[cbData - 1] = '\0';
                strcpy(temp->token2, cbNodeData);
            }
            lol = 0;
            if (escchar == 0 && tabcount == 1 && strcmp(temp->token2, "") == 0)
            {
                temp->token2[0] = '\n';
                lol = 1;
            }
            temp->next = NULL;
            cbData = 0;
            tabcount = 0;
        }
        else if (status == 0)
        {
            break;
        }
    } while (status > 0);

    close(fd);
    return escchar;
}

void writecp(const char *cpRename, int fd2, cbNode *comptemp)
{
    int i = 0;
    int wstatus = 1;
    char wbuffer[1];
    while (comptemp->value2[i] != '\0')
    {
        wbuffer[0] = comptemp->value2[i];
        wstatus = write(fd2, wbuffer, 1);
        i++;
    }
    return;
}

void freecbNode(cbNode *head)
{
    cbNode *temp = head;
    cbNode *next = NULL;
    while (temp != NULL)
    {
        next = temp->next;
        free(temp);
        temp = next;
    }
    return;
}

int compress(const char *compressFile, int escape)
{
    char rename[1000];
    strcpy(rename, compressFile);
    int z = 0;
    while (rename[z] != '\0')
    {
        z++;
    }
    rename[z] = '.';
    rename[z + 1] = 'h';
    rename[z + 2] = 'c';
    rename[z + 3] = 'z';
    rename[z + 4] = '\0';
    const char *cpRename = rename;

    int fd = open(compressFile, O_RDONLY);
    int fd2 = open(cpRename, O_RDWR | O_CREAT | O_TRUNC, 00600);

    if (fd < 0)
    {
        printf("Failed to open file %s.\n", compressFile);
        return 0;
    }
    int status = 1;
    int readIn = 0;
    char buffer[1000];
    int cpData = 0;
    char cpNodeData[1000];
    char cpBufferData[1000];

    int wstatus = 1;
    int writeIn = 0;
    char wbuffer[1];

    do
    {
        status = read(fd, buffer + readIn, 1);
        cpNodeData[cpData] = buffer[readIn];
        cpData++;
        if ((buffer[readIn] == ' ' || buffer[readIn] == '\t' || buffer[readIn] == '\n') || (status == 0 && cpData > 1))
        {
            if (cpData == 1)
            {
                cpNodeData[cpData] = '\0';
            }
            else
            {
                cpNodeData[cpData - 1] = '\0';

                cbNode *comptemp = head;
                while (comptemp != NULL)
                {
                    if (strcmp(cpNodeData, comptemp->token2) == 0)
                    {
                        writecp(cpRename, fd2, comptemp);
                        break;
                    }
                    comptemp = comptemp->next;
                }
                if (comptemp == NULL)
                {
                    printf("Error: %s was not in the Huffman Codebook\n", cpNodeData);
                    close(fd);
                    close(fd2);
                    return 0;
                }
                if (status == 0)
                {
                    return 0;
                }
            }
            if (escape == 1 && status != 0)
            {
                int j = 0;
                while (head->token2[j] != '\0')
                {
                    cpBufferData[j] = head->token2[j];
                    j++;
                }
                if (buffer[readIn] == ' ')
                {
                    cpBufferData[j] = ' ';
                }
                else if (buffer[readIn] == '\t')
                {
                    cpBufferData[j] = 't';
                }
                else if (buffer[readIn] == '\n')
                {
                    cpBufferData[j] = 'n';
                }
                cpBufferData[j + 1] = '\0';
                cbNode *comptemp = head;
                while (comptemp != NULL)
                {
                    if (strcmp(cpBufferData, comptemp->token2) == 0)
                    {
                        writecp(cpRename, fd2, comptemp);
                        break;
                    }
                    comptemp = comptemp->next;
                }

                if (comptemp == NULL)
                {
                    printf("Error: %s was not in the given Huffman Codebook\n", cpBufferData);
                    close(fd);
                    close(fd2);
                    return 0;
                }
            }
            else if (escape == 0 && status != 0)
            {
                cbNode *comptemp = head;
                while (comptemp != NULL)
                {
                    if (buffer[readIn] == comptemp->token2[0])
                    {
                        writecp(cpRename, fd2, comptemp);
                        break;
                    }
                    comptemp = comptemp->next;
                }
                if (comptemp == NULL)
                {
                    printf("Error: %s was not in the given Huffman Codebook\n", cpBufferData);
                    close(fd);
                    close(fd2);
                    return 0;
                }
            }
            cpData = 0;
        }
        else if (status == 0)
        {
            break;
        }
    } while (status > 0);
    close(fd);
    close(fd2);
    return 0;
}

int rebuildTree(const char *codebookName)
{

    int fd = open(codebookName, O_RDONLY);

    if (fd < 0)
    {
        printf("Failed to open %s.\n", codebookName);
        return 0;
    }
    int status = 1;
    int readIn = 0;
    char buffer[1000];
    int status2 = 1;
    int readIn2 = 0;
    char buffer2[1000];

    char cbNodeData[1000];
    int cbData = 0;
    char cbNodeToken[1000];
    int cbToken = 0;
    int nline = 0;
    int escchar = 1;

    int tabcount = 0;
    int lol = 0;
    int error = 0;

    Huffnode *htemp;
    Huffnode *hptr;
    Huffnode *newhhead = (Huffnode *)malloc(sizeof(Huffnode));
    newhhead->left = NULL;
    newhhead->right = NULL;
    hhead = newhhead;

    do
    {
        status = read(fd, buffer + readIn, 1);
        cbNodeData[cbData] = buffer[readIn];
        cbData++;
        if (buffer[readIn] == '\n' && (nline == 0 && escchar == 1))
        {
            cbNodeData[cbData - 1] = '\0';
            strcpy(escapechar, cbNodeData);
            nline = 1;
            cbData = 0;
            lol = 1;
        }

        if (buffer[readIn] == '\t' && status != 0)
        {
            cbNodeData[cbData - 1] = '\0';
            int i;
            for (i = 0; i < cbData - 1; i++)
            {
                if (cbNodeData[i] != '0' && cbNodeData[i] != '1')
                {
                    printf("Error: Improper bit sequence %s in given Huffman Codebook\n", cbNodeData);
                    error = 1;
                    break;
                }
            }
            if (error == 1)
            {
                error = 0;
                do
                {
                    status2 = read(fd, buffer2 + readIn2, 1);
                    if (buffer2[readIn2] == '\n' && status2 != 0)
                    {
                        break;
                    }
                } while (status2 > 0);
                cbData = 0;
            }
            else if (error == 0)
            {
                Huffnode *newnode = (Huffnode *)malloc(sizeof(Huffnode));
                newnode->left = NULL;
                newnode->right = NULL;
                htemp = hhead;
                hptr = htemp;
                int z = 0;
                cbData--;
                while (z < cbData)
                {
                    if (z == 0)
                    {
                        htemp = hhead;
                    }
                    if (cbNodeData[z] == '0')
                    {
                        hptr = hptr->left;
                        if (hptr == NULL && z + 1 == cbData)
                        {
                            htemp->left = newnode;
                            htemp = newnode;
                            do
                            {
                                status2 = read(fd, buffer2 + readIn2, 1);
                                cbNodeToken[cbToken] = buffer2[readIn2];
                                cbToken++;
                                if (buffer2[readIn2] == '\n' && status2 != 0)
                                {
                                    if (cbToken == 1)
                                    {
                                        if (strcmp(htemp->token, "") == 0)
                                        {
                                            cbNodeToken[cbToken] = '\0';
                                        }
                                        status2 = read(fd, buffer2 + readIn2, 1);
                                    }
                                    else
                                    {
                                        cbNodeToken[cbToken - 1] = '\0';
                                    }
                                    strcpy(htemp->token, cbNodeToken);
                                    cbToken = 0;
                                    break;
                                }
                            } while (status2 > 0);
                        }
                        else if (hptr == NULL && z + 1 != cbData)
                        {
                            Huffnode *placenode = (Huffnode *)malloc(sizeof(Huffnode));
                            placenode->left = NULL;
                            placenode->right = NULL;
                            htemp->left = placenode;
                            htemp = htemp->left;
                        }
                        else
                        {
                            htemp = htemp->left;
                        }

                        hptr = htemp;
                    }
                    else if (cbNodeData[z] == '1')
                    {
                        hptr = hptr->right;
                        if (hptr == NULL && z + 1 == cbData)
                        {
                            htemp->right = newnode;
                            htemp = newnode;
                            do
                            {
                                status2 = read(fd, buffer2 + readIn2, 1);
                                cbNodeToken[cbToken] = buffer2[readIn2];
                                cbToken++;
                                if (buffer2[readIn2] == '\n' && status2 != 0)
                                {
                                    if (cbToken == 1)
                                    {
                                        if (strcmp(htemp->token, "") == 0)
                                        {
                                            cbNodeToken[cbToken] = '\0';
                                        }
                                        status2 = read(fd, buffer2 + readIn2, 1);
                                    }
                                    else
                                    {
                                        cbNodeToken[cbToken - 1] = '\0';
                                    }
                                    strcpy(htemp->token, cbNodeToken);
                                    cbToken = 0;
                                    break;
                                }
                            } while (status2 > 0);
                        }
                        else if (hptr == NULL && z + 1 != cbData)
                        {
                            Huffnode *placenode = (Huffnode *)malloc(sizeof(Huffnode));
                            placenode->left = NULL;
                            placenode->right = NULL;
                            htemp->right = placenode;
                            htemp = htemp->right;
                        }
                        else
                        {
                            htemp = htemp->right;
                        }
                        hptr = htemp;
                    }
                    z++;
                }
                cbData = 0;
            }
        }
        else if (status == 0)
        {
            break;
        }

    } while (status > 0);
    close(fd);
    return escchar;
}

void writedcp(const char *dcpRename, int fd2, Huffnode *htemp)
{
    int i = 0;
    int wstatus = 1;
    char wbuffer[1];
    while (htemp->token[i] != '\0')
    {
        wbuffer[0] = htemp->token[i];
        wstatus = write(fd2, wbuffer, 1);
        i++;
    }
    return;
}

void freedcptree(Huffnode *node)
{
    if (node == NULL)
    {
        return;
    }
    freedcptree(node->left);
    freedcptree(node->right);
    free(node);
}

int decompress(const char *decompressFile, Huffnode *hhead, char *escapechar, int escchar)
{
    char rename[1000];
    strcpy(rename, decompressFile);
    int z = 0;
    while (rename[z] != '\0')
    {
        z++;
    }
    rename[z - 4] = '\0';
    const char *dcpRename = rename;
    int fd = open(decompressFile, O_RDONLY);
    int fd2 = open(dcpRename, O_RDWR | O_CREAT | O_TRUNC, 00600);
    if (fd < 0)
    {
        printf("Failed to open file %s.\n", decompressFile);
        return 0;
    }
    int status = 1;
    int readIn = 0;
    char buffer[1000];
    char toknode[1000];
    char tokbuffer[1];

    int wstatus = 1;
    int writeIn = 0;
    char wbuffer[1];
    int found = 0;
    int esclength = strlen(escapechar);
    Huffnode *htemp = hhead;

    do
    {
        if (found == 1)
        {
            htemp = hhead;
        }
        status = read(fd, buffer + readIn, 1);
        if (status == 1)
        {
            if (buffer[readIn] == '\n')
            {
                return 0;
            }

            else if (buffer[readIn] != '0' && buffer[readIn] != '1')
            {
                printf("Error: %s cannot be decompressed with given Huffman Codebook\n", decompressFile);
                close(fd);
                close(fd2);
                return 0;
            }

            else if (buffer[readIn] == '0')
            {
                htemp = htemp->left;
                if (htemp == NULL)
                {
                    printf("Error: %s cannot be decompressed with given Huffman Codebook\n", decompressFile);
                    close(fd);
                    close(fd2);
                    return 0;
                }
                else if (strcmp(htemp->token, "") != 0)
                {
                    if (escchar == 1)
                    {
                        int toklen = strlen(htemp->token);
                        if (toklen == esclength + 1)
                        {
                            strcpy(toknode, htemp->token);
                            tokbuffer[0] = toknode[toklen - 1];
                            toknode[toklen - 1] = '\0';
                            if (strcmp(toknode, escapechar) == 0)
                            {
                                if (tokbuffer[0] == ' ')
                                {
                                    wstatus = write(fd2, " ", 1);
                                }
                                else if (tokbuffer[0] == 't')
                                {
                                    wstatus = write(fd2, "\t", 1);
                                }
                                else if (tokbuffer[0] == 'n')
                                {
                                    wstatus = write(fd2, "\n", 1);
                                }
                                else
                                {
                                    writedcp(dcpRename, fd2, htemp);
                                }
                            }
                            else
                            {
                                writedcp(dcpRename, fd2, htemp);
                            }
                        }
                        else
                        {
                            writedcp(dcpRename, fd2, htemp);
                        }
                    }
                    else
                    {
                        writedcp(dcpRename, fd2, htemp);
                    }
                    found = 1;
                }
                else
                {
                    found = 0;
                }
            }
            else if (buffer[readIn] == '1')
            {
                htemp = htemp->right;
                if (htemp == NULL)
                {
                    printf("Error: %s cannot be decompressed with given Huffman Codebook\n", decompressFile);
                    close(fd);
                    close(fd2);
                    return 0;
                }
                else if (strcmp(htemp->token, "") != 0)
                {
                    if (escchar == 1)
                    {
                        int toklen = strlen(htemp->token);
                        if (toklen == esclength + 1)
                        {
                            strcpy(toknode, htemp->token);
                            tokbuffer[0] = toknode[toklen - 1];
                            toknode[toklen - 1] = '\0';
                            if (strcmp(toknode, escapechar) == 0)
                            {
                                if (tokbuffer[0] == ' ')
                                {
                                    wstatus = write(fd2, " ", 1);
                                }
                                else if (tokbuffer[0] == 't')
                                {
                                    wstatus = write(fd2, "\t", 1);
                                }
                                else if (tokbuffer[0] == 'n')
                                {
                                    wstatus = write(fd2, "\n", 1);
                                }
                                else
                                {
                                    writedcp(dcpRename, fd2, htemp);
                                }
                            }
                            else
                            {
                                writedcp(dcpRename, fd2, htemp);
                            }
                        }
                        else
                        {
                            writedcp(dcpRename, fd2, htemp);
                        }
                    }
                    else
                    {
                        writedcp(dcpRename, fd2, htemp);
                    }
                    found = 1;
                }
                else
                {
                    found = 0;
                }
            }
        }
    } while (status > 0);
    close(fd);
    close(fd2);
    return 0;
}

int searchHeap(hNode *h, char *target)
{ // search for a node with target as its token in the current minHeap
    int i = 0;
    while (h[i].frequency != 0)
    { // find the index of the target token.
        if (strcmp(h[i].token, target) == 0)
        {
            return i;
        }
        i = i + 1;
    }
    return -1; // return -1 if the target token is not in the heap.
}

void swap(hNode *heap, int a, int b)
{ // adjust the minHeap after insertion
    int tmpFre = heap[a].frequency;
    heap[a].frequency = heap[b].frequency;
    heap[b].frequency = tmpFre;
    int tmpisHuff = heap[a].isHuffTree;
    heap[a].isHuffTree = heap[b].isHuffTree;
    heap[b].isHuffTree = tmpisHuff;
    char *tmpToken = (char *)malloc((strlen(heap[a].token) + 1) * sizeof(char));
    strcpy(tmpToken, heap[a].token);
    strcpy(heap[a].token, heap[b].token);
    strcpy(heap[b].token, tmpToken);
    free(tmpToken);
}

int updateHeap(heap *h)
{
    int i = h->count - 1;
    hNode *heap = h->heap;
    while (i > 0)
    {
        if (heap[i].frequency < heap[(i - 1) / 2].frequency)
        {
            swap(heap, i, (i - 1) / 2);
        }
        i = i - 1;
    }
    return 0;
}

int inserthNode(heap *h, hNode *newNode, int isHuffTree)
{ // insert a node into the current minHeap
    if (h->count >= h->capacity)
        return 0;
    hNode *heap = h->heap;
    int count = h->count;
    strcpy(heap[count].token, newNode->token);
    heap[count].frequency = newNode->frequency;
    heap[count].isHuffTree = isHuffTree;
    updateHeap(h);
    count += 1;
    h->count = count;
    return 0;
}

int traverseMinHeap(heap *h)
{
    int count = h->count;
    hNode *heap = h->heap;
    int i;
    for (i = 0; i < count; i++)
    {
        printf("the current node is for token %s with frequency %d.\n", heap[i].token, heap[i].frequency);
    }
    return 0;
}

hNode *removeRoot(heap *h)
{
    if (h->count < 1)
        return NULL;
    int count = h->count;
    hNode *heap = h->heap;
    hNode *returned = (hNode *)malloc(sizeof(hNode));
    returned->frequency = heap[0].frequency;
    returned->isHuffTree = heap[0].isHuffTree;
    strcpy(returned->token, heap[0].token);
    heap[0] = heap[count - 1];
    count -= 1;
    h->count = count;
    int i = 0;
    while (1)
    {
        int newIdx = i;
        if (i * 2 + 1 <= count && heap[i].frequency > heap[i * 2 + 1].frequency)
            newIdx = i * 2 + 1;
        if (i * 2 + 2 <= count && heap[newIdx].frequency > heap[i * 2 + 2].frequency)
            newIdx = i * 2 + 2;
        if (newIdx == i)
            break;
        swap(heap, i, newIdx);
        i = newIdx;
    }
    return returned;
}

Huffnode *createHuffnode(int num, char *token)
{
    Huffnode *node = (Huffnode *)malloc(sizeof(Huffnode));
    if (node == NULL)
    {
        printf("Malloc failed.\n");
    }
    node->value = num;
    strcpy(node->token, token);
    node->left = NULL;
    node->right = NULL;
    return node;
}

Huffnode *searchHuffQueue(HuffQueueNode *hq, char *target)
{

    while (hq != NULL)
    {
        if (strcmp(hq->treeNode->token, target) == 0)
        {
            return hq->treeNode;
        }
        hq = hq->next;
    }
    return NULL;
}

int freeHuffQueue(HuffQueueNode *head)
{
    HuffQueueNode *temp = head;
    HuffQueueNode *pre = NULL;
    while (temp != NULL)
    {
        pre = temp;
        temp = temp->next;
        free(pre);
    }
    return 0;
}

Huffnode *buildHuffmanTree(heap *h)
{
    Huffnode *top = (Huffnode *)malloc(sizeof(Huffnode));
    HuffQueueNode *first = NULL;
    HuffQueueNode *last = NULL;
    hNode *left;
    hNode *right;
    HuffQueueNode *temp;
    if (h->count == 0)
    {
        return NULL;
    }

    if (h->count == 1)
    {
        temp = (HuffQueueNode *)malloc(sizeof(HuffQueueNode));
        left = removeRoot(h);
        right = NULL;
        top = createHuffnode(left->frequency, "tree1");
        top->left = createHuffnode(left->frequency, left->token);
        temp->treeNode = top;
        return top;
    }

    int isFirst = 1;
    char treeName[10000] = "tree";
    while (h->count > 1)
    {
        left = removeRoot(h);
        right = removeRoot(h);
        int len = strlen(treeName);
        treeName[len] = '1';
        treeName[len + 1] = '\0';
        top = createHuffnode(left->frequency + right->frequency, treeName);
        while (searchHeap(h->heap, treeName) != -1)
        {
            len = strlen(treeName);
            treeName[len] = '1';
            treeName[len + 1] = '\0';
        }
        temp = (HuffQueueNode *)malloc(sizeof(HuffQueueNode));
        temp->treeNode = top;
        temp->next = NULL;
        if (isFirst)
        {
            first = temp;
            last = temp;
            isFirst = 0;
        }
        else
        {
            last->next = temp;
            last = temp;
        }

        if (left->isHuffTree == 0)
        {
            top->left = createHuffnode(left->frequency, left->token);
        }
        else
        {
            top->left = searchHuffQueue(first, left->token); //search in queue
        }
        if (right->isHuffTree == 0)
        {
            top->right = createHuffnode(right->frequency, right->token);
        }
        else
        {
            top->right = searchHuffQueue(first, right->token);
        }
        hNode *newNode = (hNode *)malloc(sizeof(hNode));
        if (newNode == NULL)
        {
            printf("Malloc failed.\n");
        }
        newNode->frequency = top->value;
        strcpy(newNode->token, top->token);
        inserthNode(h, newNode, 1);
    }

    Huffnode *returnedNode = (Huffnode *)malloc(sizeof(Huffnode));
    returnedNode->left = last->treeNode->left;
    returnedNode->right = last->treeNode->right;
    strcpy(returnedNode->token, last->treeNode->token);
    returnedNode->value = last->treeNode->value;
    freeHuffQueue(first);
    return returnedNode;
}

int huffmanHeight(Huffnode *node)
{
    if (node == NULL)
    {
        return 0;
    }
    else
    {
        int left = huffmanHeight(node->left);
        int right = huffmanHeight(node->right);
        return (left > right) ? left + 1 : right + 1;
    }
}
int traverseHuffman(Huffnode *top)
{
    if (top->left)
    {
        traverseHuffman(top->left);
    }
    if (top->right)
    {
        traverseHuffman(top->right);
    }
    if (!(top->left || top->right))
    {
        printf("the node %s has a value of %d\n", top->token, top->value);
    }
    return 0;
}

int encodeHuffman(Huffnode *node, char *arr, int idx, int fd, char *escaping)
{

    if (node == NULL)
    {
        return 0;
    }

    if (!(node->left || node->right))
    {
        if (strlen(node->token) == 1 && node->token[0] == '\n')
        {
            int length = strlen(escaping);
            escaping[length] = 'n';
            escaping[length + 1] = '\0';
            strcpy(node->token, escaping);
            escaping[length] = '\0';
        }
        else if (strlen(node->token) == 1 && node->token[0] == '\t')
        {
            int length = strlen(escaping);
            escaping[length] = 't';
            escaping[length + 1] = '\0';
            strcpy(node->token, escaping);
            escaping[length] = '\0';
        }
        else if (strlen(node->token) == 1 && node->token[0] == ' ')
        {
            int length = strlen(escaping);
            escaping[length] = ' ';
            escaping[length + 1] = '\0';
            strcpy(node->token, escaping);
            escaping[length] = '\0';
        }

        int status = 0;
        arr[idx] = '\t';
        arr[idx + 1] = '\0';
        status = write(fd, arr, idx + 1);
        if (status > 0)
        {

            int idx = strlen(node->token);
            node->token[idx] = '\n';
            node->token[idx + 1] = '\0';
            status = write(fd, node->token, strlen(node->token));
            if (status < 0)
            {
                printf("Write failed.\n");
            }
        }
        else
        {
            printf("Write failed.\n");
        }
    }

    if (node->left)
    {
        arr[idx] = '0';
        encodeHuffman(node->left, arr, idx + 1, fd, escaping);
    }
    if (node->right)
    {
        arr[idx] = '1';
        encodeHuffman(node->right, arr, idx + 1, fd, escaping);
    }
    // close fd after calling this method
    return 0;
}

int tokenizeFile(char *fileName, heap *h)
{
    int fd = open(fileName, O_RDONLY);

    if (fd < 0)
    {
        printf("Failed to open file '%s.\n", fileName);
        return 0;
    }
    int status = 1;
    int readIn = 0;
    char buffer[1000];
    hNode *tempheap = h->heap;
    if (tempheap == NULL)
    {
        printf("Malloc failed.\n");
        return 0;
    }

    char hNodeData[1000];
    do
    {
        status = read(fd, buffer + readIn, 1);
        if ((buffer[readIn] == '\n' || buffer[readIn] == ' ' || buffer[readIn] == '\t') && status != 0)
        {
            hNodeData[readIn] = '\0';
            char *wordToken = hNodeData;
            if (readIn != 0)
            {
                if (strcmp(wordToken, "\n") == 0 || strcmp(wordToken, "\t") == 0)
                {
                    strcat(wordToken, "\\");
                }
                int idx = searchHeap(tempheap, wordToken);
                if (idx == -1)
                {
                    hNode *wordNode = (hNode *)malloc(sizeof(hNode));
                    wordNode->frequency = 1;
                    strcpy(wordNode->token, wordToken);
                    inserthNode(h, wordNode, 0);
                }
                else
                {
                    tempheap[idx].frequency += 1;
                    updateHeap(h);
                }
            }
            char temp[2];
            temp[0] = buffer[readIn];
            temp[1] = '\0';
            char delimiterToken[2];
            strcpy(delimiterToken, temp);
            int index = searchHeap(tempheap, delimiterToken);
            if (index == -1)
            {
                hNode *newNode = (hNode *)malloc(sizeof(hNode));
                newNode->frequency = 1;
                strcpy(newNode->token, delimiterToken);
                inserthNode(h, newNode, 0);
            }
            else
            {
                tempheap[index].frequency += 1;
                updateHeap(h);
            }
            readIn = 0;
        }
        else if (status == 0)
        {
            hNodeData[readIn] = '\0';
            char wordToken[1000];
            strcpy(wordToken, hNodeData);
            if (readIn != 0)
            {
                if (strcmp(wordToken, "\n") == 0 || strcmp(wordToken, "\t") == 0)
                {
                    strcat(wordToken, "\\");
                }
                int idx = searchHeap(tempheap, wordToken);
                if (idx == -1)
                {
                    hNode *wordNode = (hNode *)malloc(sizeof(hNode));
                    wordNode->frequency = 1;
                    strcpy(wordNode->token, wordToken);
                    inserthNode(h, wordNode, 0);
                }
                else
                {
                    tempheap[idx].frequency += 1;
                    updateHeap(h);
                }
            }
            readIn = 0;
        }
        else
        {
            hNodeData[readIn] = buffer[readIn];
            readIn += 1;
        }

    } while (status > 0);
    close(fd);
    if (h->heap->frequency == 0)
    {
        return -1;
    }
    return 0;
}

int buildCodebook(heap *h)
{
    hNode *heap = h->heap;
    char escaping[10000] = "#";
    char *singleEscape = "#";
    int esNum = searchHeap(heap, escaping);
    while (esNum != -1)
    {
        strcat(escaping, singleEscape);
        esNum = searchHeap(heap, escaping);
    }

    char *newLine = strcat(escaping, "n");
    int newLineNum = searchHeap(heap, newLine);
    char *tab = newLine;
    tab[strlen(newLine) - 1] = 't';
    int tabNum = searchHeap(heap, tab);
    char *space = tab;
    space[strlen(tab) - 1] = ' ';
    int spaceNum = searchHeap(heap, space);
    while (newLineNum != -1 || tabNum != -1 || spaceNum != -1)
    {
        escaping[strlen(escaping) - 1] = '\0';
        strcat(escaping, singleEscape);
        newLine = strcat(escaping, "n");
        newLineNum = searchHeap(heap, newLine);
        tab = newLine;
        tab[strlen(newLine) - 1] = 't';
        tabNum = searchHeap(heap, tab);
        space = tab;
        space[strlen(tab) - 1] = ' ';
        spaceNum = searchHeap(heap, space);
    }
    escaping[strlen(escaping) - 1] = '\0';
    strcat(escaping, "\n");
    int out = open("./HuffmanCodebook", O_RDWR | O_CREAT, 00600);
    Huffnode *topNode = buildHuffmanTree(h);
    int length = huffmanHeight(topNode) + 1;
    char arr[length];
    int status = 0;
    status = write(out, escaping, strlen(escaping));
    if (status != strlen(escaping))
    {
        printf("Write failed\n");
    }
    escaping[strlen(escaping) - 1] = '\0';
    encodeHuffman(topNode, arr, 0, out, escaping);

    return 0;
}

int recursionAllFiles(char *dirName, char flag, heap *h, int escapeC)
{
    DIR *dir = opendir(dirName);
    struct dirent *ptr;
    if (dir == NULL)
    {
        printf("Fail to open.\n");
    }
    ptr = readdir(dir);
    if (strcmp(ptr->d_name, ".") == 0)
    {
        ptr = readdir(dir);
    }
    while ((ptr = readdir(dir)) != NULL)
    {

        if (dirName[strlen(dirName) - 1] == '/')
        {
            dirName[strlen(dirName) - 1] = '\0';
        }
        char path[1000] = "";
        if (strlen(dirName) == 1 && dirName[0] == '.')
        {
            path[0] = '.';
            path[1] = '/';
            path[2] = '\0';
        }
        else
        {
            strcpy(path, dirName);
            strcat(path, "/");
            strcat(path, ptr->d_name);
        }

        if (ptr->d_type == DT_REG && strcmp(ptr->d_name, ".DS_Store") != 0)
        {
            char isHCZ[5];
            memcpy(isHCZ, ptr->d_name + (strlen(ptr->d_name) - 4), 4);
            isHCZ[4] = '\0';
            if (strcmp(isHCZ, ".hcz") != 0 && strcmp(ptr->d_name, "HuffmanCodebook") != 0)
            {
                if (flag == 'b')
                {
                    tokenizeFile(path, h);
                }
                if (flag == 'c')
                {
                    compress(path, escapeC);
                }
            }
            else if (strcmp(isHCZ, ".hcz") == 0 && strcmp(ptr->d_name, "HuffmanCodebook") != 0)
            {
                if (flag == 'd')
                {
                    decompress(path, hhead, escapechar, escapeC);
                }
            }
        }
        else if (ptr->d_type == DT_DIR)
        {
            recursionAllFiles(path, flag, h, escapeC);
        }
        else
        {
            continue;
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc < 3 || argc > 5)
    {
        printf("Incorrect number of arguments.\n");
        return 0;
    }
    if (strcmp(argv[0], "./fileCompressor") != 0)
    {
        printf("The name for the executable is wrong.\n");
        return 0;
    }

    int i = 1;
    if (argc == 3) // no Rescursion
    {
        if (strcmp(argv[2], "./HuffmanCodebook") == 0 && strcmp(argv[2], "HuffmanCodebook") != 0)
        {
            printf("Should pass in a file but passed in a Huffman codebook instead.\n");
            return 0;
        }
        if (strlen(argv[2]) > 4)
        {
            char isHCZ[5];
            memcpy(isHCZ, argv[2] + (strlen(argv[2]) - 4), 4);
            isHCZ[4] = '\0';
            if (strcmp(isHCZ, ".hcz") == 0)
            {
                printf("Should pass in a regular file but passed in a compressed file instead.\n");
                return 0;
            }
        }
        DIR *tempDir = opendir(argv[2]);
        if (tempDir == NULL)
        {
            int fd = open(argv[2], O_RDONLY);
            if (fd < 0)
            {
                printf("Failed to open %s.\n", argv[2]);
                close(fd);
                return 0;
            }
            else
            {
                close(fd);

                char *flag = argv[1];
                if (strcmp(argv[1], "-b") == 0)
                {
                    heap *h = (heap *)malloc(sizeof(heap));
                    h->heap = (hNode *)malloc(sizeof(hNode) * 10000);
                    h->capacity = 10000;
                    h->count = 0;
                    int result = tokenizeFile(argv[2], h);

                    buildCodebook(h);
                    free(h->heap);
                    free(h);
                }
                else
                {
                    printf("Wrong flag is given.\n");
                    return 0;
                }
            }
        }
        else
        {
            struct dirent *tempPTR = readdir(tempDir);
            if (tempPTR->d_type != DT_REG)
            {
                printf("Should pass in a file but did not.\n");
                closedir(tempDir);
                return 0;
            }
            closedir(tempDir);
        }
    }
    else if (argc == 4)
    {

        if (strcmp(argv[1], "-R") == 0)
        {
            if (strcmp(argv[2], "-b") != 0 && strcmp(argv[2], "-c") != 0 && strcmp(argv[2], "-d") != 0)
            {
                printf("Wrong flag is given.\n");
                return 0;
            }
            else
            {
                DIR *tempDir2 = opendir(argv[3]);
                if (tempDir2 == NULL)
                {
                    int fd2 = open(argv[3], O_RDONLY);
                    if (fd2 < 0)
                    {
                        printf("Failed to open %s.\n", argv[3]);
                        close(fd2);
                        return 0;
                    }
                    else
                    {
                        close(fd2);
                        printf("Should pass in a directory but passed in a file.\n");
                        return 0;
                    }
                }
                struct dirent *tempPTR2 = readdir(tempDir2);
                if (tempPTR2->d_type == DT_DIR)
                {
                    closedir(tempDir2);
                }
                else
                {
                    closedir(tempDir2);
                    printf("Should pass in a directory but did not.\n");
                    return 0;
                }
            }
            if (strcmp(argv[2], "-b") != 0)
            {
                printf("Wrong flag is given.\n");
            }
            heap *h = (heap *)malloc(sizeof(heap));
            h->heap = (hNode *)malloc(sizeof(hNode) * 10000);
            h->capacity = 10000;
            h->count = 0;
            recursionAllFiles(argv[3], 'b', h, 0);
            buildCodebook(h);
            free(h->heap);
            free(h);
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            char hczCheck[5];
            memcpy(hczCheck, argv[2] + (strlen(argv[2]) - 4), 4);
            hczCheck[4] = '\0';
            if (strcmp(hczCheck, ".hcz") == 0)
            {
                printf("Wrong file is given.\n");
                return 0;
            }
            if (strcmp(argv[3], "./HuffmanCodebook") != 0 && strcmp(argv[3], "HuffmanCodebook") != 0)
            {
                printf("Wrong file is not given.\n");
                return 0;
            }
            DIR *tempDir3 = opendir(argv[2]);
            if (tempDir3 == NULL)
            {
                int fd3 = open(argv[2], O_RDONLY);
                if (fd3 < 0)
                {
                    printf("Failed to open %s.\n", argv[2]);
                    close(fd3);
                    return 0;
                }
                else
                {
                    close(fd3);
                }
            }
            else
            {
                struct dirent *tempPTR3 = readdir(tempDir3);
                if (tempPTR3->d_type != DT_REG)
                {
                    printf("Should pass in a file but did not.\n");
                    closedir(tempDir3);
                    return 0;
                }
                closedir(tempDir3);
            }

            int escape = readCodebook(argv[3]);
            compress(argv[2], escape);
            freecbNode(head);
        }
        else if (strcmp(argv[1], "-d") == 0)
        {
            if (strcmp(argv[3], "./HuffmanCodebook") != 0 && strcmp(argv[3], "HuffmanCodebook") != 0)
            {
                printf("Wrong file is given.\n");
                return 0;
            }

            DIR *tempDir4 = opendir(argv[2]);
            if (tempDir4 == NULL)
            {
                int fd4 = open(argv[2], O_RDONLY);
                if (fd4 < 0)
                {
                    printf("Failed to open %s.\n", argv[2]);
                    close(fd4);
                    return 0;
                }
                else
                {
                    close(fd4);
                    char hczCheck[5];
                    memcpy(hczCheck, argv[2] + (strlen(argv[2]) - 4), 4);
                    hczCheck[4] = '\0';
                    if (strcmp(hczCheck, ".hcz") != 0)
                    {
                        printf("Wrong file is given.\n");
                        return 0;
                    }
                }
            }
            else
            {
                struct dirent *tempPTR4 = readdir(tempDir4);
                if (tempPTR4->d_type != DT_REG)
                {
                    printf("Should pass in a file but did not.\n");
                    closedir(tempDir4);
                    return 0;
                }
                closedir(tempDir4);
            }
            int escchar = rebuildTree(argv[3]);
            decompress(argv[2], hhead, escapechar, escchar);
            freedcptree(hhead);
        }
        else
        {
            printf("Wrong flag is given.\n");
        }
    }
    else if (argc == 5)
    {
        if (strcmp(argv[1], "-R") == 0)
        {
            if (strcmp(argv[2], "-c") == 0)
            {
                DIR *tempDir5 = opendir(argv[3]);
                if (tempDir5 == NULL)
                {
                    int fd5 = open(argv[3], O_RDONLY);
                    if (fd5 < 0)
                    {
                        printf("Failed to open %s.\n", argv[3]);
                        close(fd5);
                        return 0;
                    }
                    else
                    {
                        close(fd5);
                        printf("Should pass in a directroy but passed in a file.\n");
                        return 0;
                    }
                }
                else
                {
                    struct dirent *tempPTR5 = readdir(tempDir5);
                    if (tempPTR5->d_type != DT_DIR)
                    {
                        printf("Should pass in a directory but did not.\n");
                        return 0;
                    }
                    else
                    {
                        closedir(tempDir5);
                        if (strcmp(argv[4], "./HuffmanCodebook") != 0 && strcmp(argv[4], "HuffmanCodebook") != 0)
                        {
                            printf("Wrong file is given.\n");
                            return 0;
                        }
                    }
                }
                int escape = readCodebook(argv[4]);
                recursionAllFiles(argv[3], 'c', NULL, escape);
                freecbNode(head);
            }
            else if (strcmp(argv[2], "-d") == 0)
            {

                DIR *tempDir6 = opendir(argv[3]);
                if (tempDir6 == NULL)
                {
                    int fd6 = open(argv[3], O_RDONLY);
                    if (fd6 < 0)
                    {
                        printf("Failed to open %s.\n", argv[3]);
                        close(fd6);
                        return 0;
                    }
                    else
                    {
                        close(fd6);
                        printf("Should pass in a directroy but passed in a file.\n");
                        return 0;
                    }
                }
                else
                {
                    struct dirent *tempPTR6 = readdir(tempDir6);
                    if (tempPTR6->d_type != DT_DIR)
                    {
                        printf("Should pass in a directory but did not.\n");
                        return 0;
                    }
                    else
                    {
                        closedir(tempDir6);
                        if (strcmp(argv[4], "./HuffmanCodebook") != 0 && strcmp(argv[4], "HuffmanCodebook") != 0)
                        {
                            printf("Wrong file is given.\n");
                            return 0;
                        }
                    }
                }
                int escchar = rebuildTree(argv[4]);
                recursionAllFiles(argv[3], 'd', NULL, escchar);
                freedcptree(hhead);
            }
            else
            {
                printf("Wrong flag is given.\n");
                return 0;
            }
        }
        else
        {
            printf("Wrong flag is given.\n");
            return 0;
        }
        return 0;
    }
}
