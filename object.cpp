#include "object.h"

// todo: Decompose this

std::vector<Object*> GetVectorFromCell(Object* cell_head, Scope* scope, bool eval) {
    if (!Is<Cell>(cell_head)) {
        throw RuntimeError{"Incorrect args type!"};
    }
    std::vector<Object*> cell_vector = {};
    Object* current_head;
    while (cell_head && Is<Cell>(cell_head)) {
        if (eval) {
            if (!As<Cell>(cell_head)->GetFirst()) {
                throw RuntimeError{"Incorrect args type"};
            }
            current_head = As<Cell>(cell_head)->GetFirst()->Eval(scope);
        } else {
            current_head = As<Cell>(cell_head)->GetFirst();
        }
        cell_vector.push_back(current_head);
        cell_head = As<Cell>(cell_head)->GetSecond();
    }
    if (cell_head && !Is<Cell>(cell_head)) {  // corner case
        cell_vector.push_back(cell_head);
    }
    return cell_vector;
}

Object* Symbol::Eval(Scope* scope) {
    if (!scope->Find(name_)) {
        throw NameError{"Undefined command " + name_};
    }
    return scope->Get(name_);
}

Object* Cell::Eval(Scope* scope) {
    if (!head_) {
        throw RuntimeError{"Error in eval of cell head"};
    }
    // Process incorrect pairs
    if (Is<Symbol>(head_) && As<Symbol>(head_)->GetName() == ".") {
        throw SyntaxError{"This syntax didn't support"};
    }
    // Get evaluate of head
    auto eval_head = head_->Eval(scope);
    if (!eval_head) {
        throw RuntimeError{"Error in cell eval"};
    }
    if (Is<Lambda>(eval_head)) {
        Lambda* lambda_func = As<Lambda>(eval_head);
        return lambda_func->Apply(tail_, scope);
    }
    if (Is<Function>(eval_head)) {
        // Convert it to function
        Function* func = As<Function>(eval_head);
        // and apply it to tail
        return func->Apply(tail_, scope);
    }
    throw RuntimeError{"didn't support (...) without function"};
}

std::string Cell::Serialize() {
    // todo: Переделать это
    if (!head_) {
        return "(())";
    }
    Object* current_head = this;
    // corner case with nested lists
    if (!As<Cell>(current_head)->GetSecond() && Is<Cell>(As<Cell>(current_head)->GetFirst())) {
        current_head = As<Cell>(current_head)->GetFirst();
    }
    std::string result = "(";
    while (current_head) {
        auto next_head = As<Cell>(current_head)->GetFirst();
        if (!next_head) {
            throw RuntimeError{"Error in Serialize"};
        }
        result += next_head->Serialize();
        auto next_tail = As<Cell>(current_head)->GetSecond();
        if (next_tail && !Is<Cell>(next_tail)) {  // is pair or proper
            result += " . ";
            result += next_tail->Serialize();
            break;
        }
        current_head = next_tail;
        if (!current_head) {
            break;
        }
        result += " ";
    }
    result += ")";
    return result;
}

Object* Quote::Apply(Object* head, Scope*) {
    if (Is<Cell>(head) && !As<Cell>(head)->GetSecond()) {
        if (!As<Cell>(head)->GetFirst() || Is<Number>(As<Cell>(head)->GetFirst())) {
            return head;
        }
        return As<Cell>(head)->GetFirst();
    }
    return head;
}

Object* CheckType::Apply(Object* head, Scope* scope) {
    if (!head) {
        throw RuntimeError{"Incorrect args"};
    }
    return Heap::Instance().Make<Bool>(IsTypeOf(head, scope));
}

bool IsBool::IsTypeOf(Object* target_object, Scope* scope) {
    if (!Is<Cell>(target_object)) {
        return Is<Bool>(target_object);
    }
    if (As<Cell>(target_object)->GetSecond()) {
        throw RuntimeError{"Incorrect args"};
    }
    if (!As<Cell>(target_object)->GetFirst()) {
        return false;
    }
    return Is<Bool>(As<Cell>(target_object)->GetFirst()->Eval(scope));
}

