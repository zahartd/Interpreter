#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>

#include "error.h"
#include "constans.h"

// fwd decl
class Object;
class Number;
class Symbol;
class Cell;
class Lambda;
class Scope;

// Base class of all objects and states in Scheme
class Object {
public:
    virtual ~Object() = default;

    virtual Object* Eval(Scope*) {
        throw RuntimeError{"No eval"};
    }

    virtual std::string Serialize() {
        throw RuntimeError{"No Serialize"};
    }

    virtual Object* Clone() {
        throw RuntimeError{"No Serialize"};
    }
};

// Helper functions

template <class T>
T* As(Object* obj) {
    return dynamic_cast<T*>(obj);
}

template <class T>
bool Is(Object* obj) {
    return As<T>(obj) != nullptr;
}

class Heap {
private:
    std::unordered_set<Object*> objects_tree_;

public:
    static Heap& Instance() {
        static Heap heap;
        return heap;
    }

    template <class T, class... Args>
    T* Make(Args&&... args) {
        T* object = new T(std::forward<Args>(args)...);
        objects_tree_.insert(object);
        return object;
    }

    void GarbageCollector(Object* target_scope);

    void Destroy(Object* ptr);

    std::unordered_set<Object*>& GetObjects() {
        return objects_tree_;
    }

    void Clear() {
        objects_tree_.clear();
    }

private:
    void MarkingObjects(Object* current_object, std::unordered_set<Object*>& marks);

    void Sweep(std::unordered_set<Object*>& marks);

    Heap() = default;

    ~Heap() {
        for (Object* object_ptr : objects_tree_) {
            delete object_ptr;
        }
        Clear();
    }
};

class Scope : public Object {
public:
    using Namespace = std::unordered_map<std::string, Object*>;

private:
    Scope* parent_scope_ = nullptr;
    Namespace namespace_;

public:
    explicit Scope(Namespace&& a_namespace) : namespace_(a_namespace) {
    }

    explicit Scope(Scope* init_parent_scope = nullptr) : parent_scope_(init_parent_scope){};

    void Define(const std::string& target_name, Object* value) {
        namespace_[target_name] = value;
    }

    bool Find(const std::string target_name) const noexcept;

    Object*& Get(const std::string target_name);

    [[maybe_unused]] Scope* GetParentScope() {
        return parent_scope_;
    }

    Namespace& GetNamespace() {
        return namespace_;
    }
};

std::vector<Object*> GetVectorFromCell(Object* cell_head, Scope* scope, bool eval = true);

class Function : public Object {
public:
    virtual Object* Apply(Object*, Scope*) {
        throw RuntimeError{"No impl"};
    };
};

// Proxy object for Numbers in Scheme
class Number final : public Object {
private:
    NumericT value_ = 0;

public:
    constexpr Number() noexcept = default;

    Number(NumericT init_value) noexcept : value_(init_value){};

    Number(const Number& other) = default;

    Number& operator=(const Number& other) = default;

    NumericT GetValue() const noexcept {
        return value_;
    }

    Object* Clone() override {
        return Heap::Instance().Make<Number>(value_);
    }

    Object* Eval(Scope*) override {
        return this;
    }
    std::string Serialize() override {
        return std::to_string(value_);
    }

    void SetValue(NumericT new_value) noexcept {
        value_ = new_value;
    }

    auto operator<=>(const Number& other) const {
        return value_ <=> other.value_;
    };

    bool operator==(const Number& other) const {
        return value_ == other.value_;
    };

    Number operator+(const Number& other) const {
        return Number(value_ + other.value_);
    }

    Number operator-(const Number& other) const {
        return Number(value_ - other.value_);
    }

    Number operator*(const Number& other) const {
        return Number(value_ * other.value_);
    }

    Number operator/(const Number& other) const {
        return Number(value_ / other.value_);
    }
};

