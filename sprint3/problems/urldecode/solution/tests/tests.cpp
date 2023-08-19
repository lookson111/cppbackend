#define BOOST_TEST_MODULE urldecode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"
using namespace std::literals;

static constexpr std::string_view decode_true_code = 
	"https://ru.wikipedia.org/wiki/\%D0\%97\%D0\%B0\%D0\%B3\%D0\%BB\%D0\%B0"
    "\%D0\%B2\%D0\%BD\%D0\%B0\%D1\%8F_\%D1\%81\%D1\%82\%D1\%80\%D0\%B0\%D0"
    "\%BD\%D0\%B8\%D1\%86\%D0\%B0"sv;
static const std::string decode_true_encode = 
    "https://ru.wikipedia.org/wiki/Заглавная_страница"s;
static constexpr std::string_view decode_throw = 
	"https://ru.wikipedia.org/wiki/\%vD0\%97\%D0\%B0\%D0\%B3\%D0\%BB\%D0"
    "\%B0\%D0\%B2\%D0\%BD\%D0\%B0\%D1"sv;
static constexpr std::string_view decode_not_full =
    "https://ru.wikipedia.org/wiki/\%D0\%97\%D0\%B0\%D0\%B3\%D0\%BB\%D"
    "\%B0\%D0\%B2\%D0\%BD\%D0\%B0\%D"sv;

static constexpr std::string_view decode_space_code   = "Hello+world!"sv;
static constexpr std::string_view decode_space_encode = "hello world!"sv;

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {

    BOOST_TEST(UrlDecode(""sv) == ""s);
    BOOST_TEST(UrlDecode("http:://ya.ru"sv) == "http:://ya.ru"s);
    BOOST_TEST(UrlDecode(decode_true_code) == decode_true_encode);
    BOOST_CHECK_THROW(UrlDecode(decode_throw), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode(decode_not_full), std::invalid_argument);
    BOOST_TEST(UrlDecode(decode_space_code) == decode_space_encode);
    // Напишите остальные тесты для функции UrlDecode самостоятельно
}