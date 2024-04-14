#pragma once

#include <string>

#include <object.h>

class Interpreter {
private:
    Scope global_scope_;

public:
    Interpreter()
            : global_scope_({{"quote", Heap::Instance().Make<Quote>()},
                             {"boolean?", Heap::Instance().Make<IsBool>()},
                             {"number?", Heap::Instance().Make<IsNumber>()},
                             {"pair?", Heap::Instance().Make<IsPair>()},
                             {"null?", Heap::Instance().Make<IsNull>()},
                             {"list?", Heap::Instance().Make<IsList>()},
                             {"symbol?", Heap::Instance().Make<IsSymbol>()},
                             {"not", Heap::Instance().Make<Not>()},
                             {"and", Heap::Instance().Make<And>()},
                             {"or", Heap::Instance().Make<Or>()},
                             {"<", Heap::Instance().Make<Less>()},
                             {">", Heap::Instance().Make<Greater>()},
                             {"=", Heap::Instance().Make<Equal>()},
                             {"<=", Heap::Instance().Make<LessEqual>()},
                             {">=", Heap::Instance().Make<GreaterEqual>()},
                             {"+", Heap::Instance().Make<Add>()},
                             {"*", Heap::Instance().Make<Product>()},
                             {"-", Heap::Instance().Make<Sub>()},
                             {"/", Heap::Instance().Make<Divide>()},
                             {"max", Heap::Instance().Make<Max>()},
                             {"min", Heap::Instance().Make<Min>()},
                             {"abs", Heap::Instance().Make<Abs>()},
                             {"cons", Heap::Instance().Make<Cons>()},
                             {"car", Heap::Instance().Make<Car>()},
                             {"cdr", Heap::Instance().Make<Cdr>()},
                             {"list", Heap::Instance().Make<List>()},
                             {"list-ref", Heap::Instance().Make<ListRef>()},
                             {"list-tail", Heap::Instance().Make<ListTail>()},
                             {"define", Heap::Instance().Make<Define>()},
                             {"set!", Heap::Instance().Make<Set>()},
                             {"set-car!", Heap::Instance().Make<SetCar>()},
                             {"set-cdr!", Heap::Instance().Make<SetCdr>()},
                             {"if", Heap::Instance().Make<If>()},
                             {"lambda", Heap::Instance().Make<MakeLambda>()}}) {
    }

    std::string Run(const std::string&);

private:
    void MarkingObjects(Object* current_object, std::unordered_set<Object*>& marks);

    void Sweep(std::unordered_set<Object*>& marks);

    void GarbageCollector();
};