// Proxy object for Symbols in Schema
class Symbol final : public Object {
private:
    std::string name_;

public:
    Symbol() noexcept = default;

    Symbol(const std::string& init_name) noexcept : name_(init_name) {
    }

    Object* Clone() override {
        return Heap::Instance().Make<Symbol>(name_);
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    void SetName(const std::string& new_name) noexcept {
        name_ = new_name;
    }

    std::string Serialize() override {
        return name_;
    }

    Object* Eval(Scope* scope) override;
};

// Proxy object for Bool in Schema
class Bool final : public Object {
private:
    bool state_ = false;

public:
    constexpr Bool() noexcept = default;

    Bool(bool init_state) noexcept : state_(init_state) {
    }

    Object* Clone() override {
        return Heap::Instance().Make<Bool>(state_);
    }

    bool GetState() const noexcept {
        return state_;
    }
    Object* Eval(Scope*) override {
        return this;
    }

    std::string Serialize() override {
        return state_ ? "#t" : "#f";
    }

    [[maybe_unused]] void SetState(bool new_state) noexcept {
        state_ = new_state;
    }
};

class Cell final : public Object {
private:
    Object* head_ = nullptr;
    Object* tail_ = nullptr;

public:
    constexpr Cell() noexcept = default;

    Cell(Object* init_head, Object* init_tail) : head_(init_head), tail_(init_tail) {
    }

    Object* Clone() override {
        return Heap::Instance().Make<Cell>(head_->Clone(), tail_->Clone());
    }

    Object* GetFirst() noexcept {
        return head_;
    }

    Object* GetSecond() noexcept {
        return tail_;
    }

    void SetHead(Object* new_head) noexcept {
        head_ = new_head;
    }

    void SetTail(Object* new_tail) noexcept {
        tail_ = new_tail;
    }

    Object* Eval(Scope* scope) override;

    std::string Serialize() override;
};

class Quote final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

//

class CheckType : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;

private:
    virtual bool IsTypeOf(Object*, Scope*) {
        throw RuntimeError{"Not impl"};
    };
};

class IsBool final : public CheckType {
private:
    bool IsTypeOf(Object* target_object, Scope* scope) override;
};

class IsNumber final : public CheckType {
private:
    bool IsTypeOf(Object* target_object, Scope* scope) override;
};

class IsSymbol final : public CheckType {
private:
    bool IsTypeOf(Object* target_object, Scope* scope) override;
};

class IsPair final : public CheckType {
private:
    bool IsTypeOf(Object* target_object, Scope* scope) override;
};

class IsNull final : public CheckType {
private:
    bool IsTypeOf(Object* target_object, Scope* scope) override;
};

class IsList final : public CheckType {
private:
    bool IsTypeOf(Object* target_object, Scope* scope) override;
};

//

template <class F>
class BoolOperator : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override {
        bool state = !bool_operation_func_(true, false);
        if (!head) {
            return Heap::Instance().Make<Bool>(state);
        }
        if (!Is<Cell>(head) || !As<Cell>(head)->GetFirst()) {
            throw RuntimeError{"Incorrect cell"};
        }
        Object* object = nullptr;
        while (head && Is<Cell>(head)) {
            object = As<Cell>(head)->GetFirst()->Eval(scope);
            if (Is<Bool>(object)) {
                state = bool_operation_func_(state, As<Bool>(object)->GetState());
            } else {
                state = bool_operation_func_(state, true);
            }
            head = As<Cell>(head)->GetSecond();
            if (head && Is<Cell>(head) && state == bool_operation_func_(true, false)) {
                return Heap::Instance().Make<Bool>(state);
            }
        }
        if (state) {
            return object;
        }
        return Heap::Instance().Make<Bool>(state);
    };

private:
    F bool_operation_func_;
};

class Not final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class And final : public BoolOperator<std::logical_and<bool>> {};

class Or final : public BoolOperator<std::logical_or<bool>> {};

//

