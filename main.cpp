#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <cmath>
#include <unistd.h>

using namespace std;

const double PI = acos(-1);

struct Node {
    int index;
    double x, y;
    int correspondingNodeIndex;
    vector<int> neighbors;

    Node();
//    Node(int index, double x, double y);
    Node(int index = 0, double x = 0, double y = 0, int correspondingNodeIndex = -1, int neighbor1 = -1, int neighbor2 = -1);
    void operator=(const Node rhs);
    bool operator==(const Node rhs); //simplified judgement
    double distance(Node rhs);
    int anotherNeighborIndex(int neighbor1);
};

struct Line {
    int index;
    vector<Node> nodes;
    int oppositeLineIndex;

    Line();
    Line(int index): index(index) {}
    void operator=(const Line rhs);
};

struct Quad {
    int index;
    vector<Node> nodes;

    Quad();
    Quad(int index, Node n1, Node n2, Node n3, Node n4);
};

class Group {
public:
    void readNodes();
    void findNeighbors();
    void findCorners();
private:
    vector<Node> inputNodeGroup;
    vector<Node> arrangedInputNodeGroup;
    vector<Node> corners;
};

int main() {
    Group G;
    G.readNodes();
    G.findNeighbors();
    G.findCorners();
}

Node::Node() {
    x = 0.0;
    y = 0.0;
    index = -1;
    correspondingNodeIndex = -1;
    neighbors.clear();
}

//Node::Node(int index, double x, double y) {
//    this->x = x;
//    this->y = y;
//    this->index = index;
//    this->correspondingNodeIndex = -1;
//    this->neighbors.clear();
//}

Node::Node(int index, double x, double y, int correspondingNodeIndex, int neighbor1, int neighbor2) {
    this->x = x;
    this->y = y;
    this->index = index;
    this->correspondingNodeIndex = correspondingNodeIndex;
    this->neighbors.clear();
    this->neighbors.push_back(neighbor1);
    this->neighbors.push_back(neighbor2);
}

void Node::operator=(const Node rhs) {
    this->x = rhs.x;
    this->y = rhs.y;
    this->index = rhs.index;
    this->correspondingNodeIndex = rhs.correspondingNodeIndex;
    this->neighbors.clear();
    for (int i = 0; i < rhs.neighbors.size(); i++) {
        this->neighbors.push_back(rhs.neighbors[i]);
    }
}

bool Node::operator==(const Node rhs) {
    return this->index == rhs.index;
}

double Node::distance(Node rhs) {
    return sqrt((this->x - rhs.x) * (this->x - rhs.x) + (this->y - rhs.y) * (this->y - rhs.y));
}

int Node::anotherNeighborIndex(int neighbor1) {
    return neighbor1 == this->neighbors[0] ? this->neighbors[1] : this->neighbors[0];
}

Line::Line() {
    nodes.clear();
    index = -1;
    oppositeLineIndex = -1;
}

void Line::operator=(const Line rhs) {
    this->nodes.clear();
    for (int i = 0; i < rhs.nodes.size(); i++) {
        this->nodes.push_back(rhs.nodes[i]);
    }
    this->index = rhs.index;
    this->oppositeLineIndex = rhs.oppositeLineIndex;
}

Quad::Quad() {
    nodes.clear();
    index = -1;
}

Quad::Quad(int index, Node n1, Node n2, Node n3, Node n4) {
    this->nodes.clear();
    this->nodes.push_back(n1);
    this->nodes.push_back(n2);
    this->nodes.push_back(n3);
    this->nodes.push_back(n4);
    this->index = index;
}

void Group::readNodes() {
    string fileName;
    cout << "Please input the name of .pl2 file you want to read: ";
    cin >> fileName;

    char* path;
    path = getcwd(NULL, 0);
    string readFilePath = path;
    readFilePath += "/../meshFile";
    readFilePath += fileName;
    readFilePath += ".ple";

    ifstream fin(readFilePath.c_str());

    string line;
    getline(fin, line);
    line.erase(0, line.find_first_not_of(" "));
    line.erase(line.find_last_not_of(" ") + 1);

    int numberOfNodes = 0;
    stringstream ss(line);
    string elementNumberStirng, nodeNumberString, temp;
    ss >> elementNumberStirng >> nodeNumberString >> temp;
    numberOfNodes = stoi(nodeNumberString);

    for (int i = 0; i < numberOfNodes; i++) {
        getline(fin, line);
        stringstream strs(line);
        string nodeIndexString, nodeXString, nodeYString;
        strs >> nodeIndexString >> nodeXString >> nodeYString;
        Node newNode(stoi(nodeIndexString) - 1, stod(nodeXString), stod(nodeYString));
        inputNodeGroup.push_back(newNode);
    }
    fin.close();
}

void Group::findNeighbors() {
    double shortestDistance = 0.0;
    int loopTime = 0;
    int neighborIndex = -1;

    for (int i = 0; i < inputNodeGroup.size(); i++) {
        for (auto eachNode : inputNodeGroup) {
            if (loopTime == 0 && eachNode.index != i) {
                shortestDistance = inputNodeGroup[i].distance(eachNode);
                neighborIndex = eachNode.index;
                loopTime++;
                continue;
            }
            if (eachNode.index != i && inputNodeGroup[i].distance(eachNode) < shortestDistance) {
                shortestDistance = inputNodeGroup[i].distance(eachNode);
                neighborIndex = eachNode.index;
            }
        }
        inputNodeGroup[i].neighbors.push_back(neighborIndex);
        //After the first loop, neighbor1 has been found
        loopTime = 0;
        for (auto eachNode : inputNodeGroup) {
            if (loopTime == 0 && eachNode.index != i && eachNode.index != inputNodeGroup[i].neighbors[0]) {
                shortestDistance = inputNodeGroup[i].distance(eachNode);
                neighborIndex = eachNode.index;
                loopTime++;
                continue;
            }
            if (eachNode.index != i && eachNode.index != inputNodeGroup[i].neighbors[0] && inputNodeGroup[i].distance(eachNode) < shortestDistance) {
                shortestDistance = inputNodeGroup[i].distance(eachNode);
                neighborIndex = eachNode.index;
            }
        }
        //After the second loop, neighbor2 has been found
        inputNodeGroup[i].neighbors.push_back(neighborIndex);
    }
}

void Group::findCorners() {
    double allowableError = 0.5;
    for (auto each : inputNodeGroup) {
        double a = each.distance(inputNodeGroup[each.neighbors[0]]);
        double b = each.distance(inputNodeGroup[each.neighbors[1]]);
        double c = inputNodeGroup[each.neighbors[0]].distance(inputNodeGroup[each.neighbors[1]]);
        double radius = acos((a * a + b * b - c * c) / (2 * a * b));
        if (radius >= PI / 2 - allowableError && radius <= PI / 2 + allowableError) {
            corners.push_back(each);
        }
    }
}