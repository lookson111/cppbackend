#include <iostream>
#include <pqxx/pqxx>

using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: connect_db <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }

        // Подключаемся к БД, указывая её параметры в качестве аргумента
        pqxx::connection conn{argv[1]};

        // Создаём транзакцию. Это понятие будет разобрано в следующих уроках.
        // Транзакция нужна, чтобы выполнять запросы.
        pqxx::work w(conn);

        // Используя транзакцию создадим таблицу в выбранной базе данных:
        w.exec(
            "CREATE TABLE IF NOT EXISTS movies (id SERIAL PRIMARY KEY, title varchar(200) NOT NULL, year integer NOT NULL);"_zv);

        w.exec("DELETE FROM movies;"_zv);
        w.exec(
            "INSERT INTO movies (title, year) VALUES ('Trash', 2014), ('The Kid', 2000), "
            "('The Sting', 1973), ('The Terminal', 2004), ('Amarcord', 1973), ('The King''s Speech', 2010), "
            "('Det sjunde inseglet', 1957), ('Groundhog Day', 1993);"_zv);
        // Создадим таблицу points и добавим в неё строки с неопределёнными значениями
        w.exec("CREATE TABLE IF NOT EXISTS points (x int, y int);"_zv);
        w.exec("INSERT INTO points VALUES (DEFAULT, 10), (20, DEFAULT);"_zv);
        std::string title;
        int year;
        constexpr auto tag_ins_movie = "ins_movie"_zv;
        conn.prepare(tag_ins_movie, "INSERT INTO movies (title, year) VALUES ($1, $2)"_zv);
        std::cout << "Enter name, press ENTER, enter year, press CTRL+D" << std::endl;
        while (std::getline(std::cin, title) >> year) {
            w.exec("INSERT INTO movies (title, year) VALUES (" + w.quote(title) + ", "
                   + std::to_string(year) + ")");
            w.exec_prepared(tag_ins_movie, title, year);
            std::cin >> std::ws;
        } 

        // Применяем все изменения
        w.commit();
        pqxx::read_transaction r(conn);
        
        // Вычислим кубический корень из 100:
        {
            double cube_root_of_100 = r.query_value<double>("SELECT ||/100.;"_zv);
            std::cout << "Cube root of 100: " << cube_root_of_100 << std::endl;
        }
        
        // Перечислим фильмы до 2000 года.
        {
            auto query_text = "SELECT id, title, year FROM movies WHERE year < 2000 ORDER BY year, title"_zv;
        
            // Выполняем запрос и итерируемся по строкам ответа
            for (auto [id, title, year] : r.query<int, std::string_view, int>(query_text)) {
                std::cout << "Movie "sv << title << " ["sv << id << "] filmed in "sv << year
                          << std::endl;
            }
        }
        
        // Найдём фильм 2004 года. Мы уверены, что он есть в базе.
        { 
            // Тут не получится использовать std::string_view, так как 
            // мы не сохранили объект ответа, данные будут уничтожены.
            auto [id, title] = r.query1<int, std::string>("SELECT id, title FROM movies WHERE year=2004 LIMIT 1;"_zv);
            std::cout << "Movie of 2004: "sv << title << " ["sv << id << "];"sv << std::endl;
        }
        
        // Поищем фильм 1999 года. Мы не уверены, что он есть в базе.
        {
            // Результат query01 возвращается в виде optional:
            std::optional result = r.query01<int, std::string>(
                "SELECT id, title FROM movies WHERE year=1999 LIMIT 1;"_zv);
        
            if (result) {
                auto [id, title] = *result;
                std::cout << "Movie of 1999: "sv << title << " ["sv << id << "];"sv << std::endl;
            } else {
                std::cout << "No movie of 1999 in database"sv << std::endl;
            }
        }
        {
            for (auto [x, y] : r.query<std::optional<int>, std::optional<int>>("SELECT x, y FROM points;"_zv)) {
                std::cout << x.value_or(-9999) << ":" << y.value_or(-9999) << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