template <class F>
class Comparator : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override {
        if (!head) {
            return Heap::Instance().Make<Bool>(true);
        }
        std::vector<Object*> args = GetVectorFromCell(head, scope);
        if (args.size() < 2) {
            throw RuntimeError{"Incorrect compare args count, require > 1"};
        }
        for (size_t i = 0; i < args.size() - 1; ++i) {
            if (!Is<Number>(args[i]) || !Is<Number>(args[i + 1])) {
                throw RuntimeError{"Invalid args in compare func!"};
            }
            // monotonic functions
            if (!compare_func_(*As<Number>(args[i]), *As<Number>(args[i + 1]))) {
                return Heap::Instance().Make<Bool>(false);
            }
        }
        return Heap::Instance().Make<Bool>(true);
    };

private:
    F compare_func_;
};

class Equal final : public Comparator<std::equal_to<Number>> {};

class Less final : public Comparator<std::less<Number>> {};

class Greater final : public Comparator<std::greater<Number>> {};

class LessEqual final : public Comparator<std::less_equal<Number>> {};

class GreaterEqual final : public Comparator<std::greater_equal<Number>> {};

//

template <class Operation, bool IsGroupOperation, NumericT NeutralElement = -1>
class ArithmeticOperator : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override {
        if (!head) {
            if (!IsGroupOperation) {
                throw RuntimeError{"Didn't support neutral element"};
            }
            return Heap::Instance().Make<Number>(NeutralElement);
        }
        std::vector<Object*> args = GetVectorFromCell(head, scope);
        if (args.empty()) {
            return Heap::Instance().Make<Number>();
        }
        if (!args.front() || !Is<Number>(args.front())) {
            throw RuntimeError{"Incorrect args for arithmetics"};
        }
        Number result = *As<Number>(args.front());
        for (size_t i = 1; i < args.size(); ++i) {
            if (!Is<Number>(args[i])) {
                throw RuntimeError{"!!!!"};
            }
            result = operation_func_(result, *As<Number>(args[i]));
        }
        return Heap::Instance().Make<Number>(result);
    };

private:
    Operation operation_func_;
};

class Add final : public ArithmeticOperator<std::plus<Number>, true, 0> {};

class Product final : public ArithmeticOperator<std::multiplies<Number>, true, 1> {};

class Sub final : public ArithmeticOperator<std::minus<Number>, false> {};

class Divide final : public ArithmeticOperator<std::divides<Number>, false> {};

template <class T>
struct MaxOp {
    constexpr T operator()(const T& lhs, const T& rhs) const {
        return lhs > rhs ? lhs : rhs;
    }
};

template <class T>
struct MinOp {
    constexpr T operator()(const T& lhs, const T& rhs) const {
        return lhs < rhs ? lhs : rhs;
    }
};

class Max final : public ArithmeticOperator<MaxOp<Number>, false> {};

class Min final : public ArithmeticOperator<MinOp<Number>, false> {};

//

class Abs final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

//

class Cons final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class Car final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class Cdr final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class List final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class ListRef final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class ListTail final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class If final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class Define final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class Set final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class SetCar final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class SetCdr final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class MakeLambda final : public Function {
public:
    Object* Apply(Object* head, Scope* scope) override;
};

class Lambda final : public Function {
private:
    Scope* scope_ = nullptr;
    std::vector<std::string> args_;
    std::vector<Object*> body_;

public:
    // Some ctrs
    Lambda(std::vector<std::string> args, std::vector<Object*> body, Scope* parent_scope);

    Object* Apply(Object* ptr, Scope* scope) override;

    [[maybe_unused]] std::vector<std::string>& GetArgs() {
        return args_;
    }

    std::vector<Object*>& GetBody() {
        return body_;
    }

    Scope* GetScope() {
        return scope_;
    }

    Object* Clone() override {
        return Heap::Instance().Make<Lambda>(args_, body_, scope_);
    };
};
