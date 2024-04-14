#pragma once

#include <variant>
#include <optional>
#include <istream>
#include "string"

#include "constans.h"

struct SymbolToken {
    std::string name;

    SymbolToken(std::string str) noexcept : name(std::move(str)) {
    }

    SymbolToken(char chr) noexcept : name(std::string{chr}) {
    }

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

struct BooleanToken {
    bool state = false;

    BooleanToken(bool new_state) : state(new_state) {
    }

    bool operator==(const BooleanToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    NumericT value;

    ConstantToken(NumericT value) noexcept : value(value) {
    }

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<std::monostate, ConstantToken, BracketToken, SymbolToken, BooleanToken,
        QuoteToken, DotToken>;

class Tokenizer {
private:
    std::istream* input_stream_;
    Token last_read_token_;
    bool end_flag_ = false;

public:
    // Creates a tokenizer that reads characters from the in stream.
    Tokenizer(std::istream* in);

    // Whether we have reached the end of the stream or not.
    bool IsEnd() const noexcept;

    // Attempt to read the next token.
    // Either IsEnd() will become true, or the token can be obtained via GetToken().
    void Next();

    // Get the current token.
    Token GetToken() const noexcept;

private:
    [[maybe_unused]] static bool IsBinaryOperation(int target_char, int next_char) noexcept;

    static bool IsNumber(int target_char, int next_char) noexcept;

    static bool IsSymbolsInternal(int target_char) noexcept;

    static bool IsSymbolsStart(int target_char) noexcept;

    static bool IsEndSymbol(int target_char) noexcept;

    int SkipSpaces() noexcept;

    int GetNextChar() const noexcept;
};
