#include <string>
#include <variant>

#include "nlohmann/json.hpp"

class RaftServer;

/*
** enum of possible command accepted
*/
enum class CommandType
{
    DELETE,
    APPEND,
    LIST,
    LOAD,
    CLIENT_DELETE
};

/*
** Association between the previous enum and the json file data
*/
NLOHMANN_JSON_SERIALIZE_ENUM(CommandType,
                             { { CommandType::DELETE, "DELETE" },
                               { CommandType::APPEND, "APPEND" },
                               { CommandType::LIST, "LIST" },
                               { CommandType::LOAD, "LOAD" },
                               { CommandType::CLIENT_DELETE,
                                 "CLIENT_DELETE" } })

/*
** Class Command
**
** Specification about a command sent and received between servers and clients
*/
class Command
{
public:
    /*
    ** Constructor
    **
    ** @CommandType type : Specify the type of command
    */
    Command(CommandType type, int client_uid, int command_id)
        : type(type)
        , client_uid(client_uid)
        , command_id(command_id)
    {}

    /*
    ** call_execute Function
    **
    ** Call the right execute function from the InternProcessus Class
    */
    virtual void call_execute(RaftServer &) = 0;
    virtual nlohmann::json to_json() const = 0;

    CommandType type;
    int client_uid;
    int command_id;
};

/*
**  Class Delete
**
**  A specific command of type DELETE
*/
class Delete : public Command
{
public:
    Delete()
        : Command(CommandType::DELETE, -1, -1){};
    Delete(int client_uid, int command_id, const std::string &uid)
        : Command(CommandType::DELETE, client_uid, command_id)
        , uid(uid){};

    virtual void call_execute(RaftServer &) override;
    virtual nlohmann::json to_json() const override;

    std::string uid;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Delete, type, client_uid, command_id, type,
                                   uid)


/*
**  Class List
**
**  A specific command of type LIST
*/
class List : public Command
{
public:
    List()
        : Command(CommandType::LIST, -1, -1){};
    List(int client_uid, int command_id)
        : Command(CommandType::LIST, client_uid, command_id){};

    virtual void call_execute(RaftServer &) override;
    virtual nlohmann::json to_json() const override;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(List, type, client_uid, command_id)

/*
**  Class Append
**
**  A specific command of type APPEND
*/
class Append : public Command
{
public:
    Append()
        : Command(CommandType::APPEND, -1, -1){};
    Append(int client_uid, int command_id, const std::string &uid, const std::string &content)
        : Command(CommandType::APPEND, client_uid, command_id)
        , uid(uid)
        , content(content){};

    virtual void call_execute(RaftServer &) override;
    virtual nlohmann::json to_json() const override;

    std::string uid;
    std::string content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Append, type, client_uid, command_id,
                                   content, uid)

/*
**  Class Load
**
**  A specific command of type LOAD
*/
class Load : public Command
{
public:
    Load()
        : Command(CommandType::LOAD, -1, -1){};
    Load(int client_uid, int command_id, const std::string &filename,
         const std::string &content)
        : Command(CommandType::LOAD, client_uid, command_id)
        , filename(filename)
        , content(content){};

    virtual void call_execute(RaftServer &) override;
    virtual nlohmann::json to_json() const override;

    std::string filename;
    std::string content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Load, type, client_uid, command_id, filename,
                                   content)

namespace nlohmann
{
    template <>
    struct adl_serializer<std::shared_ptr<Command>>
    {
        /*
        ** to_json Function
        **
        ** Transform a Command to json
        */
        static void to_json(json &j, const std::shared_ptr<Command> &opt)
        {
            if (opt)
                j = opt->to_json();
            else
                j = nullptr;
        }

        /*
        ** from_json Function
        **
        ** Instance a command directly from a input json
        */
        static void from_json(const json &j, std::shared_ptr<Command> &opt)
        {
            if (j.is_null())
                opt = nullptr;
            else
            {
                switch (j.at("type").get<CommandType>())
                {
                case CommandType::DELETE:
                    opt = std::make_unique<Delete>(j.get<Delete>());
                    break;
                case CommandType::APPEND:
                    opt = std::make_unique<Append>(j.get<Append>());
                    break;
                case CommandType::LIST:
                    opt = std::make_unique<List>(j.get<List>());
                    break;
                case CommandType::LOAD:
                    opt = std::make_unique<Load>(j.get<Load>());
                    break;
                case CommandType::CLIENT_DELETE:
                    throw std::runtime_error(
                        "CLIENT_DELETE is not supposed to be serialized");
                    break;
                }
            }
        }
    };
} // namespace nlohmann
