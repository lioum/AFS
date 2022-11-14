#include "repl.hh"

#include <cstdlib>
#include <exception>
#include <memory>
#include <mpi.h>
#include <optional>

#include "repl_message.hh"
#include "handshake_message.hh"

namespace repl
{
    
    REPL::REPL(MPI_Comm com, int nb_servers)
    : Server(com, nb_servers)
    , running(true)
    {}

    void REPL::work() {
      std::string input;
      while (running) {
        std::cout << "REPL(" << state.get_rank() << ")$ ";
        std::cin >> input;
        std::shared_ptr<REPL_message> bite = process_message(input);
        input.clear();
        if (bite != nullptr) {
          send(bite->get_target_rank(), bite);
          running = false;
        }
      }
    }

    void REPL::on_message_callback(std::shared_ptr<message::Message> message)
    {
      json j = message->serialize_json();
      std::cout << "REPL(" << state.get_rank() << "): Received message" << std::endl;
      if (j["MESSAGE_TYPE"] == message::MessageType::HANDSHAKE)
      {
        std::cout << message::HandshakeStatus::SUCCESS << std::endl;
        if (j["HANDSHAKE"]["STATUS"] == message::HandshakeStatus::SUCCESS)
        {
          std::cout << "REPL(" << state.get_rank() << "): Handshake successul" << std::endl;
          running = true;
        }
      }
    }

    std::shared_ptr<REPL_message> REPL::process_message(std::string input)
    {
      std::string res;
      int target_rank;
      if (input == "CRASH") // DEAFEN
      {
        std::cout << "REPL: Crashing which one ? ";
        std::cin >> res;
      try {
        target_rank = std::stoi(res);
      } catch (std::invalid_argument &e) {
        std::cout << "REPL: Invalid rank" << std::endl;
        return nullptr;
      }
        return std::make_shared<REPL_message>(ReplType::CRASH, target_rank, state.get_rank());
      } else if (input == "SPEED") {
        std::cout << "REPL: changing speed of which one ? ";
        std::cin >> res;
      try {
        target_rank = std::stoi(res);
      } catch (std::invalid_argument &e) {
        std::cout << "REPL: Invalid rank" << std::endl;
        return nullptr;
      }
        std::string speed_str;
        std::cout << "REPL: What speed (fast, medium, low) ? ";
        std::cin >> speed_str;
        if (speed_str != "low" && speed_str != "medium" && speed_str != "fast") {
          std::cout << "REPL: Invalid speed" << std::endl;
          return nullptr;
        }
        ReplSpeed speed = ReplSpeed::FAST;
        if (speed_str == "medium") {
          speed = ReplSpeed::MEDIUM;
        } else if (speed_str == "low") {
          speed = ReplSpeed::LOW;
        }
        return std::make_shared<REPL_message>(target_rank, state.get_rank(), speed);
      } else if (input == "START") {
        std::cout << "REPL: Starting Which one ? ";
        std::cin >> res;
      try {
        target_rank = std::stoi(res);
      } catch (std::invalid_argument &e) {
        std::cout << "REPL: Invalid rank" << std::endl;
        return nullptr;
      }
        return std::make_shared<REPL_message>(ReplType::START, target_rank, state.get_rank());
      }
        return nullptr;
    }
}

class Visitor
{
  virtual void visit(Message msg);
  virtual void visit(REPL_Message msg);
}

class Receive_Visitor
{
  virtual void visit(Message msg);

  virtual void visit(REPL_Message msg)
  {
    msg->attr2;
  }
}

class Message
{
  int attr1;

  virtual Message deserialize();
  virtual void accept(Visitor v);
}

class REPL_Message : Message
{
  int attr2;

  virtual Message deserialize(string str) static
  {
      msg = Message::deserialize(str);
      msg = dynamic_cast<REPL_Message>(msg);
      msg.attr2 = // deserialize code
      return make_shared<Message>(msg);
  }

  virtual void accept(Visitor v)
  {
    v->visit(*this)
  }
}
int main() {
  string str = mpi_received();
  auto repl_msg = REPL_Message::deserialize(str);
}