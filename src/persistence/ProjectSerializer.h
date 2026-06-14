#pragma once
#include <QString>
#include <memory>
#include "../graph/CircuitGraph.h"

/**
 * Serialises/deserialises CircuitGraph to/from a JSON file.
 *
 * JSON schema:
 * {
 *   "version": 1,
 *   "components": [ { ...component fields... }, ... ],
 *   "wires": [
 *     { "id":N, "startCompId":N, "startPinName":"", "endCompId":N, "endPinName":"", "path":[...] }
 *   ],
 *   "junctions": [ { "id":N, "x":N, "y":N }, ... ]
 * }
 */
class ProjectSerializer {
public:
    /** Returns empty string on success, error message on failure */
    static QString save(const CircuitGraph& graph, const QString& filePath);
    static QString load(CircuitGraph& graph, const QString& filePath);

private:
    static std::shared_ptr<Component> createComponent(const QString& type);
};