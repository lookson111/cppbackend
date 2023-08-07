
SELECT year FROM movies WHERE year BETWEEN 2000 AND 2020 GROUP BY year HAVING sum(box)=(
    SELECT max(box_sum) FROM (
            SELECT sum(box) as box_sum FROM movies WHERE year BETWEEN 2000 AND 2020 GROUP BY year
    ) AS sum_tab
);

// Фильмов с каким жанром больше всего в БД?
SELECT genre FROM movies GROUP BY genre HAVING count(*)=(
    SELECT max(genre_cnt) FROM (
            SELECT count(*) as genre_cnt FROM movies GROUP BY genre
    ) AS sum_tab
);
// Российских фильмов (с country='Rus') с каким жанром больше всего в БД?
SELECT genre FROM movies WHERE country='Rus' GROUP BY genre HAVING count(*)=(
    SELECT max(genre_cnt) FROM (
            SELECT count(*) as genre_cnt FROM movies WHERE country='Rus' GROUP BY genre
    ) AS sum_tab
);

SELECT box::float/budget AS genre_cnt FROM movies WHERE box>0;

// Вычислите самый провальный фильм — у которого отношение box к budget самое низкое, но тем не менее box больше нуля. Постарайтесь найти ответ одним запросом. Название впишите текстом.
// Подсказка: чтобы при делении ответ не округлялся до целого числа, конвертируйте box к типу float. Это можно делать так: box::float/budget.
SELECT name FROM movies WHERE box::float/budget=(
    SELECT min(box_bud) FROM (
            SELECT box::float/budget AS box_bud FROM movies WHERE box>0
    ) AS min_tab
);

// У какого фильма самое длинное название? Длина строки вычисляется функцией length. 
// Это не агрегация, так что к length можно применить агрегацию.
SELECT name FROM movies WHERE length(name)=(
    SELECT max(box_bud) FROM (
            SELECT length(name) AS box_bud FROM movies
    ) AS min_tab
);

//Какая страна заработала больше всего за всё время? 
//Имеется ввиду чистая прибыль: суммарная разница между box и budget. Впишите трёхбуквенное название страны.
SELECT country FROM movies GROUP BY country HAVING sum(box-budget)=(
    SELECT max(genre_cnt) FROM (
            SELECT sum(box-budget) as genre_cnt FROM movies GROUP BY country
    ) AS sum_tab
);

// Сколько стран сняло не менее 20 фильмов, начиная с 2000 года?
SELECT country FROM movies GROUP BY country HAVING sum(box-budget)=(
    SELECT max(genre_cnt) FROM (
            SELECT sum(box-budget) as genre_cnt FROM movies GROUP BY country
    ) AS sum_tab
);

// Сколько стран сняло не менее 20 фильмов, начиная с 2000 года?
SELECT count(col_cnt) FROM (
        SELECT count(*) as col_cnt FROM movies WHERE year>=2000 GROUP BY country
) AS sum_tab WHERE col_cnt>=20;

