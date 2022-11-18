#include <string>
#include <variant>

#include "nlohmann/json.hpp"

class InternProcessus;

class Command {
public:
    virtual ~Command();

    virtual void call_execute(InternProcessus &) = 0;
};

class Delete: public Command {
public:
    Delete() = default;
    virtual ~Delete();

    void call_execute(InternProcessus &) override;
    
    std::variant<std::string, int> uid;
};

class List: public Command {
public:
    List() = default;
    virtual ~List();

    void call_execute(InternProcessus &) override;
};

class Append: public Command {
public:
    Append() = default;
    virtual ~Append();

    void call_execute(InternProcessus &) override;

    std::string content;
    int uid;
};

class Load: public Command {
public:
    Load() = default;
    virtual ~Load();

    void call_execute(InternProcessus &) override;

    std::string filename;
    std::string content;
};