#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
    CHECK(HtmlDecode("Johnson&amp;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&ampJohnson"sv)  == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMP;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMPJohnson"sv)  == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AmPJohnson"sv) == "Johnson&AmPJohnson"s);
    CHECK(HtmlDecode("Johnson&amJohnson"sv) == "Johnson&amJohnson"s);
    CHECK(HtmlDecode("&ltJohnson&apos;Johnson&GT"sv) == "<Johnson\'Johnson>"s);
    CHECK(HtmlDecode("Johnson&Johnson"sv)     == "Johnson&Johnson"s); 
    CHECK(HtmlDecode("&amp;lt;"sv) == "&lt;"s);

}

// Напишите недостающие тесты самостоятельно
