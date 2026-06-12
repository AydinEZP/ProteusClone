#pragma once
#include <vector>
#include <memory>

class Pin;
class Wire;
class Junction;

/**
 * Represents one electrical net node: a set of pins all connected
 * together through wires and junctions.
 */
struct NetNode {
    int id {-1};
    std::vector<std::shared_ptr<Pin>>      pins;
    std::vector<std::shared_ptr<Wire>>     wires;
    std::vector<std::shared_ptr<Junction>> junctions;
};