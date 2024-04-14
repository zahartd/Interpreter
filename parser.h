#pragma once

#include <memory>

#include "object.h"
#include <tokenizer.h>
#include <error.h>

Object* Read(Tokenizer* tokenizer);

Object* ReadList(Tokenizer* tokenizer);