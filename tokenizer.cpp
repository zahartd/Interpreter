#include <tokenizer.h>

#include <array>
#include <algorithm>

#include <error.h>

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool BooleanToken::operator==(const BooleanToken& other) const {
    return state == other.state;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream* in) : input_stream_(in) {
    Next();
}

bool Tokenizer::IsEnd() const noexcept {
    return end_flag_;  // == EOF (\0)
}

void Tokenizer::Next() {
    int current_char = SkipSpaces();
    // Skip if it EOF
    if (IsEndSymbol(current_char)) {
        end_flag_ = true;
        return;
    }
    // Processing one symbol tokens
    if (current_char == '(') {
        last_read_token_.emplace<BracketToken>(BracketToken::OPEN);
        input_stream_->get();
    } else if (current_char == ')') {
        last_read_token_.emplace<BracketToken>(BracketToken::CLOSE);
        input_stream_->get();
    } else if (current_char == '\'') {
        last_read_token_.emplace<QuoteToken>();
        input_stream_->get();
    } else if (current_char == '.') {
        last_read_token_.emplace<DotToken>();
        input_stream_->get();
    } else if (current_char == '#') {
        input_stream_->get();
        current_char = input_stream_->peek();
        if (current_char == 't') {
            last_read_token_.emplace<BooleanToken>(true);
        } else if (current_char == 'f') {
            last_read_token_.emplace<BooleanToken>(false);
        } else {
            throw SyntaxError("Incorrectness! #");  // todo: Add more informational text of error
        }
        input_stream_->get();
        // Processing more symbols tokens:
    } else if (IsNumber(current_char, GetNextChar())) {
        if (current_char == '+') {
            current_char = input_stream_->peek();
        }
        std::string current_number;  // todo: SSO?
        do {
            current_number += current_char;
            input_stream_->get();
            current_char = input_stream_->peek();
        } while (std::isdigit(current_char));
        last_read_token_.emplace<ConstantToken>(std::stoll(current_number));
    } else if (IsSymbolsStart(current_char)) {
        std::string symbols;
        do {
            symbols += input_stream_->get();
        } while (IsSymbolsInternal(input_stream_->peek()));
        last_read_token_.emplace<SymbolToken>(symbols);
    } else {
        throw SyntaxError("Unknown name!");
    }
}

Token Tokenizer::GetToken() const noexcept {
    return last_read_token_;
}

[[maybe_unused]] bool Tokenizer::IsBinaryOperation(int target_char, int next_char) noexcept {
    static constexpr std::array<char, 7> kOperationsChars{'+', '-', '*', '/', '<', '=', '>'};
    return (std::find(kOperationsChars.begin(), kOperationsChars.end(), target_char) !=
            kOperationsChars.end()) &&
           !isdigit(next_char);
}

bool Tokenizer::IsNumber(int target_char, int next_char) noexcept {
    return ((target_char == '+' || target_char == '-') && std::isdigit(next_char)) ||
           std::isdigit(target_char);
}

bool Tokenizer::IsSymbolsInternal(int target_char) noexcept {
    static constexpr std::array<int, 10> kAvailableChars{'<', '=', '>', '*', '/',
                                                         '#', '?', '!', '-'};
    return std::isalpha(target_char) || std::isdigit(target_char) ||
           std::find(kAvailableChars.begin(), kAvailableChars.end(), target_char) !=
           kAvailableChars.end();
}

bool Tokenizer::IsSymbolsStart(int target_char) noexcept {
    static constexpr std::array<int, 8> kAvailableChars{'<', '=', '>', '*', '/', '#', '+', '-'};
    return std::isalpha(target_char) || std::find(kAvailableChars.begin(), kAvailableChars.end(),
                                                  target_char) != kAvailableChars.end();
}

bool Tokenizer::IsEndSymbol(int target_char) noexcept {
    return target_char == EOF;
}

int Tokenizer::GetNextChar() const noexcept {
    input_stream_->get();
    int next_char = input_stream_->peek();
    input_stream_->unget();
    return next_char;
}

int Tokenizer::SkipSpaces() noexcept {
    int current_char = input_stream_->peek();
    while (std::isspace(current_char)) {
        input_stream_->get();
        current_char = input_stream_->peek();
    }
    return current_char;
}
