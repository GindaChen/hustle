#include "Parser.h"

extern const int SERIAL_BLOCK_SIZE = 4096;
char tableList[SERIAL_BLOCK_SIZE];
char project[SERIAL_BLOCK_SIZE];
char loopPred[SERIAL_BLOCK_SIZE];
char otherPred[SERIAL_BLOCK_SIZE];
char aggregate[SERIAL_BLOCK_SIZE];
char groupBy[SERIAL_BLOCK_SIZE];
char orderBy[SERIAL_BLOCK_SIZE];
char *currPos = nullptr;

namespace hustle {
namespace parser {
void Parser::parse(const std::string &sql, hustle::HustleDB &hustleDB) {
  checkExplain(sql);

  // TODO(Lichengxi): enable concurrency, add locks
  hustleDB.getPlan(sql);

  std::string text =
      "{\"tableList\": [" + std::string(tableList) +
          "], \"project\": [" + std::string(project) +
          "], \"loop_pred\": [" + std::string(loopPred) +
          "], \"other_pred\": [" + std::string(otherPred) +
          "], \"aggregate\": [" + std::string(aggregate) +
          "], \"group_by\": [" + std::string(groupBy) +
          "], \"order_by\": [" + std::string(orderBy) +
          "]}";

  json j = json::parse(text);
  parse_tree_ = j;
  preprocessing();
}

std::shared_ptr<ParseTree> Parser::getParseTree() {
  return parse_tree_;
}

void Parser::preprocessing() {
  for (auto &loop_pred : parse_tree_->loop_pred) {
    for (auto it = loop_pred->predicates.begin();
         it != loop_pred->predicates.end();) {
      if ((*it)->plan_type == "SELECT_Pred") {
        parse_tree_->other_pred.push_back(std::move(*it));
        loop_pred->predicates.erase(it);
      } else {
        it += 1;
      }
    }
  }
}

std::string Parser::toString(int indent) {
  json j = parse_tree_;
  return j.dump(indent);
}

void Parser::checkExplain(const std::string &sql) {
  if (!absl::StartsWithIgnoreCase(sql, "EXPLAIN")) {
    std::cerr << "Not starting with EXPLAIN keyword" << std::endl;
    exit(-1);
  }
}

}
}