bool IsNumber::IsTypeOf(Object* target_object, Scope* scope) {
    if (!Is<Cell>(target_object)) {
        return Is<Number>(target_object);
    }
    if (As<Cell>(target_object)->GetSecond()) {
        throw RuntimeError{"Incorrect args"};
    }
    if (!As<Cell>(target_object)->GetFirst()) {
        return false;
    }
    return Is<Number>(As<Cell>(target_object)->GetFirst()->Eval(scope));
}

bool IsSymbol::IsTypeOf(Object* target_object, Scope* scope) {
    if (!Is<Cell>(target_object)) {
        return Is<Symbol>(target_object);
    }
    if (As<Cell>(target_object)->GetSecond()) {
        throw RuntimeError{"Incorrect args"};
    }
    if (!As<Cell>(target_object)->GetFirst()) {
        return false;
    }
    return Is<Symbol>(As<Cell>(target_object)->GetFirst()->Eval(scope));
}

bool IsPair::IsTypeOf(Object* target_object, Scope* scope) {
    if (!Is<Cell>(target_object)) {
        return false;
    }
    auto current_head = As<Cell>(As<Cell>(target_object)->GetFirst()->Eval(scope));
    if (!current_head) {
        return false;
    }
    std::vector<Object*> args = GetVectorFromCell(current_head, scope);
    return args.size() == 2;
}

bool IsNull::IsTypeOf(Object* target_object, Scope* scope) {
    auto current_head = As<Cell>(target_object)->GetFirst()->Eval(scope);
    return !current_head;
}

bool IsList::IsTypeOf(Object* target_object, Scope* scope) {
    if (!Is<Cell>(target_object)) {  // corner
        return false;
    }
    auto current_head = As<Cell>(target_object)->GetFirst()->Eval(scope);
    if (!current_head) {  // empty
        return true;
    }
    // iterate list
    auto it_cell = As<Cell>(current_head);
    while (Is<Cell>(it_cell->GetSecond())) {
        it_cell = As<Cell>(it_cell->GetSecond());
    }
    if (it_cell->GetSecond()) {  // in end of list must be nullptr ( () )
        return false;
    }
    return true;
}

Object* Not::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head) || As<Cell>(head)->GetSecond()) {
        throw RuntimeError{"Incorrect args"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope);
    if (args.size() > 1) {
        throw RuntimeError{"Incorrect args"};
    }
    if (!Is<Bool>(args[0])) {
        return Heap::Instance().Make<Bool>(false);
    }
    return Heap::Instance().Make<Bool>(!As<Bool>(args[0])->GetState());
}

Object* Abs::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw RuntimeError{"Incorrect args"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope);
    if (args.size() > 1) {
        throw RuntimeError{"Incorrect args"};
    }
    if (!Is<Number>(args[0])) {
        throw RuntimeError{"Incorrect args"};
    }
    return Heap::Instance().Make<Number>(std::abs(As<Number>(args[0])->GetValue()));
}

// Pair operations
// Make Pair
Object* Cons::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw RuntimeError{"Incorrect args"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope, false);
    if (args.size() != 2) {
        throw RuntimeError{"cons requires 2 arguments"};
    }
    return Heap::Instance().Make<Cell>(args[0]->Eval(scope), args[1]->Eval(scope));
}

Object* Car::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw RuntimeError{"Incorrect args"};
    }
    head = As<Cell>(head)->GetFirst()->Eval(scope);
    if (!head) {
        throw RuntimeError{"car requires non-zero arguments"};
    }
    if (!Is<Cell>(head)) {
        return head;
    }
    // unpacking
    if (!As<Cell>(head)) {
        return As<Cell>(head->Eval(scope))->GetFirst();
    }
    return As<Cell>(head)->GetFirst();
}

Object* Cdr::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw RuntimeError{"Incorrect args"};
    }
    head = As<Cell>(head)->GetFirst()->Eval(scope);
    if (!head) {
        throw RuntimeError{"cdr requires non-zero arguments"};
    }
    if (!Is<Cell>(head)) {
        return nullptr;
    }
    // unpacking
    if (!As<Cell>(head)) {
        return As<Cell>(head->Eval(scope))->GetSecond();
    }
    return As<Cell>(head)->GetSecond();
}

