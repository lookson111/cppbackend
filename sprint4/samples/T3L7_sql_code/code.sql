// Выберите запрос, выводящий список актёров, игравших в финских фильмах после 2016 года
SELECT * FROM relations 
  JOIN movies ON movies.id=relations.movie 
  JOIN persons ON persons.id=relations.person 
  WHERE movies.country='Fin' AND movies.year>=2016 AND relations.role='актёр'; 
  
//Используя GROUP BY вместе с JOIN, сопоставьте жанр фильмов с количеством наград, 
//которые получили фильмы этого жанра. Учитывайте только фильмы, снятые до 2000 года (не включительно):
SELECT movies.genre, count(*) FROM movies
  JOIN awards ON movies.id=awards.movie
  WHERE movies.year<2000 GROUP BY movies.genre;
  
// Без использования вложенных запросов найдите количество фильмов, не получивших наград.
// Подсказка: вам пригодится LEFT JOIN.
SELECT count(*) FROM movies
LEFT JOIN awards ON movies.id=awards.movie WHERE awards.award IS NULL;

// Узнайте имя актёра, который снялся в наибольшем количестве фильмов. 
// Учтите, что одна и та же персоналия может выступать в фильмах как в качестве актёра, 
// так и в других амплуа. Вас интересует только участие в качестве актёра.
// Задачу можно решить несколькими способами, но постарайтесь узнать ответ одним запросом, использующим JOIN.
SELECT person, name, count(*) FROM relations 
    JOIN persons ON persons.id=person 
    WHERE relations.role='актёр' 
    GROUP BY relations.person, persons.name 
    ORDER BY count(*) DESC LIMIT 10;
  
// Узнайте название фильма, который получил больше всего «Патриков» (золотых, серебряных и бронзовых). 
// Подсказка: для определения нужных наград вам поможет операция LIKE.
SELECT name, count(*) FROM movies JOIN awards ON movies.id=movie WHERE awards.award LIKE '%Патрик' GROUP BY movies.name ORDER BY count(*) DESC;

// Найдите количество людей из таблицы persons, которые получили более одного Оскара, 
// будучи режиссёром фильма. Попробуйте найти ответ одним запросом.
SELECT count(*) FROM
    (SELECT relations.person as p_pers, count(*) as p_cnt FROM relations
        JOIN awards ON awards.movie=relations.movie
        JOIN persons ON persons.id=relations.person
        WHERE relations.role='режиссёр' AND awards.award='Оскар'
        GROUP BY relations.person
        HAVING count(*)>1
    ) AS t_table;
