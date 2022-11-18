#include "processus.hh"

void Delete::call_execute(InternProcessus &process)
{
    process.execute(*this);
}

void ClientDelete::call_execute(InternProcessus &process)
{
    process.execute(*this);
}

void Append::call_execute(InternProcessus &process)
{
    process.execute(*this);
}

void List::call_execute(InternProcessus &process)
{
    process.execute(*this);
}

void Load::call_execute(InternProcessus &process)
{
    process.execute(*this);
}

nlohmann::json Load::to_json() const
{
    return *this;
}

nlohmann::json Append::to_json() const
{
    return *this;
}

nlohmann::json List::to_json() const
{
    return *this;
}

nlohmann::json Delete::to_json() const
{
    return *this;
}

nlohmann::json ClientDelete::to_json() const
{
    throw std::runtime_error("ClientDelete::to_json() not implemented");
}