// List operations

Object* List::Apply(Object* head, Scope* scope) {
    if (!head) {
        return nullptr;
    }
    if (!Is<Cell>(head)) {
        throw RuntimeError{"Incorrect args"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope);
    // packing
    auto list = Heap::Instance().Make<Cell>(args.back(), nullptr);
    for (ssize_t i = args.size() - 2; i >= 0; --i) {
        list = Heap::Instance().Make<Cell>(args[i], list);
    }
    return list;
}

Object* ListRef::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw RuntimeError{"list-ref expected 2 args"};
    }
    auto first_arg = As<Cell>(head)->GetFirst()->Eval(scope);
    std::vector<Object*> target_list = GetVectorFromCell(first_arg, scope);
    auto second_arg = As<Cell>(As<Cell>(head)->GetSecond())->GetFirst();
    if (!Is<Number>(second_arg)) {
        throw RuntimeError{"list-ref expected Number type operand as second arg"};
    }
    if (As<Number>(second_arg)->GetValue() < 0) {
        throw RuntimeError{""};
    }
    size_t target_index = static_cast<size_t>(As<Number>(second_arg)->GetValue());
    if (target_index >= target_list.size()) {
        throw RuntimeError{"in list-ref index out of range"};
    }
    return target_list[target_index];
}

Object* ListTail::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw RuntimeError{"list-ref expected 2 args"};
    }
    auto first_arg = As<Cell>(head)->GetFirst()->Eval(scope);
    std::vector<Object*> target_list = GetVectorFromCell(first_arg, scope);
    auto second_arg = As<Cell>(As<Cell>(head)->GetSecond())->GetFirst();
    if (!Is<Number>(second_arg)) {
        throw RuntimeError{"list-ref expected Number type operand as second arg"};
    }
    if (As<Number>(second_arg)->GetValue() < 0) {
        throw RuntimeError{""};
    }
    size_t target_index = static_cast<size_t>(As<Number>(second_arg)->GetValue());
    if (target_index > target_list.size()) {
        throw RuntimeError{"in list-ref index out of range"};
    }
    if (target_index == target_list.size()) {
        return nullptr;
    }
    auto new_sublist = Heap::Instance().Make<Cell>(target_list.back(), nullptr);
    for (ssize_t i = target_list.size() - 2; i >= static_cast<ssize_t>(target_index); --i) {
        new_sublist = Heap::Instance().Make<Cell>(target_list[i], new_sublist);
    }
    return new_sublist;
}

bool Scope::Find(const std::string target_name) const noexcept {
    auto namespace_it = namespace_.find(target_name);
    if (namespace_it != namespace_.end()) {
        return true;
    }
    if (parent_scope_ != nullptr) {
        return parent_scope_->Find(target_name);
    }
    return false;
}

Object*& Scope::Get(const std::string target_name) {
    auto namespace_it = namespace_.find(target_name);
    if (namespace_it != namespace_.end()) {
        return namespace_it->second;
    }
    if (parent_scope_ != nullptr) {
        return parent_scope_->Get(target_name);
    }
    namespace_[target_name] = Heap::Instance().Make<Object>();
    return namespace_[target_name];
}

Object* If::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw SyntaxError{"if expected 1 or 2 arguments and maybe return value as 3 argument"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope, false);
    if (args.size() == 1) {
        return Heap::Instance().Make<Cell>(nullptr, nullptr);
    }
    if (args.size() > 3) {
        throw SyntaxError{"if expected 1 or 2 arguments and maybe return value as 3 argument"};
    }
    auto eval_cond = args[0]->Eval(scope);
    bool condition_flag = !Is<Bool>(eval_cond) || As<Bool>(eval_cond)->GetState();
    if (condition_flag) {
        if (args.size() > 1) {
            return args[1]->Eval(scope);  // true branch
        }
    } else {
        if (args.size() > 2) {
            return args[2]->Eval(scope);  // false branch
        }
        return nullptr;
    }
    return Heap::Instance().Make<Cell>(nullptr, nullptr);
}

