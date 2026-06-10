#pragma once
#include <QString>

/**
 * Abstract base for all undo-able commands (GoF Command pattern).
 */
class BaseCommand {
public:
    virtual ~BaseCommand() = default;
    virtual void execute() = 0;
    virtual void undo()    = 0;
    virtual QString description() const = 0;
};