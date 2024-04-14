#include "scheme.h"

#include <sstream>

#include "tokenizer.h"
#include "parser.h"

void Interpreter::Sweep(std::unordered_set<Object*>& marks) {
    std::unordered_set<Object*> objects = Heap::Instance().GetObjects();
    for (auto ptr : objects) {
        if (marks.find(ptr) == marks.end()) {
            Heap::Instance().Destroy(ptr);
        }
    }
}

void Interpreter::MarkingObjects(Object* current_object, std::unordered_set<Object*>& marks) {
    if (!current_object) {
        return;
    }
    std::vector<Object*> not_visited_children;
    marks.insert(current_object);
    if (Is<Cell>(current_object)) {
        auto cell_ptr = As<Cell>(current_object);
        not_visited_children.push_back(cell_ptr->GetFirst());
        not_visited_children.push_back(cell_ptr->GetSecond());
    } else if (Is<Lambda>(current_object)) {
        auto lambda_ptr = As<Lambda>(current_object);
        for (Object* to : lambda_ptr->GetBody()) {
            not_visited_children.push_back(to);
        }
        not_visited_children.push_back(lambda_ptr->GetScope());
    } else if (Is<Scope>(current_object)) {
        auto scope_ptr = As<Scope>(current_object);
        for (auto& [name, ptr] : scope_ptr->GetNamespace()) {
            not_visited_children.push_back(ptr);
        }
        not_visited_children.push_back(scope_ptr->GetParentScope());
    }
    for (auto to : not_visited_children) {
        if (to && marks.find(to) == marks.end()) {
            MarkingObjects(to, marks);
        }
    }
}

void Interpreter::GarbageCollector() {
    // Mark and Sweep
    std::unordered_set<Object*> marks;
    MarkingObjects(&global_scope_, marks);
    Sweep(marks);
}

std::string Interpreter::Run(const std::string& code) {
    std::stringstream input_stream{code};
    Tokenizer tokenizer{&input_stream};
    Object* parser_result = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Tokenizer error in parser process");
    }
    if (!parser_result) {
        throw RuntimeError("Parser work error");
    }
    Object* eval_result = parser_result->Eval(&global_scope_);
    std::string result;
    if (!eval_result) {
        result = "()";
    } else {
        result = eval_result->Serialize();
    }
    GarbageCollector();
    return result;
}