Object* Define::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head) || !As<Cell>(head)->GetSecond()) {
        throw SyntaxError{"Invalid args Define 1"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope, false);
    if (args.size() < 2) {
        throw SyntaxError{"Invalid args Define 2"};
    }
    if (!Is<Cell>(args[0])) {  // if it variable
        if (args.size() != 2) {
            throw SyntaxError{"Invalid args in Define"};
        }
        if (!Is<Symbol>(args[0])) {
            throw SyntaxError{"Invalid args Define 3"};
        }
        std::string variable_name = As<Symbol>(args[0])->GetName();
        if (!args[1]) {
            throw SyntaxError{"Invalid args Define 4"};
        }
        Object* variable_value = args[1]->Eval(scope);
        scope->Define(variable_name, variable_value);
    } else {  // if it lambda
        auto lambda_header = GetVectorFromCell(args[0], scope, false);
        if (!Is<Symbol>(lambda_header[0])) {
            throw SyntaxError{"Invalid args Define 5"};
        }
        std::string lambda_name = As<Symbol>(lambda_header[0])->GetName();

        // Read lambda args
        std::vector<std::string> lambda_args;
        if (lambda_header.size() > 1) {
            lambda_args.reserve(lambda_header.size() - 1);
            for (size_t i = 1; i < lambda_header.size(); ++i) {
                if (!Is<Symbol>(lambda_header[i])) {
                    throw SyntaxError{"Invalid args Define 6"};
                }
                lambda_args.emplace_back(As<Symbol>(lambda_header[i])->GetName());
            }
        }
        // Read lambda body
        std::vector<Object*> lambda_body;
        lambda_body.reserve(args.size() - 1);
        for (size_t i = 1; i < args.size(); ++i) {
            lambda_body.push_back(args[i]);
        }
        // Create lambda
        scope->Define(lambda_name, Heap::Instance().Make<Lambda>(lambda_args, lambda_body, scope));
    }
    return nullptr;
}

Object* Set::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head) || !As<Cell>(head)->GetSecond()) {
        throw SyntaxError{"Invalid args Set 1"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope, false);
    if (args.size() != 2) {
        throw SyntaxError{"Invalid args Set 2"};
    }
    if (!Is<Cell>(args[0])) {  // if it variable
        if (!Is<Symbol>(args[0])) {
            throw SyntaxError{"Invalid args Set 3"};
        }
        std::string variable_name = As<Symbol>(args[0])->GetName();
        if (!scope->Find(variable_name)) {
            throw NameError{"Invalid args Set 4"};
        }
        if (!args[1]) {
            throw SyntaxError{"Invalid args Set 5"};
        }
        scope->Get(variable_name) = args[1]->Eval(scope);
    } else {  // if it lambda
        throw SyntaxError{"Invalid args Set 6"};
    }
    return nullptr;
}

Object* SetCar::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head) || !As<Cell>(head)->GetSecond()) {
        throw SyntaxError{"Invalid args"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope, false);
    if (args.size() != 2) {
        throw SyntaxError{"Invalid args"};
    }
    auto to_assign = args[0]->Eval(scope);
    if (Is<Symbol>(to_assign)) {
        std::string variable_name = As<Symbol>(to_assign)->GetName();
        if (!scope->Find(variable_name)) {
            throw NameError{"Invalid args"};
        }
        to_assign = scope->Get(variable_name);
    }
    if (!args[1]) {
        throw SyntaxError{"Invalid args"};
    }
    auto new_head = args[1]->Eval(scope);
    if (!Is<Cell>(to_assign)) {
        throw SyntaxError{"Invalid args"};
    }
    As<Cell>(to_assign)->SetHead(new_head);
    return nullptr;
}

