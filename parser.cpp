#include <parser.h>

#include <variant>

Object* Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError{"Nothing to read!"};
    }
    auto current_token = tokenizer->GetToken();
    tokenizer->Next();  // Reading the next token
    if (ConstantToken* constant_current_token = std::get_if<ConstantToken>(&current_token)) {
        return Heap::Instance().Make<Number>(constant_current_token->value);
    } else if (SymbolToken* symbol_current_token = std::get_if<SymbolToken>(&current_token)) {
        return Heap::Instance().Make<Symbol>(symbol_current_token->name);
    } else if (BooleanToken* boolean_current_token = std::get_if<BooleanToken>(&current_token)) {
        return Heap::Instance().Make<Bool>(boolean_current_token->state);
    } else if (std::holds_alternative<DotToken>(current_token)) {
        return Heap::Instance().Make<Symbol>(".");
    } else if (std::holds_alternative<QuoteToken>(current_token)) {
        return Heap::Instance().Make<Cell>(Heap::Instance().Make<Symbol>("quote"), Read(tokenizer));
    } else if (BracketToken* bracket_current_token = std::get_if<BracketToken>(&current_token)) {
        if (*bracket_current_token == BracketToken::CLOSE) {
            throw SyntaxError{"Expected '(' but you input ')'"};
        }
        return ReadList(tokenizer);
    } else {
        throw SyntaxError{"Unknown sequence"};
    }
}

Object* ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError{"Some error"};
    }
    auto current_token = tokenizer->GetToken();
    if (std::holds_alternative<BracketToken>(current_token) &&
        std::get<BracketToken>(current_token) == BracketToken::CLOSE) {
        tokenizer->Next();  // Reading the next token
        return nullptr;
    }

    // Pair (Cell): ( [head] [DotToken (is pair) / " " (is list)] [tail] )
    if (tokenizer->IsEnd()) {
        throw SyntaxError{"The cell is not closed"};
    }
    Object* head = Read(tokenizer);

    if (tokenizer->IsEnd()) {
        throw SyntaxError{"Some error"};
    }
    current_token = tokenizer->GetToken();
    if (std::holds_alternative<DotToken>(current_token)) {  // Is pair?
        tokenizer->Next();
        Object* tail = Read(tokenizer);
        if (tokenizer->IsEnd()) {
            throw SyntaxError{"The cell is not closed"};
        }
        current_token = tokenizer->GetToken();
        if (std::holds_alternative<BracketToken>(current_token) &&
            std::get<BracketToken>(current_token) == BracketToken::CLOSE) {
            tokenizer->Next();  // Reading the next token
            return Heap::Instance().Make<Cell>(head, tail);
        } else {
            throw SyntaxError{"Not supported syntax"};
        }
    } else {  // Is list?
        Object* tail = ReadList(tokenizer);
        return Heap::Instance().Make<Cell>(head, tail);
    }
}