Object* SetCdr::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head) || !As<Cell>(head)->GetSecond()) {
        throw SyntaxError{"Invalid args"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope, false);
    if (args.size() != 2) {
        throw SyntaxError{"Invalid args"};
    }
    auto to_assign = args[0]->Eval(scope);
    if (Is<Symbol>(to_assign)) {
        std::string variable_name = As<Symbol>(to_assign)->GetName();
        if (!scope->Find(variable_name)) {
            throw NameError{"Invalid args"};
        }
        to_assign = scope->Get(variable_name);
    }
    if (!args[1]) {
        throw SyntaxError{"Invalid args"};
    }
    auto new_tail = args[1]->Eval(scope);
    if (!Is<Cell>(to_assign)) {
        throw SyntaxError{"Invalid args"};
    }
    As<Cell>(to_assign)->SetTail(new_tail);
    return nullptr;
}

Object* MakeLambda::Apply(Object* head, Scope* scope) {
    if (!head || !Is<Cell>(head)) {
        throw SyntaxError{"Invalid args in make Lambda 1"};
    }
    std::vector<Object*> args = GetVectorFromCell(head, scope, false);
    if (args.size() < 2) {
        throw SyntaxError{"Invalid args in make Lambda 2"};
    }
    // Gen lambda name
    //    std::string lambda_name = Lambda::CreateRandomName(15);
    // Read lambda args
    std::vector<std::string> lambda_args;
    if (args[0]) {
        std::vector<Object*> lambda_args_not_eval = GetVectorFromCell(args[0], scope, false);
        lambda_args.reserve(lambda_args_not_eval.size());
        for (size_t i = 0; i < lambda_args_not_eval.size(); ++i) {
            if (!Is<Symbol>(lambda_args_not_eval[i])) {
                throw SyntaxError{"Invalid args in make Lambda 3"};
            }
            lambda_args.push_back(As<Symbol>(lambda_args_not_eval[i])->GetName());
        }
    }
    // Read lambda body
    std::vector<Object*> lambda_body;
    lambda_body.reserve(args.size() - 1);
    for (size_t i = 1; i < args.size(); ++i) {
        lambda_body.push_back(args[i]);
    }
    // Create lambda
    return Heap::Instance().Make<Lambda>(lambda_args, lambda_body, scope);
}

Lambda::Lambda(std::vector<std::string> args, std::vector<Object*> body, Scope* parent_scope)
        : args_(std::move(args)), body_(std::move(body)) {
    scope_ = Heap::Instance().Make<Scope>(parent_scope);
    //    lambda_scopes_catalog.emplace_back(scope_);
}

Object* Lambda::Apply(Object* head, Scope* scope) {
    Scope* local_scope = Heap::Instance().Make<Scope>(scope_);
    //    lambda_scopes_catalog.emplace_back(local_scope);
    if (head && Is<Cell>(head)) {
        std::vector<Object*> args = GetVectorFromCell(head, scope, false);
        if (args.size() != args_.size()) {
            throw RuntimeError{"Invalid args in lambda apply"};
        }
        // Init current local scope
        for (size_t i = 0; i < args.size(); ++i) {
            local_scope->Define(args_[i], args[i]->Eval(scope));
        }
    } else {
        if (!args_.empty()) {
            throw RuntimeError{"Invalid args in lambda apply"};
        }
    }
    Object* result = nullptr;
    for (auto ptr_to_command : body_) {
        result = ptr_to_command->Eval(local_scope);
    }
    return result;
}

void Heap::Destroy(Object* ptr) {
    auto to_destroy_it = objects_tree_.find(ptr);
    if (to_destroy_it != objects_tree_.end()) {
        delete ptr;
        objects_tree_.erase(to_destroy_it);
    }
}

void Heap::Sweep(std::unordered_set<Object*>& marks) {
    for (auto ptr : objects_tree_) {
        if (marks.find(ptr) == marks.end()) {
            Destroy(ptr);
        }
    }
}

void Heap::MarkingObjects(Object* current_object, std::unordered_set<Object*>& marks) {
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

void Heap::GarbageCollector(Object* target_scope) {
    // Mark and Sweep
    std::unordered_set<Object*> marks;
    MarkingObjects(target_scope, marks);
    Sweep(marks);
}
